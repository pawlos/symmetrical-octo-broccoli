using System.ComponentModel;
using System.Runtime.CompilerServices;
using LiveChartsCore;
using LiveChartsCore.SkiaSharpView;
using LiveChartsCore.SkiaSharpView.Painting;
using LiveChartsCore.SkiaSharpView.VisualElements;
using SkiaSharp;

namespace OctoVis.ViewModel;

public sealed class ProfilerViewModel : INotifyPropertyChanged
{
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

    public bool LogParsed { get; set; }= false;
    public bool LogNotParsed { get; set; } = true;
}