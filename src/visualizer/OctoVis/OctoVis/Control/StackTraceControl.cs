using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;
using OctoVis.Model;

namespace OctoVis.Control;

public class StackTraceControl : FrameworkElement
{
    public enum ColorPalette
    {
        Hot,
        Cold
    }

    private record Tuple(Guid Id, string Entry, double Fraction);
    private record RectTooltip(Rect BoundingBox, string Tooltip);

    private List<RectTooltip> TooltipsCoords = new();

    private readonly Color[] FlameColors = [Colors.Orange, Colors.Yellow, Colors.Chocolate, Colors.Goldenrod, Colors.Coral];
    private readonly Color[] ColdColors = [Colors.Blue, Colors.SteelBlue, Colors.CadetBlue, Colors.Navy, Colors.MediumAquamarine];

    public static readonly DependencyProperty PaletteProperty = DependencyProperty.Register(
        nameof(Palette), typeof(ColorPalette), typeof(StackTraceControl), new PropertyMetadata(ColorPalette.Cold));

    public ColorPalette Palette
    {
        get => (ColorPalette)GetValue(PaletteProperty);
        set => SetValue(PaletteProperty, value);
    }

    public static readonly DependencyProperty ZoomProperty = DependencyProperty.Register(
        nameof(Zoom), typeof(double), typeof(StackTraceControl),
        new FrameworkPropertyMetadata(1.0,
            FrameworkPropertyMetadataOptions.AffectsRender | FrameworkPropertyMetadataOptions.AffectsArrange));

    public double Zoom
    {
        get => (double)GetValue(ZoomProperty);
        set => SetValue(ZoomProperty, value);
    }

    public static readonly DependencyProperty StackFramesProperty
        = DependencyProperty.Register(
            nameof(StackFrames), typeof(List<List<ProfilerDataModel.StackFrame>>), typeof(StackTraceControl),
            new PropertyMetadata(default(List<List<ProfilerDataModel.StackFrame>>)));

    public List<List<ProfilerDataModel.StackFrame>> StackFrames
    {
        get => (List<List<ProfilerDataModel.StackFrame>>)GetValue(StackFramesProperty);
        set => SetValue(StackFramesProperty, value);
    }

    protected override void OnRender(DrawingContext drawingContext)
    {
        base.OnRender(drawingContext);
        TooltipsCoords.Clear();
        drawingContext.DrawRectangle(new SolidColorBrush(Colors.Transparent),
            new Pen(Brushes.Transparent, 0.0), new Rect(0, 0, ActualWidth, ActualHeight));

        var elementHeight = 20 * Zoom;

        foreach (var (idx, stack) in StackFrames.Where(x=>x.Count > 0).Index())
        {
            var posY = ActualHeight - elementHeight;
            int i = 0;
            foreach (var info in stack)
            {
                var color = GetColor(i);
                var size = ActualWidth / StackFrames.Count;
                var rect = new Rect(
                    new Point(idx * size, posY),
                    new Point((idx + 1) * size, posY + elementHeight));
                drawingContext.PushClip(new RectangleGeometry(rect));
                drawingContext.DrawRectangle(new SolidColorBrush(color),
                    new Pen(new SolidColorBrush(Colors.Transparent), 2.0),
                    rect);
                TooltipsCoords.Add(new RectTooltip(rect, info.FrameInfo));

                if (idx < 1 || StackFrames.ElementAt(idx - 1).ElementAtOrDefault(i)?.FrameInfo != info.FrameInfo)
                {
                    var ft = new FormattedText(
                        info.FrameInfo,
                        CultureInfo.CurrentCulture,
                        FlowDirection.LeftToRight,
                        new Typeface(new FontFamily("Segoe UI"),
                            FontStyles.Normal,
                            FontWeights.Normal,
                            FontStretches.Normal),
                        12 * Zoom,
                        GetFontColor,
                        null,
                        TextFormattingMode.Display,
                        72);
                    drawingContext.DrawText(ft, new Point(idx * size, posY));
                }

                posY -= elementHeight;
                drawingContext.Pop();
                i++;
            }
        }
    }

    private Color GetColor(int i) => Palette == ColorPalette.Cold
        ? ColdColors[i % ColdColors.Length]
        : FlameColors[i % FlameColors.Length];

    private Brush GetFontColor => Palette == ColorPalette.Cold ? Brushes.WhiteSmoke : Brushes.Black;

    protected override HitTestResult HitTestCore(PointHitTestParameters hitTestParameters)
    {
        var tooltip = TooltipsCoords.FirstOrDefault(x => x.BoundingBox.Contains(hitTestParameters.HitPoint));
        if (tooltip is not null)
        {
            ToolTipService.SetToolTip(this, tooltip.Tooltip);
            ToolTipService.SetPlacement(this, PlacementMode.Mouse);
            return new PointHitTestResult(this, hitTestParameters.HitPoint);
        }

        ToolTipService.SetToolTip(this, null);
        return null;
    }

    protected override Size MeasureOverride(Size availableSize)
    {
        return availableSize;
    }

    protected override void OnMouseWheel(MouseWheelEventArgs e)
    {
        if (e.Delta > 0) Zoom += 0.5;
        if (e.Delta < 0) Zoom -= 0.5;

        if (Zoom > 5) Zoom = 5;
        if (Zoom < 1) Zoom = 1;
    }
}