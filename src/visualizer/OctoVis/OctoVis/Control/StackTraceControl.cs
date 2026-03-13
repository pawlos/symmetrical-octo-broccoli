using System.Globalization;
using System.Windows;
using System.Windows.Controls;
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

    private record RectTooltip(Rect BoundingBox, string Tooltip);

    private readonly List<RectTooltip> _tooltipsCoords = new();

    private readonly SolidColorBrush[] _flameColorsBrushes = [
        new SolidColorBrush(Colors.Orange),
        new SolidColorBrush(Colors.Yellow),
        new SolidColorBrush(Colors.Chocolate),
        new SolidColorBrush(Colors.Goldenrod),
        new SolidColorBrush(Colors.Coral)];
    private readonly SolidColorBrush[] _coldColorsBrushes = [
        new SolidColorBrush(Colors.Blue),
        new SolidColorBrush(Colors.SteelBlue),
        new SolidColorBrush(Colors.CadetBlue),
        new SolidColorBrush(Colors.Navy),
        new SolidColorBrush(Colors.MediumAquamarine)];

    private readonly SolidColorBrush _transparentBrush = new(Colors.Transparent);

    private readonly Typeface _typeface = new Typeface(new FontFamily("Segoe UI"),
        FontStyles.Normal,
        FontWeights.Normal,
        FontStretches.Normal);

    public static readonly DependencyProperty PaletteProperty = DependencyProperty.Register(
        nameof(Palette), typeof(ColorPalette), typeof(StackTraceControl),
        new FrameworkPropertyMetadata(ColorPalette.Cold, FrameworkPropertyMetadataOptions.AffectsRender));

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
            nameof(StackFrames), typeof(FlameGraphNode), typeof(StackTraceControl),
            new PropertyMetadata(null, (d, _) => ((StackTraceControl)d).InvalidateVisual()));

    public FlameGraphNode? StackFrames
    {
        get => (FlameGraphNode?)GetValue(StackFramesProperty);
        set => SetValue(StackFramesProperty, value);
    }

    protected override void OnRender(DrawingContext drawingContext)
    {
        base.OnRender(drawingContext);
        _tooltipsCoords.Clear();
        drawingContext.DrawRectangle(_transparentBrush,
            new Pen(Brushes.Transparent, 0.0), new Rect(0, 0, ActualWidth, ActualHeight));

        var root = StackFrames;
        if (root is null || root.SampleCount == 0) return;

        var elementHeight = 20 * Zoom;
        var childX = 0.0;

        foreach (var child in root.Children)
        {
            var childWidth = ActualWidth * child.SampleCount / root.SampleCount;
            RenderNode(drawingContext, child, childX, childWidth, 0, elementHeight);
            childX += childWidth;
        }
    }

    private void RenderNode(DrawingContext dc, FlameGraphNode node, double x, double width, int depth, double elementHeight)
    {
        if (width < 1.0) return;

        var y = ActualHeight - (depth + 1) * elementHeight;
        var rect = new Rect(x, y, width, elementHeight);

        var brush = GetBrush(depth);
        dc.PushClip(new RectangleGeometry(rect));
        dc.DrawRectangle(brush, new Pen(_transparentBrush, 1.0), rect);

        var label = node.Frame?.FrameInfo ?? string.Empty;
        _tooltipsCoords.Add(new RectTooltip(rect, $"{label} ({node.SampleCount} sample{(node.SampleCount != 1 ? "s" : "")})"));

        if (width > 40 && !string.IsNullOrEmpty(label))
        {
            var ft = new FormattedText(
                label,
                CultureInfo.CurrentCulture,
                FlowDirection.LeftToRight,
                _typeface,
                12 * Zoom,
                GetFontColor,
                null,
                TextFormattingMode.Display,
                72);
            ft.MaxTextWidth = Math.Max(1, width - 4);
            ft.MaxTextHeight = elementHeight;
            ft.Trimming = TextTrimming.CharacterEllipsis;
            dc.DrawText(ft, new Point(x + 2, y));
        }

        dc.Pop();

        if (node.SampleCount == 0) return;

        var childX = x;
        foreach (var child in node.Children)
        {
            var childWidth = width * child.SampleCount / node.SampleCount;
            RenderNode(dc, child, childX, childWidth, depth + 1, elementHeight);
            childX += childWidth;
        }
    }

    private SolidColorBrush GetBrush(int i) => Palette == ColorPalette.Cold
        ? _coldColorsBrushes[i % _coldColorsBrushes.Length]
        : _flameColorsBrushes[i % _flameColorsBrushes.Length];

    private Brush GetFontColor => Palette == ColorPalette.Cold ? Brushes.WhiteSmoke : Brushes.Black;

    protected override HitTestResult HitTestCore(PointHitTestParameters hitTestParameters)
    {
        var tooltip = _tooltipsCoords.FirstOrDefault(x => x.BoundingBox.Contains(hitTestParameters.HitPoint));
        if (tooltip is not null)
        {
            ToolTipService.SetToolTip(this, tooltip.Tooltip);
            return new PointHitTestResult(this, hitTestParameters.HitPoint);
        }

        ToolTipService.SetToolTip(this, null);
        #nullable disable
        return null;
        #nullable restore
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

    protected override void OnMouseRightButtonUp(MouseButtonEventArgs e)
    {
        if (e.ButtonState == MouseButtonState.Released)
        {
            Palette = Palette == ColorPalette.Hot ? ColorPalette.Cold : ColorPalette.Hot;
            e.Handled = true;
        }

        base.OnMouseRightButtonUp(e);
    }
}
