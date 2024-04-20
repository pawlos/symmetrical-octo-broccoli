namespace OctoVis.Model;

public class SettingsDataModel
{
    public enum DataSize
    {
        Bytes,
        KiloBytes,
        MegaBytes
    }

    public enum TimeSize
    {
        Minute,
        Second,
        Millisecond
    }

    public DataSize TimelineYAxis { get; set; }

    public TimeSize TimelineXAxis { get; set; }

    public string Filter { get; set; } = string.Empty;
}