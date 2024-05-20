using System.Globalization;
using System.Windows;
using System.Windows.Media;
using OctoVis.Parser.v0_0_2;

namespace OctoVis.Control;

public class TraceCallsControl : FrameworkElement
{
    public static readonly DependencyProperty EnterExitCallsProperty = DependencyProperty.Register(
        nameof(EnterExitCalls), typeof(IEnumerable<LogParser.EnterExitEntry>), typeof(TraceCallsControl),
        new FrameworkPropertyMetadata(default(IEnumerable<LogParser.EnterExitEntry>),
            FrameworkPropertyMetadataOptions.AffectsRender));

    public IEnumerable<LogParser.EnterExitEntry> EnterExitCalls
    {
        get => (IEnumerable<LogParser.EnterExitEntry>)GetValue(EnterExitCallsProperty);
        set => SetValue(EnterExitCallsProperty, value);
    }

    protected override void OnRender(DrawingContext drawingContext)
    {
        base.OnRender(drawingContext);
        if (!EnterExitCalls?.Any() ?? true) return;

        var longest = EnterExitCalls.MaxBy(x => x.Duration);
        var brush = new SolidColorBrush(Colors.Brown);
        drawingContext.DrawRectangle(brush, new Pen(Brushes.Tomato, 1.0),
            new Rect(10, 10, ActualWidth-20, 30));
        var ft = new FormattedText(
            $"{longest.Class}.{longest.MethodName}",
            CultureInfo.CurrentCulture,
            FlowDirection.LeftToRight,
            new Typeface(new FontFamily("Segoe UI"),
                FontStyles.Normal,
                FontWeights.Normal,
                FontStretches.Normal),
            20,
            new SolidColorBrush(Colors.Black),
            null,
            TextFormattingMode.Display,
            72);
        drawingContext.DrawText(ft, new Point(15, 10));
    }
}