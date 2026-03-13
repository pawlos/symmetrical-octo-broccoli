using System.IO;
using System.IO.Pipes;
using System.Text;
using OctoVis.Model;

namespace OctoVis.Parser;

public class BinaryPipeParser
{
    private const byte CmdSyncStarted = 0x00;
    private const byte CmdException = 0x01;
    private const byte CmdMemoryAlloc = 0x02;
    private const byte CmdGc = 0x03;
    private const byte CmdNewThread = 0x04;
    private const byte CmdThreadDestroyed = 0x05;
    private const byte CmdSyncFinished = 0xFF;

    public event Action<ProfilerDataModel>? ModelUpdated;
    public event Action<int>? ProgressUpdated;

    public async Task ConnectAndReadAsync(string pipeName, CancellationToken ct)
    {
        using var pipe = new NamedPipeClientStream(".", pipeName, PipeDirection.In, PipeOptions.Asynchronous);
        await pipe.ConnectAsync(ct);

        var model = new ProfilerDataModel();
        ulong startTicks = 0;
        uint totalMemory = 0;
        var eventCount = 0;

        try
        {
            while (!ct.IsCancellationRequested)
            {
                var cmd = await ReadUInt8Async(pipe, ct);
                var timestamp = await ReadUInt64Async(pipe, ct);

                if (startTicks == 0) startTicks = timestamp;
                ulong relativeTime = (timestamp - startTicks) / 1000; // ns → µs

                if (cmd == CmdSyncFinished)
                {
                    ModelUpdated?.Invoke(model);
                    break;
                }

                switch (cmd)
                {
                    case CmdSyncStarted:
                        _ = await ReadStringAsync(pipe, ct); // profiler version
                        model.NetVersion = await ReadStringAsync(pipe, ct);
                        model.SampleRate = await ReadUInt32Async(pipe, ct);
                        break;

                    case CmdException:
                        model = await HandleExceptionAsync(pipe, model, relativeTime, ct);
                        break;

                    case CmdMemoryAlloc:
                        (model, totalMemory) = await HandleMemoryAllocAsync(pipe, model, relativeTime, totalMemory, ct);
                        break;

                    case CmdGc:
                        model = await HandleGcAsync(pipe, model, relativeTime, ct);
                        break;

                    case CmdNewThread:
                        model = await HandleNewThreadAsync(pipe, model, relativeTime, ct);
                        break;

                    case CmdThreadDestroyed:
                        _ = await ReadStringAsync(pipe, ct); // discard thread_id
                        break;
                }

                eventCount++;
                ProgressUpdated?.Invoke(eventCount);
            }
        }
        catch (OperationCanceledException) { }
        catch (IOException) { /* pipe closed by profiler */ }

        ModelUpdated?.Invoke(model);
    }

    private static async Task<ProfilerDataModel> HandleExceptionAsync(
        PipeStream pipe, ProfilerDataModel model, ulong time, CancellationToken ct)
    {
        var exType = await ReadStringAsync(pipe, ct);
        var threadId = await ReadStringAsync(pipe, ct);
        var threadName = await ReadStringAsync(pipe, ct);
        var frameCount = await ReadUInt32Async(pipe, ct);

        var stack = new List<ProfilerDataModel.StackFrame>();
        for (uint i = 0; i < frameCount; i++)
        {
            var frameStr = await ReadStringAsync(pipe, ct);
            stack.Add(ParseFrame(frameStr));
        }

        var point = new DataPoint(time, 0);
        model.ExceptionData.Add(point);
        stack.Reverse();
        model.ExceptionsInfo[point.Time] = new ProfilerDataModel.ExceptionInfo(
            exType,
            threadId,
            string.IsNullOrEmpty(threadName) ? "Main thread" : threadName,
            stack);

        return model;
    }

    private static async Task<(ProfilerDataModel, uint)> HandleMemoryAllocAsync(
        PipeStream pipe, ProfilerDataModel model, ulong time, uint totalMemory, CancellationToken ct)
    {
        var typeName = await ReadStringAsync(pipe, ct);
        var bytes = (uint)await ReadUInt64Async(pipe, ct);
        var frameCount = await ReadUInt32Async(pipe, ct);

        var stack = new List<ProfilerDataModel.StackFrame>();
        for (uint i = 0; i < frameCount; i++)
        {
            var frameStr = await ReadStringAsync(pipe, ct);
            stack.Add(ParseFrame(frameStr));
        }

        totalMemory += bytes;

        if (model.TypeAllocationInfo.TryGetValue(typeName, out var v))
            model.TypeAllocationInfo[typeName] = (v.TotalMemory + bytes, v.Count + 1);
        else
            model.TypeAllocationInfo[typeName] = (bytes, 1);

        model.MemoryData.Add(new DataPoint(time, bytes));
        model.TotalMemoryData.Add(new DataPoint(time, totalMemory));

        stack.Reverse();
        model.ObjectAllocationPath[Guid.NewGuid()] = (typeName, stack);

        return (model, totalMemory);
    }

    private static async Task<ProfilerDataModel> HandleGcAsync(
        PipeStream pipe, ProfilerDataModel model, ulong time, CancellationToken ct)
    {
        _ = await ReadStringAsync(pipe, ct); // gc_type string (e.g. "GEN0,GEN2")
        model.GcData.Add(new RangeDataPoint(time, 0));
        return model;
    }

    private static async Task<ProfilerDataModel> HandleNewThreadAsync(
        PipeStream pipe, ProfilerDataModel model, ulong time, CancellationToken ct)
    {
        var threadInfo = await ReadStringAsync(pipe, ct);
        // format: "win32,clr,name"
        var parts = threadInfo.Split(',', 3);
        if (parts.Length >= 3)
        {
            var win32Id = int.TryParse(parts[0], out var wid) ? wid : 0;
            var managedId = ulong.TryParse(parts[1], out var mid) ? mid : 0;
            var name = parts[2];
            model.Threads.Add(new ProfilerDataModel.ThreadInfo(
                time, win32Id, managedId,
                string.IsNullOrEmpty(name) ? "Main thread" : name));
        }
        return model;
    }

    private static ProfilerDataModel.StackFrame ParseFrame(string frameStr)
    {
        // format: "native;<ip>" or "managed;<function_name>;<ip>"
        var parts = frameStr.Split(';', 3);
        if (parts.Length >= 1 && parts[0].Equals("native", StringComparison.OrdinalIgnoreCase))
            return new ProfilerDataModel.StackFrame(string.Empty, parts.Length > 1 ? parts[1] : string.Empty);
        if (parts.Length >= 2)
            return new ProfilerDataModel.StackFrame(parts[1], parts.Length > 2 ? parts[2] : string.Empty);
        return new ProfilerDataModel.StackFrame(string.Empty, frameStr);
    }

    private static async Task<byte> ReadUInt8Async(PipeStream pipe, CancellationToken ct)
    {
        var buf = new byte[1];
        await pipe.ReadExactlyAsync(buf, ct);
        return buf[0];
    }

    private static async Task<uint> ReadUInt32Async(PipeStream pipe, CancellationToken ct)
    {
        var buf = new byte[4];
        await pipe.ReadExactlyAsync(buf, ct);
        return BitConverter.ToUInt32(buf, 0);
    }

    private static async Task<ulong> ReadUInt64Async(PipeStream pipe, CancellationToken ct)
    {
        var buf = new byte[8];
        await pipe.ReadExactlyAsync(buf, ct);
        return BitConverter.ToUInt64(buf, 0);
    }

    private static async Task<string> ReadStringAsync(PipeStream pipe, CancellationToken ct)
    {
        var len = await ReadUInt32Async(pipe, ct);
        if (len == 0) return string.Empty;
        var buf = new byte[len];
        await pipe.ReadExactlyAsync(buf, ct);
        return Encoding.UTF8.GetString(buf);
    }
}
