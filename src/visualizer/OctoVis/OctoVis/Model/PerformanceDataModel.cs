using OctoVis.Parser.v0_0_2;

namespace OctoVis.Model;

public class PerformanceDataModel : IDataModel
{
    public ulong StartMarker { get; set; }

    public ulong EndMarker { get; set; }

    public TimeSpan RunTime => TimeSpan.FromTicks((long)(EndMarker - StartMarker));
    public List<LogParser.EnterExitEntryStacked> EnterExitModel { get; set; }
}