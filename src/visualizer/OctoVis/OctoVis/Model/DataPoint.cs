namespace OctoVis.Model;

public record DataPoint(ulong Time, uint Value);

public record RangeDataPoint(ulong TimeStart, ulong Duration);