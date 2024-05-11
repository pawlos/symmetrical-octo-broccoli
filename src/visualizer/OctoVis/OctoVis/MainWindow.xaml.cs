using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
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
    private string? _profilerFile;
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

    private void OpenLog_OnClick(object sender, RoutedEventArgs e)
    {
        ParseFile(PickFile("Open log file", "Log files|*.txt"));
    }

    private string PickFile(string title, string allowedExtension)
    {
        var ofd = new OpenFileDialog
        {
            Filter = allowedExtension,
            Title = title
        };
        ofd.ShowDialog();
        return ofd.FileName;
    }

    private async void ProfileApp_OnClick(object sender, RoutedEventArgs e)
    {
        if (!File.Exists("OctoProfiler.dll"))
        {
            _profilerFile = PickFile("Find profiler dll", "Library|*.dll");
        }

        if (string.IsNullOrWhiteSpace(_profilerFile))
        {
            MessageBox.Show("Could not find profiler dll.");
            return;
        }
        var fileName = $"{Guid.NewGuid().ToString()}.txt";
        var program = PickFile("Open application to profile", "Program|*.exe");
        var p = new Process
        {
            StartInfo =
            {
                FileName = program,
                EnvironmentVariables =
                {
                    ["DOTNET_EnableDiagnostics"] = "1",
                    ["COR_ENABLE_PROFILING"] = "1",
                    ["CORECLR_ENABLE_PROFILING"] = "1",
                    ["COR_PROFILER"] = "{8A8CC829-CCF2-49FE-BBAE-0F022228071A}",
                    ["CORECLR_PROFILER"] = "{8A8CC829-CCF2-49FE-BBAE-0F022228071A}",
                    ["COR_PROFILER_PATH"] = _profilerFile,
                    ["CORECLR_PROFILER_PATH_64"] = _profilerFile,
                    ["OCTO_PROFILER_FILE"] = fileName
                }
            }
        };
        Hide();
        p.Start();

        await p.WaitForExitAsync();
        Show();
        ParseFile(fileName);
    }

    private void Control_OnMouseDoubleClick(object sender, MouseButtonEventArgs e)
    {
        if (ItemsControl.ContainerFromElement((DataGrid)sender, e.OriginalSource as DependencyObject) is not DataGridRow row) return;

        if (row.DataContext is not ProfilerViewModel.TypeAllocationsEntry tat) return;

        var stw = new StackTraceWindow
        {
            DataContext = tat
        };

        stw.ShowDialog();
    }
}