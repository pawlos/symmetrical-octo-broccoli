namespace OctoVis.Model;

public class SettingsDataModel
{
    public enum DataSize
    {
        Bytes,
        KiloBytes,
        MegaBytes
    }

    public DataSize TimelineYAxis { get; set; }
}