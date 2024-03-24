namespace OctoVis.Model;

public class ProfilerDataModel
{
    public Dictionary<string,uint> TypeAllocationInfo { get; set; } = new();

    public Dictionary<double,string> ExceptionInfo { get; set; } = new();

    public List<DataPoint> ExceptionData { get; set; } = new();

    public List<DataPoint> MemoryData { get; set; } = new();
}