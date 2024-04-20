using System.ComponentModel;
using System.Diagnostics;
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
    private SettingsDataModel _settings = new();

    public MainWindow()
    {
        InitializeComponent();
        DataContext = new { LogParsed = false, LogNotParsed = true };
    }

    private void ParseFile(string fileName)
    {
        _settings = new SettingsDataModel
        {
            TimelineXAxis = SettingsDataModel.TimeSize.Millisecond,
            TimelineYAxis = SettingsDataModel.DataSize.KiloBytes,
            Filter = string.Empty
        };
        var data = LogParser.ParseFile(fileName);
        if (data is null)
        {
            return;
        }

        _data = data;
        var d = ProfilerViewModel.FromDataModel(_data, _settings);
        DataContext = d;
        d.Settings.PropertyChanged += OnPropertyChanged;
    }

    private void OnPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        var model = (SettingsViewModel)sender!;
        HandleXResolution(model, e);
        HandleYResolution(model, e);
        HandleFilter(model, e);

        var d = ProfilerViewModel.FromDataModel(_data, _settings);
        DataContext = d;
        d.Settings.PropertyChanged += OnPropertyChanged;
    }

    private void HandleFilter(SettingsViewModel model, PropertyChangedEventArgs e)
    {
        if (e.PropertyName.Contains(nameof(model.Filter)))
        {
            _settings.Filter = model.Filter;
        }
    }

    private void HandleYResolution(SettingsViewModel model, PropertyChangedEventArgs e)
    {
        if (e.PropertyName.Contains(nameof(model.SelectedYResolution)))
        {
            if (model.SelectedYResolution == nameof(SettingsDataModel.DataSize.Bytes))
            {
                _settings.TimelineYAxis = SettingsDataModel.DataSize.Bytes;
            }

            if (model.SelectedYResolution == nameof(SettingsDataModel.DataSize.KiloBytes))
            {
                _settings.TimelineYAxis = SettingsDataModel.DataSize.KiloBytes;
            }

            if (model.SelectedYResolution == nameof(SettingsDataModel.DataSize.MegaBytes))
            {
                _settings.TimelineYAxis = SettingsDataModel.DataSize.MegaBytes;
            }
        }
    }

    private void HandleXResolution(SettingsViewModel model, PropertyChangedEventArgs e)
    {
        if (e.PropertyName.Contains(nameof(model.SelectedXResolution)))
        {
            if (model.SelectedXResolution == nameof(SettingsDataModel.TimeSize.Second))
            {
                _settings.TimelineXAxis = SettingsDataModel.TimeSize.Second;
            }

            if (model.SelectedXResolution == nameof(SettingsDataModel.TimeSize.Millisecond))
            {
                _settings.TimelineXAxis = SettingsDataModel.TimeSize.Millisecond;
            }

            if (model.SelectedXResolution == nameof(SettingsDataModel.TimeSize.Minute))
            {
                _settings.TimelineXAxis = SettingsDataModel.TimeSize.Minute;
            }
        }
    }

    private void ButtonBase_OnClick(object sender, RoutedEventArgs e)
    {
        OpenFileDialog ofd = new OpenFileDialog();
        ofd.ShowDialog();
        ParseFile(ofd.FileName);
    }
}