using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using LiveChartsCore;
using LiveChartsCore.Defaults;
using LiveChartsCore.SkiaSharpView;
using LiveChartsCore.SkiaSharpView.Painting;
using OctoVis.Parser;
using OctoVis.ViewModel;
using SkiaSharp;

namespace OctoVis;

/// <summary>
/// Interaction logic for MainWindow.xaml
/// </summary>
public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        var profilerViewModel = new ProfilerViewModel();
        var data = LogParser.ParseFile("log2.txt");
        if (data is null)
        {
            return;
        }

        (ObservablePoint[] memoryData, ObservablePoint[] exceptionData, Dictionary<double, string> exceptionInfo)
            = data.Value;
        profilerViewModel.Memory = new[]
        {
            new LineSeries<ObservablePoint>
            {
                Values = memoryData
            } as ISeries<ObservablePoint>,
            new ScatterSeries<ObservablePoint>
            {
                Values = exceptionData,
                YToolTipLabelFormatter = point =>  exceptionInfo[point.Model!.X!.Value],
                Fill = new SolidColorPaint(SKColor.Parse("FF0000"))
            }
        };
        DataContext = profilerViewModel;
    }
}