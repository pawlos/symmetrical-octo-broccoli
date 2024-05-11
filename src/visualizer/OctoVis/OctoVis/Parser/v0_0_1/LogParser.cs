using System.IO;
using System.Text.RegularExpressions;
using OctoVis.Model;

namespace OctoVis.Parser.v0_0_1;

public class LogParser : IParser
{
    public ProfilerDataModel Parse(ulong startTicks, StreamReader stream)
    {
        uint totalMemoryAllocated = 0;
        ulong? gcStartEventTicks = null;
        var model = new ProfilerDataModel();
        while (!stream.EndOfStream)
        {
            var line = stream.ReadLine() ?? string.Empty;
            if (line.Contains("OctoProfiler::Prepare for shutdown..."))
            {
                var endTicks = ParseTimestamp(line);
                model.TotalTime = TimeSpan.FromMicroseconds(CalculateTimestamp(endTicks, startTicks));
                model.StartMarker = CalculateTimestamp(startTicks, startTicks);
                model.EndMarker = CalculateTimestamp(endTicks, startTicks);
            }
            else if (line.Contains("OctoProfiler::Detected"))
            {
                var match = Regex.Match(line, @"\[[^ ]+\] OctoProfiler::Detected (.+)");
                model.NetVersion = match.Groups[1].Value;
            }
            else if (line.Contains("OctoProfiler::ObjectAllocated"))
            {
                totalMemoryAllocated = ParseMemoryAllocationPart(startTicks, stream, line, totalMemoryAllocated, model);
            }
            else if (line.Contains("OctoProfiler::GarbageCollection"))
            {
                var time = ParseTimestamp(line);
                if (line.Contains("OctoProfiler::GarbageCollectionStarted"))
                {
                    gcStartEventTicks = time;
                }
                else if (line.Contains("OctoProfiler::GarbageCollectionFinished") && gcStartEventTicks.HasValue)
                {
                    var gcEnd = time;
                    var gcStartTime = CalculateTimestamp(gcStartEventTicks.Value, startTicks);
                    var duration = CalculateTimestamp(gcEnd, startTicks) - gcStartTime;
                    model.GcData.Add(new RangeDataPoint(gcStartTime, duration));
                    model.TotalGcTime += TimeSpan.FromTicks((long)duration);
                }
            }
            else if (line.Contains("OctoProfiler::ThreadCreated"))
            {
                var time = ParseTimestamp(line);
                //  [{0},{1}]
                var match = Regex.Match(line, @"OctoProfiler::ThreadCreated \[(\d+),(\d+)\]");
                var threadId = int.Parse(match.Groups[1].Value);
                var managedId = ulong.Parse(match.Groups[2].Value);
                model.Threads.Add(new ProfilerDataModel.ThreadInfo(
                    CalculateTimestamp(time, startTicks),
                    threadId,
                    managedId));
            }
            else if (line.Contains("OctoProfiler::ExceptionThrown"))
            {
                var currentTicks = ParseTimestamp(line);
                var (exceptionType, threadId) = ParseException(line);

                var stack = new List<ProfilerDataModel.StackFrame>();
                var stackFrame = stream.ReadLine();
                while (!stackFrame?.Contains("OctoProfiler::DoStackSnapshot end") ?? false)
                {
                    var (module, frame) = ParseFrame(stackFrame);
                    stack.Add(new ProfilerDataModel.StackFrame(module, frame));
                    stackFrame = stream.ReadLine();
                }

                var point = new DataPoint(CalculateTimestamp(currentTicks, startTicks), 0);
                model.ExceptionData.Add(point);
                stack.Reverse();
                model.ExceptionsInfo.Add(point.Time, new ProfilerDataModel.ExceptionInfo(
                    exceptionType,
                    threadId,
                    stack));
            }
        }

        return model;
    }

    private static uint ParseMemoryAllocationPart(
        ulong startTicks,
        StreamReader stream,
        string line,
        uint totalMemoryAllocated,
        ProfilerDataModel model)
    {
        var currentTicks = ParseTimestamp(line);
        (string type, uint bytes) = ParseTypeAndBytesAllocated(line);
        totalMemoryAllocated += bytes;
        if (model.TypeAllocationInfo.TryGetValue(type, out var v))
        {
            var (oldMemory, oldCount) = v;
            model.TypeAllocationInfo[type] = (oldMemory + bytes, oldCount + 1);
        }
        else
        {
            model.TypeAllocationInfo.Add(type, (bytes, 1));
        }

        model.MemoryData.Add(new DataPoint(CalculateTimestamp(currentTicks, startTicks), bytes));
        model.TotalMemoryData.Add(new DataPoint(CalculateTimestamp(currentTicks, startTicks),
            totalMemoryAllocated));

        var allocationStack = new List<ProfilerDataModel.StackFrame>();
        var stackFrame = stream.ReadLine();
        while (!stackFrame?.Contains("OctoProfiler::DoStackSnapshot end") ?? false)
        {
            var (module, frame) = ParseFrame(stackFrame);
            allocationStack.Add(new ProfilerDataModel.StackFrame(module, frame));
            stackFrame = stream.ReadLine();
        }

        allocationStack.Reverse();
        model.ObjectAllocationPath.Add(Guid.NewGuid(), (type, allocationStack));
        return totalMemoryAllocated;
    }


    private static (string, uint) ParseTypeAndBytesAllocated(string line)
    {
        var match = Regex.Match(line, @"\[[^ ]+\] OctoProfiler::ObjectAllocated (\d+) \[B\] for (.*)");
        return (match.Groups[2].Value, uint.Parse(match.Groups[1].Value));
    }

    private static ulong ParseTimestamp(string line)
    {
        var match = Regex.Match(line, @"\[(\d+)ns\].*");
        return ulong.Parse(match.Groups[1].Value);
    }

    private static (string,string) ParseException(string line)
    {
        var match = Regex.Match(line, @"\[[^ ]+\] OctoProfiler::ExceptionThrown (.*) on thread (.*)");
        return (match.Groups[1].Value, match.Groups[2].Value);
    }

    private static ulong CalculateTimestamp(ulong currentTicks, ulong startTicks)
    {
        // convert to microsecond
        return (currentTicks - startTicks) / 1000;
    }

    private static (string,string) ParseFrame(string stackFrameLine)
    {
        var match = Regex.Match(stackFrameLine, @"\[(\d+)ns\] OctoProfiler::(Native|Managed) frame ([^;]*);?([^;]*)");
        return match.Groups[2].Value == "Native" ?
            (string.Empty, match.Groups[3].Value) :
            (match.Groups[3].Value, match.Groups[4].Value);
    }
}