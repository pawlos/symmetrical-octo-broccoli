using System.Windows;
using LiveChartsCore;
using LiveChartsCore.Defaults;
using LiveChartsCore.SkiaSharpView;
using LiveChartsCore.SkiaSharpView.Painting;
using Microsoft.Win32;
using OctoVis.Model;
using OctoVis.Parser;
using OctoVis.ViewModel;
using SkiaSharp;

namespace OctoVis;

/// <summary>
/// Interaction logic for MainWindow.xaml
/// </summary>
public partial class MainWindow : Window
{
    private ProfilerDataModel _data = new();
    public MainWindow()
    {
        InitializeComponent();
        DataContext = new { LogParsed = false, LogNotParsed = true };
    }

    private void ParseFile(string fileName)
    {
        var data = LogParser.ParseFile(fileName);
        if (data is null)
        {
            return;
        }

        _data = data;
        DataContext = ProfilerViewModel.FromDataModel(_data, new SettingsDataModel());
    }


    private void ButtonBase_OnClick(object sender, RoutedEventArgs e)
    {
        OpenFileDialog ofd = new OpenFileDialog();
        ofd.ShowDialog();
        ParseFile(ofd.FileName);
    }
}