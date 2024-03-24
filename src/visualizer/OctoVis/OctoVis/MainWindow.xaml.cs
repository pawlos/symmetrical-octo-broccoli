using System.Windows;
using LiveChartsCore;
using LiveChartsCore.Defaults;
using LiveChartsCore.SkiaSharpView;
using LiveChartsCore.SkiaSharpView.Painting;
using Microsoft.Win32;
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
        DataContext = new { LogParsed = false, LogNotParsed = true };
    }

    private void ParseFile(string fileName)
    {
        var profilerViewModel = new ProfilerViewModel();
        var data = LogParser.ParseFile(fileName);
        if (data is null)
        {
            return;
        }

        profilerViewModel.Timeline = new[]
        {
            new LineSeries<ObservablePoint>
            {
                Values = data.MemoryData.Select(x => new ObservablePoint(x.Time, x.Value))
            } as ISeries<ObservablePoint>,
            new ScatterSeries<ObservablePoint>
            {
                Values = data.ExceptionData.Select(x=> new ObservablePoint(x.Time, x.Value)),
                YToolTipLabelFormatter = point =>  data.ExceptionInfo[point.Model!.X!.Value],
                Fill = new SolidColorPaint(SKColor.Parse("FF0000"))
            }
        };
        var series = data.TypeAllocationInfo
            .OrderByDescending(x=>x.Value)
            .Take(20)
            .Select(x => new PieSeries<double>
            {
                Values = new [] { (double)x.Value },
                Name = x.Key
            }).ToArray();
        profilerViewModel.MemoryByType = series as ISeries?[];
        profilerViewModel.LogParsed = true;
        profilerViewModel.LogNotParsed = false;
        DataContext = profilerViewModel;
    }


    private void ButtonBase_OnClick(object sender, RoutedEventArgs e)
    {
        OpenFileDialog ofd = new OpenFileDialog();
        ofd.ShowDialog();
        ParseFile(ofd.FileName);
    }
}