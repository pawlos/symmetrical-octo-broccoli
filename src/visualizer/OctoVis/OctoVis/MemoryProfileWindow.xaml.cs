using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using OctoVis.Model;
using OctoVis.ViewModel;
using OctoVis.Parser;

namespace OctoVis;

public partial class MemoryProfileWindow : Window
{
    private ProfilerDataModel _data = new();
    private SettingsDataModel _settings = new();
    public MemoryProfileWindow()
    {
        InitializeComponent();
    }

    public void ParseFile(string fileName)
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


    private void Control_OnMouseDoubleClick(object sender, MouseButtonEventArgs e)
    {
        if (ItemsControl.ContainerFromElement((DataGrid)sender, e.OriginalSource as DependencyObject) is not DataGridRow
            row) return;

        if (row.DataContext is not ProfilerViewModel.TypeAllocationsEntry tat) return;

        var stw = new StackTraceWindow
        {
            DataContext = tat
        };

        stw.ShowDialog();
    }
}