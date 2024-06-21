using System.Globalization;
using System.Windows;
using System.Windows.Media;
using OctoVis.Parser.v0_0_2;

namespace OctoVis.Control;

public class ThreadsControl : FrameworkElement
{
    private readonly Typeface _typeface = new(new FontFamily("Segoe UI"),
        FontStyles.Normal,
        FontWeights.Normal,
        FontStretches.Normal);

    private readonly SolidColorBrush _transparentBrush = new(Colors.Transparent);

    public static readonly DependencyProperty ThreadsPerfInfoProperty = DependencyProperty.Register(
        nameof(ThreadsPerfInfo), typeof(List<LogParser.ThreadPerfInfo>), typeof(ThreadsControl),
        new FrameworkPropertyMetadata(default(List<LogParser.ThreadPerfInfo>?), FrameworkPropertyMetadataOptions.AffectsRender));

    public List<LogParser.ThreadPerfInfo>? ThreadsPerfInfo
    {
        get => (List<LogParser.ThreadPerfInfo>)GetValue(ThreadsPerfInfoProperty);
        set => SetValue(ThreadsPerfInfoProperty, value);
    }

    public static readonly DependencyProperty ForegroundProperty = DependencyProperty.Register(
        nameof(Foreground), typeof(Color), typeof(ThreadsControl), new FrameworkPropertyMetadata(
            Colors.Green, FrameworkPropertyMetadataOptions.AffectsRender));

    public Color Foreground
    {
        get => (Color)GetValue(ForegroundProperty);
        set => SetValue(ForegroundProperty, value);
    }

    public static readonly DependencyProperty GraphFillColorProperty = DependencyProperty.Register(
        nameof(GraphFillColor), typeof(Color), typeof(ThreadsControl), new FrameworkPropertyMetadata(
            Colors.Red, FrameworkPropertyMetadataOptions.AffectsRender));

    public Color GraphFillColor
    {
        get => (Color)GetValue(GraphFillColorProperty);
        set => SetValue(GraphFillColorProperty, value);
    }

    protected override void OnRender(DrawingContext drawingContext)
    {
        base.OnRender(drawingContext);
        if (ThreadsPerfInfo is null or { Count: 0}) return;

        var minTimestamp = ThreadsPerfInfo.SelectMany(x => x.Time).MinBy(x => x);
        var maxTimestamp = ThreadsPerfInfo.SelectMany(x => x.Time).MaxBy(x => x);

        var posY = 20;
        var lineHeight = 20;
        var borderBrush = new SolidColorBrush(Foreground);
        var borderPen = new Pen(borderBrush, 1.0);
        var fillPen = new Pen(new SolidColorBrush(GraphFillColor), 1.0);
        foreach (var perfInfo in ThreadsPerfInfo)
        {
            var ft = new FormattedText(
                perfInfo.ThreadId,
                CultureInfo.CurrentCulture,
                FlowDirection.LeftToRight,
                _typeface,
                12,
                borderBrush,
                null,
                TextFormattingMode.Display,
                72);
            drawingContext.DrawText(ft, new Point(20, posY));
            var width = ActualWidth - (20 + ft.Width + 20);
            var left = 20 + ft.Width + 10;
            drawingContext.DrawRectangle(_transparentBrush, borderPen, new Rect(
                left, posY, width, lineHeight));
            var oldPos = -1.0d;
            foreach (var tick in perfInfo.Time)
            {
                var top = (tick - minTimestamp);
                var bottom = (float)(maxTimestamp - minTimestamp);
                var pos = left + width * (top / bottom);

                if (!(Math.Abs(oldPos - pos) >= 1)) continue;
                drawingContext.DrawLine(fillPen,
                    new Point(pos, posY), new Point(pos, posY + 20));
                oldPos = pos;
            }
            posY += 30;
        }

    }
}