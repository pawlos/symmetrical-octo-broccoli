using OctoVis.Parser.v0_0_2;

namespace OctoVis.Model;

public class ProfilerDataModel : IDataModel
{
    public record StackFrame(string Module, string FrameInfo);

    public record ThreadInfo(ulong Time, int NativeThreadId, ulong ManagedThreadId, string ThreadName);

    public record ExceptionInfo(string ExceptionType, string ThreadId, string ThreadName, List<StackFrame> StackTrace);
    public Dictionary<string, (uint TotalMemory, uint Count)> TypeAllocationInfo { get; set; } = new();

    public Dictionary<Guid, (string Type, List<StackFrame> StackTraces)> ObjectAllocationPath = new();

    public Dictionary<double, ExceptionInfo> ExceptionsInfo { get; set; } = new();

    public List<DataPoint> ExceptionData { get; set; } = new();

    public List<DataPoint> TotalMemoryData { get; set; } = new();

    public List<DataPoint> MemoryData { get; set; } = new();

    public List<RangeDataPoint> GcData { get; set; } = new();

    public List<ThreadInfo> Threads { get; set; } = new();

    public TimeSpan TotalTime { get; set; }

    public ulong StartMarker { get; set; }

    public ulong EndMarker { get; set; }

    public TimeSpan TotalGcTime { get; set; }

    public string NetVersion { get; set; } = string.Empty;
}