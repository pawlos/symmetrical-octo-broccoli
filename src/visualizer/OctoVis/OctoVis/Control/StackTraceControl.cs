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
        _tooltipsCoords.Clear();
        drawingContext.DrawRectangle(_transparentBrush,
            new Pen(Brushes.Transparent, 0.0), new Rect(0, 0, ActualWidth, ActualHeight));

        var elementHeight = 20 * Zoom;

        foreach (var (idx, stack) in StackFrames.Where(x=>x.Count > 0).Index())
        {
            var posY = ActualHeight - elementHeight;
            int i = 0;
            foreach (var info in stack)
            {
                var brush = GetBrush(i);
                var size = ActualWidth / StackFrames.Count;
                var rect = new Rect(
                    new Point(idx * size, posY),
                    new Point((idx + 1) * size, posY + elementHeight));
                drawingContext.PushClip(new RectangleGeometry(rect));
                drawingContext.DrawRectangle(brush,
                    new Pen(_transparentBrush, 2.0),
                    rect);
                _tooltipsCoords.Add(new RectTooltip(rect, info.FrameInfo));

                if (idx < 1 || StackFrames.ElementAt(idx - 1).ElementAtOrDefault(i)?.FrameInfo != info.FrameInfo)
                {
                    var ft = new FormattedText(
                        info.FrameInfo,
                        CultureInfo.CurrentCulture,
                        FlowDirection.LeftToRight,
                        _typeface,
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