using System.ComponentModel;
using System.Runtime.CompilerServices;
using LiveChartsCore;
using LiveChartsCore.Defaults;
using LiveChartsCore.SkiaSharpView;
using LiveChartsCore.SkiaSharpView.Painting;
using LiveChartsCore.SkiaSharpView.VisualElements;
using OctoVis.Model;
using SkiaSharp;

namespace OctoVis.ViewModel;

public sealed class ProfilerViewModel : INotifyPropertyChanged
{
    public record TypeAllocationsTable(string Name, uint TotalAllocations, uint Count);

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

    public Axis[] TimelineYAxis { get; set; } = new Axis[]
    {
        new Axis
        {
            Name = "Total memory allocations [kB]",
        }
    };
    public ISeries[] Timeline { get; set; } = null!;
    public ISeries?[] MemoryByType { get; set; } = null!;

    public RectangularSection[] GcSections { get; set; } = null!;
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

    public string TotalTime { get; set; } = string.Empty;

    public string TotalGcTime { get; set; } = string.Empty;
    public bool LogParsed { get; set; }= false;
    public bool LogNotParsed { get; set; } = true;

    public static ProfilerViewModel FromDataModel(
        ProfilerDataModel data,
        SettingsDataModel settings)
    {
        var profilerViewModel = new ProfilerViewModel
        {
            Timeline = new[]
            {
                new LineSeries<ObservablePoint>
                {
                    Values = data.MemoryData.Select(x => new ObservablePoint(x.Time, x.Value))
                } as ISeries<ObservablePoint>,
                new ScatterSeries<ObservablePoint>
                {
                    Values = data.ExceptionData.Select(x=> new ObservablePoint(x.Time, x.Value)),
                    XToolTipLabelFormatter = point =>  data.ExceptionInfo[point.Model!.X!.Value],
                    Fill = new SolidColorPaint(SKColor.Parse("FF0000"))
                }
            }
        };
        var gcSections = data.GcData.Select(gc => new RectangularSection
        {
            Xi = gc.TimeStart,
            Xj = gc.TimeStart + gc.Duration,
            Fill = new SolidColorPaint {Color = SKColors.Orange.WithAlpha(80) },
            Stroke = new SolidColorPaint {Color = SKColors.Orange, StrokeThickness = 3},
            Label = "GC",
            LabelSize = 200
        }).ToArray();
        profilerViewModel.GcSections = gcSections;
        var series = data.TypeAllocationInfo
            .OrderByDescending(x=>x.Value)
            .SkipWhile(x=>x.Key == "<<no info>>")
            .Take(20)
            .Select(x => new PieSeries<double>
            {
                Values = new [] { (double)(x.Value.TotalMemory) },
                Name = x.Key
            }).ToArray();
        profilerViewModel.MemoryByType = series as ISeries?[];
        profilerViewModel.TotalGcTime = data.TotalGcTime.ToString();
        profilerViewModel.TotalTime = data.TotalTime.ToString();
        profilerViewModel.NetVersion = data.NetVersion;
        profilerViewModel.LogParsed = true;
        profilerViewModel.LogNotParsed = false;

        profilerViewModel.TypeAllocationInfo =
            data.TypeAllocationInfo
                .Select(x => new TypeAllocationsTable(x.Key, x.Value.TotalMemory, x.Value.Count))
                .ToArray();
        return profilerViewModel;
    }

    public string NetVersion { get; set; }
}