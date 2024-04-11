namespace OctoVis.Model;

public class ProfilerDataModel
{
    public record AllocationStackFrame(string FrameInfo);
    public Dictionary<string, (uint TotalMemory, uint Count)> TypeAllocationInfo { get; set; } = new();

    public Dictionary<Guid, (string type, List<AllocationStackFrame>)> ObjectAllocationPath = new();

    public Dictionary<double,string> ExceptionInfo { get; set; } = new();

    public List<DataPoint> ExceptionData { get; set; } = new();

    public List<DataPoint> MemoryData { get; set; } = new();

    public List<RangeDataPoint> GcData { get; set; } = new();

    public TimeSpan TotalTime { get; set; }

    public TimeSpan TotalGcTime { get; set; }

    public string NetVersion { get; set; }
}