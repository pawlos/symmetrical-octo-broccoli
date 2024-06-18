using System.Globalization;
using System.Windows;
using System.Windows.Media;
using OctoVis.Parser.v0_0_2;

namespace OctoVis.Control;

public class ThreadsControl : FrameworkElement
{
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

    protected override void OnRender(DrawingContext drawingContext)
    {
        base.OnRender(drawingContext);
        if (ThreadsPerfInfo is null or { Count: 0}) return;

        var minTimestamp = ThreadsPerfInfo.SelectMany(x => x.Time).MinBy(x => x);
        var maxTimestamp = ThreadsPerfInfo.SelectMany(x => x.Time).MaxBy(x => x);

        var posY = 20;
        var lineHeight = 20;
        foreach (var perfInfo in ThreadsPerfInfo)
        {
            var ft = new FormattedText(
                perfInfo.ThreadId,
                CultureInfo.CurrentCulture,
                FlowDirection.LeftToRight,
                new Typeface(new FontFamily("Segoe UI"),
                    FontStyles.Normal,
                    FontWeights.Normal,
                    FontStretches.Normal),
                12,
                new SolidColorBrush(Foreground),
                null,
                TextFormattingMode.Display,
                72);
            drawingContext.DrawText(ft, new Point(20, posY));
            var scb = new SolidColorBrush(Colors.Transparent);
            var width = ActualWidth - (20 + ft.Width + 20);
            var left = 20 + ft.Width + 10;
            drawingContext.DrawRectangle(scb, new Pen(new SolidColorBrush(Foreground), 1.0), new Rect(
                left, posY, width, lineHeight));
            foreach (var tick in perfInfo.Time)
            {
                var top = (tick - minTimestamp);
                var bottom = (float)(maxTimestamp - minTimestamp);
                var pos = left + width * (top / bottom);

                drawingContext.DrawLine(new Pen(new SolidColorBrush(Colors.Red), 1.0),
                    new Point(pos, posY), new Point(pos, posY + 20));
            }
            posY += 30;
        }

    }
}