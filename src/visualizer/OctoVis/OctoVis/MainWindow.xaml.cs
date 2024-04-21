using System.ComponentModel;
using System.Windows;
using Microsoft.Win32;
using OctoVis.Model;
using OctoVis.Parser;
using OctoVis.ViewModel;

namespace OctoVis;

/// <summary>
/// Interaction logic for MainWindow.xaml
/// </summary>
public partial class MainWindow
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
        if (e.PropertyName is null)
            return;
        if (e.PropertyName.Contains(nameof(model.Filter)))
        {
            _settings.Filter = model.Filter;
        }
    }

    private void HandleYResolution(SettingsViewModel model, PropertyChangedEventArgs e)
    {
        if (e.PropertyName is null)
            return;
        if (!e.PropertyName.Contains(nameof(model.SelectedYResolution))) return;
        _settings.TimelineYAxis = model.SelectedYResolution switch
        {
            nameof(SettingsDataModel.DataSize.Bytes) => SettingsDataModel.DataSize.Bytes,
            nameof(SettingsDataModel.DataSize.KiloBytes) => SettingsDataModel.DataSize.KiloBytes,
            nameof(SettingsDataModel.DataSize.MegaBytes) => SettingsDataModel.DataSize.MegaBytes,
            _ => _settings.TimelineYAxis
        };
    }

    private void HandleXResolution(SettingsViewModel model, PropertyChangedEventArgs e)
    {
        if (e.PropertyName is null)
            return;

        if (!e.PropertyName.Contains(nameof(model.SelectedXResolution))) return;
        _settings.TimelineXAxis = model.SelectedXResolution switch
        {
            nameof(SettingsDataModel.TimeSize.Second) => SettingsDataModel.TimeSize.Second,
            nameof(SettingsDataModel.TimeSize.Millisecond) => SettingsDataModel.TimeSize.Millisecond,
            nameof(SettingsDataModel.TimeSize.Minute) => SettingsDataModel.TimeSize.Minute,
            _ => _settings.TimelineXAxis
        };
    }

    private void ButtonBase_OnClick(object sender, RoutedEventArgs e)
    {
        var ofd = new OpenFileDialog();
        ofd.ShowDialog();
        ParseFile(ofd.FileName);
    }
}