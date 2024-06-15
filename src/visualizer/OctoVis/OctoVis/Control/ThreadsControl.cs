using System.Globalization;
using System.Windows;
using System.Windows.Media;
using OctoVis.Parser.v0_0_2;

namespace OctoVis.Control;

public class ThreadsControl : FrameworkElement
{
    public static readonly DependencyProperty ThreadsPerfInfoProperty = DependencyProperty.Register(
        nameof(ThreadsPerfInfo), typeof(List<LogParser.ThreadPerfInfo>), typeof(ThreadsControl), new PropertyMetadata(default(List<LogParser.ThreadPerfInfo>)));

    public List<LogParser.ThreadPerfInfo> ThreadsPerfInfo
    {
        get => (List<LogParser.ThreadPerfInfo>)GetValue(ThreadsPerfInfoProperty);
        set => SetValue(ThreadsPerfInfoProperty, value);
    }
    protected override void OnRender(DrawingContext drawingContext)
    {
        base.OnRender(drawingContext);
        if (ThreadsPerfInfo is null) return;

        var posY = 20;
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
                new SolidColorBrush(Colors.Green),
                null,
                TextFormattingMode.Display,
                72);
            drawingContext.DrawText(ft, new Point(20, posY));
            posY += 20;
        }

    }
}