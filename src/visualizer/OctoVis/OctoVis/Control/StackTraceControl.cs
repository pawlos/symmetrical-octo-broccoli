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

    public static readonly DependencyProperty HorizontalZoomProperty = DependencyProperty.Register(
        nameof(HorizontalZoom), typeof(double), typeof(StackTraceControl),
        new FrameworkPropertyMetadata(1.0,
            FrameworkPropertyMetadataOptions.AffectsRender | FrameworkPropertyMetadataOptions.AffectsMeasure));

    public double HorizontalZoom
    {
        get => (double)GetValue(HorizontalZoomProperty);
        set => SetValue(HorizontalZoomProperty, value);
    }

    private bool _isPanning;
    private Point _panStart;

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
        ToolTipService.SetToolTip(this, tooltip?.Tooltip);
        return new PointHitTestResult(this, hitTestParameters.HitPoint);
    }

    protected override Size MeasureOverride(Size availableSize)
    {
        var scrollViewer = FindParentScrollViewer();
        var baseWidth = scrollViewer is { ViewportWidth: > 0 } ? scrollViewer.ViewportWidth :
            (double.IsInfinity(availableSize.Width) ? 800 : availableSize.Width);
        var baseHeight = scrollViewer is { ViewportHeight: > 0 } ? scrollViewer.ViewportHeight :
            (double.IsInfinity(availableSize.Height) ? 450 : availableSize.Height);

        var contentHeight = GetMaxDepth(StackFrames) * 20 * Zoom;

        return new Size(baseWidth * HorizontalZoom, Math.Max(baseHeight, contentHeight));
    }

    private static int GetMaxDepth(FlameGraphNode? node)
    {
        if (node is null || node.Children.Count == 0) return 1;
        var max = 0;
        foreach (var child in node.Children)
        {
            var d = GetMaxDepth(child);
            if (d > max) max = d;
        }
        return max + 1;
    }

    protected override void OnMouseWheel(MouseWheelEventArgs e)
    {
        if (Keyboard.Modifiers.HasFlag(ModifierKeys.Control))
        {
            if (e.Delta > 0) HorizontalZoom += 0.5;
            if (e.Delta < 0) HorizontalZoom -= 0.5;

            if (HorizontalZoom > 5) HorizontalZoom = 5;
            if (HorizontalZoom < 1) HorizontalZoom = 1;
        }
        else
        {
            if (e.Delta > 0) Zoom += 0.5;
            if (e.Delta < 0) Zoom -= 0.5;

            if (Zoom > 5) Zoom = 5;
            if (Zoom < 1) Zoom = 1;
        }

        e.Handled = true;
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

    protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e)
    {
        if (Keyboard.Modifiers.HasFlag(ModifierKeys.Shift))
        {
            var scrollViewer = FindParentScrollViewer();
            if (scrollViewer is not null)
            {
                _isPanning = true;
                _panStart = e.GetPosition(scrollViewer);
                Cursor = Cursors.Hand;
                CaptureMouse();
                e.Handled = true;
            }
        }

        base.OnMouseLeftButtonDown(e);
    }

    protected override void OnMouseMove(MouseEventArgs e)
    {
        if (_isPanning)
        {
            var scrollViewer = FindParentScrollViewer();
            if (scrollViewer is not null)
            {
                var current = e.GetPosition(scrollViewer);
                var deltaX = _panStart.X - current.X;
                var deltaY = _panStart.Y - current.Y;
                scrollViewer.ScrollToHorizontalOffset(scrollViewer.HorizontalOffset + deltaX);
                scrollViewer.ScrollToVerticalOffset(scrollViewer.VerticalOffset + deltaY);
                _panStart = current;
                e.Handled = true;
            }
        }

        base.OnMouseMove(e);
    }

    protected override void OnMouseLeftButtonUp(MouseButtonEventArgs e)
    {
        if (_isPanning)
        {
            _isPanning = false;
            Cursor = Cursors.Arrow;
            ReleaseMouseCapture();
            e.Handled = true;
        }

        base.OnMouseLeftButtonUp(e);
    }

    private ScrollViewer? FindParentScrollViewer()
    {
        DependencyObject? current = this;
        while (current is not null)
        {
            current = VisualTreeHelper.GetParent(current);
            if (current is ScrollViewer sv) return sv;
        }

        return null;
    }
}
