using System.Windows;
using System.Windows.Media;
using OctoVis.Model;

namespace OctoVis.Control;

public class StackTraceControl : FrameworkElement
{
    public static readonly DependencyProperty ZoomProperty = DependencyProperty.Register(
        nameof(Zoom), typeof(double), typeof(StackTraceControl),
        new FrameworkPropertyMetadata(1.0, FrameworkPropertyMetadataOptions.AffectsRender | FrameworkPropertyMetadataOptions.AffectsMeasure));

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
    }

    protected override Size MeasureOverride(Size availableSize)
    {
        return availableSize;
    }
}