using System.ComponentModel;
using System.Runtime.CompilerServices;
using LiveChartsCore;
using LiveChartsCore.Defaults;
using LiveChartsCore.Measure;
using LiveChartsCore.SkiaSharpView;
using LiveChartsCore.SkiaSharpView.Drawing.Geometries;
using LiveChartsCore.SkiaSharpView.Painting;
using LiveChartsCore.SkiaSharpView.VisualElements;
using OctoVis.Model;
using SkiaSharp;

namespace OctoVis.ViewModel;

public sealed class ProfilerViewModel : INotifyPropertyChanged
{
    public record TypeAllocationsTable(string Name, double TotalAllocations, uint Count, uint UniqueStackTraces);
    public record ExceptionTable(string Name, string ThreadId, int Count);

    public LabelVisual TimelineTitle { get; set; } = new LabelVisual
    {
        Text = "Timeline - memory allocations/exceptions",
        TextSize = 20,
        Paint = new SolidColorPaint(SKColors.DarkSlateGray)
    };

    public LabelVisual Top20AllocationsTitle { get; set; } = new LabelVisual
    {
        Text = "Top 20 allocations",
        TextSize = 20,
        Paint = new SolidColorPaint(SKColors.DarkSlateGray)
    };

    public Axis[] TimelineYAxis { get; set; } = null!;
    public Axis[] TimelineXAxis { get; set; } = null!;

    public ISeries[] Timeline { get; set; } = null!;
    public ISeries?[] MemoryByType { get; set; } = null!;

    public RectangularSection[] Sections { get; set; } = null!;
    public event PropertyChangedEventHandler? PropertyChanged;

    private void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    private bool SetField<T>(ref T field, T value, [CallerMemberName] string? propertyName = null)
    {
        if (EqualityComparer<T>.Default.Equals(field, value)) return false;
        field = value;
        OnPropertyChanged(propertyName);
        return true;
    }

    public TypeAllocationsTable[] TypeAllocationInfo { get; set; } = null!;

    public ExceptionTable[] ExceptionCountInfo { get; set; } = null!;

    public string Filter { get; set; } = string.Empty;
    public string TotalTime { get; set; } = string.Empty;

    public string TotalGcTime { get; set; } = string.Empty;
    public bool LogParsed { get; set; }
    public bool LogNotParsed { get; set; } = true;

    public SettingsViewModel Settings { get; private set; } = null!;

    public static ProfilerViewModel FromDataModel(
        ProfilerDataModel data,
        SettingsDataModel settings)
    {
        var profilerViewModel = new ProfilerViewModel
        {
            Timeline =
            [
                new ColumnSeries<ObservablePoint>
                {
                    Values = data.MemoryData.GroupBy(x => GroupSamples(x, settings))
                        .Select(x => new ObservablePoint(x.Key, ParseData(x.Sum(y => y.Value), settings))),
                    Fill = new SolidColorPaint(SKColors.CornflowerBlue),
                    ScalesYAt = 0
                },
                new LineSeries<ObservablePoint>
                {
                    Values = data.TotalMemoryData.GroupBy(x => GroupSamples(x, settings))
                        .Select(x => new ObservablePoint(x.Key, ParseData(x.Max(y => y.Value), settings))),
                    Stroke = new SolidColorPaint(SKColors.ForestGreen),
                    Fill = null,
                    GeometrySize = 0,
                    ScalesYAt = 1
                },
                new ScatterSeries<ObservablePoint, RoundedRectangleGeometry>
                {
                    Values = data.ExceptionData.GroupBy(x => GroupSamples(x, settings))
                        .Select(x => new ObservablePoint(x.Key, 10)),
                    XToolTipLabelFormatter = point => data.ExceptionsInfo[point.Model!.X!.Value].ExceptionType,
                    YToolTipLabelFormatter = point => data.ExceptionsInfo[point.Model!.X!.Value].ExceptionType,
                    Fill = new SolidColorPaint(SKColor.Parse("FF0000"))
                },
            ]
        };
        var sections = data.GcData.GroupBy(x => GroupSamples(x, settings))
            .Select(gc => new RectangularSection
            {
                Xi = gc.Key,
                Xj = gc.Key + TimeSpan.FromMicroseconds((long)gc.OrderBy(l => l.TimeStart).Last().Duration)
                    .TotalSeconds,
                Fill = new SolidColorPaint { Color = SKColors.Orange.WithAlpha(80) },
                Stroke = new SolidColorPaint { Color = SKColors.Orange, StrokeThickness = 3 },
                Label = "GC",
                LabelSize = 200
            });
        sections = sections.Append(new RectangularSection
        {
            Xi = Sample(data.StartMarker, settings.TimelineXAxis),
            Xj = Sample(data.EndMarker, settings.TimelineXAxis),
            Stroke = new SolidColorPaint {Color = SKColors.Lime, StrokeThickness = 20},
            Yi = -10,
        });
        profilerViewModel.Sections = sections.ToArray();
        ISeries?[] series = data.TypeAllocationInfo
            .OrderByDescending(x => x.Value)
            .SkipWhile(x => x.Key == "<<no info>>")
            .Take(20)
            .Select(x => new PieSeries<double>
            {
                Values = new[] { (double)x.Value.TotalMemory },
                Name = x.Key
            } as ISeries).ToArray();
        profilerViewModel.MemoryByType = series;
        profilerViewModel.TotalGcTime = data.TotalGcTime.ToString();
        profilerViewModel.TotalTime = data.TotalTime.ToString();
        profilerViewModel.NetVersion = data.NetVersion;
        profilerViewModel.LogParsed = true;
        profilerViewModel.LogNotParsed = false;

        profilerViewModel.TypeAllocationInfo =
            data.TypeAllocationInfo
                .Where(x => string.IsNullOrWhiteSpace(settings.Filter) ||
                            (!string.IsNullOrWhiteSpace(settings.Filter) && x.Key.Contains(settings.Filter)))
                .Select(x => new TypeAllocationsTable(x.Key, ParseData(x.Value.TotalMemory, settings), x.Value.Count,
                    (uint)data.ObjectAllocationPath.Where(t => t.Value.Type == x.Key)
                        .Distinct(new StackTraceComparer())
                        .Count()))
                .ToArray();
        profilerViewModel.TimelineYAxis =
        [
            new()
            {
                Name = $"Memory allocations [{settings.TimelineYAxis}]",
                ShowSeparatorLines = true,
                UnitWidth = settings.TimelineYAxis switch
                {
                    SettingsDataModel.DataSize.Bytes => 1,
                    SettingsDataModel.DataSize.KiloBytes => 1024,
                    SettingsDataModel.DataSize.MegaBytes => 1024 * 1024,
                    _ => throw new ArgumentOutOfRangeException()
                },
                MinLimit = -10,
            },
            new()
            {
                Name = $"Total memory allocations [{settings.TimelineYAxis}]",
                UnitWidth = settings.TimelineYAxis switch
                {
                    SettingsDataModel.DataSize.Bytes => 1,
                    SettingsDataModel.DataSize.KiloBytes => 1024,
                    SettingsDataModel.DataSize.MegaBytes => 1024 * 1024,
                    _ => throw new ArgumentOutOfRangeException()
                },
                ShowSeparatorLines = false,
                Position = AxisPosition.End,
                MinLimit = -10,
            }
        ];
        profilerViewModel.TimelineXAxis =
        [
            new()
            {
                Name = $"Time [{settings.TimelineXAxis}]",
                UnitWidth = settings.TimelineXAxis switch
                {
                    SettingsDataModel.TimeSize.Second => 0.5,
                    SettingsDataModel.TimeSize.Millisecond => 1000,
                    SettingsDataModel.TimeSize.Minute => 1,
                    _ => throw new ArgumentOutOfRangeException()
                },
                MinLimit = 0,
            }
        ];
        profilerViewModel.ExceptionCountInfo = data.ExceptionsInfo
            .GroupBy(x => new {x.Value.ExceptionType, x.Value.ThreadId})
            .Select(x => new ExceptionTable(x.Key.ExceptionType, x.Key.ThreadId, x.Count()))
            .ToArray();
        profilerViewModel.Settings = new SettingsViewModel(settings);
        return profilerViewModel;
    }
    public string NetVersion { get; set; } = string.Empty;

    private static double ParseData(double data, SettingsDataModel settings)
    {
        return settings.TimelineYAxis switch
        {
            SettingsDataModel.DataSize.Bytes => data,
            SettingsDataModel.DataSize.KiloBytes => data / 1024,
            SettingsDataModel.DataSize.MegaBytes => data / (1024 * 1014),
            _ => throw new ArgumentOutOfRangeException()
        };
    }

    private static double GroupSamples(RangeDataPoint x, SettingsDataModel settings)
    {
        return Sample(x.TimeStart, settings.TimelineXAxis);
    }

    private static double GroupSamples(DataPoint x, SettingsDataModel settings)
    {
        return Sample(x.Time, settings.TimelineXAxis);
    }

    private static double Sample(ulong time, SettingsDataModel.TimeSize size)
    {
        return size switch
        {
            SettingsDataModel.TimeSize.Millisecond => Math.Round(TimeSpan.FromMicroseconds((long)time).TotalMilliseconds, 4),
            SettingsDataModel.TimeSize.Second => Math.Round(TimeSpan.FromMicroseconds((long)time).TotalSeconds, 4),
            SettingsDataModel.TimeSize.Minute => Math.Round(TimeSpan.FromMicroseconds((long)time).TotalMinutes, 4),
            _ => throw new ArgumentOutOfRangeException()
        };
    }

}

public class StackTraceComparer : IEqualityComparer<
    KeyValuePair<Guid, (string Type, List<ProfilerDataModel.StackFrame> StackTraces)>>
{
    public bool Equals(KeyValuePair<Guid, (string Type, List<ProfilerDataModel.StackFrame> StackTraces)> x,
        KeyValuePair<Guid, (string Type, List<ProfilerDataModel.StackFrame> StackTraces)> y)
    {
        return ((Object)x.Value.Type).Equals(y.Value.Type) && x.Value.StackTraces.SequenceEqual(y.Value.StackTraces);
    }

    public int GetHashCode(
        KeyValuePair<Guid, (string Type, List<ProfilerDataModel.StackFrame> StackTraces)> obj)
    {
        return HashCode.Combine(obj.Value.Type);
    }
}