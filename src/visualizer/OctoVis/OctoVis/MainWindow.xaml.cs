using System.Diagnostics;
using System.IO;
using System.Windows;
using Microsoft.Win32;
using OctoVis.Model;
using OctoVis.Parser;

namespace OctoVis;

/// <summary>
/// Interaction logic for MainWindow.xaml
/// </summary>
public partial class MainWindow
{
    private string? _profilerFile;

    public MainWindow()
    {
        InitializeComponent();
        DataContext = new { LogParsed = false, LogNotParsed = true };
    }

    private void OpenLog_OnClick(object sender, RoutedEventArgs e)
    {
        string file = PickFile("Open log file", "Log files|*.txt");

        if (string.IsNullOrWhiteSpace(file))
            return;

        if (CreateProfilingWindow(file))
        {
            Hide();
        }
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

        var result = MessageBox.Show("Profile for memory?", "Profile type?", MessageBoxButton.YesNo);

        var profileType = result == MessageBoxResult.No;
        var fileName = $"{Guid.NewGuid().ToString()}.txt";
        var program = PickFile("Open application to profile", "Program|*.exe");
        if (string.IsNullOrWhiteSpace(program))
        {
            MessageBox.Show("Could not find program exe.");
            return;
        }
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
                    ["COR_PROFILER"] = "{10B46309-C972-4F33-B5AB-5E6E3EBA2B1A}",
                    ["CORECLR_PROFILER"] = "{10B46309-C972-4F33-B5AB-5E6E3EBA2B1A}",
                    ["COR_PROFILER_PATH"] = _profilerFile,
                    ["CORECLR_PROFILER_PATH_64"] = _profilerFile,
                    ["OCTO_PROFILER_FILE"] = fileName,
                }
            }
        };
        if (profileType)
        {
            p.StartInfo.EnvironmentVariables.Add("OCTO_MONITOR_ENTERLEAVE", "true");
        }

        Hide();
        p.Start();

        await p.WaitForExitAsync();
        CreateProfilingWindow(fileName);
    }

    private static bool CreateProfilingWindow(string fileName)
    {
        var data = LogParser.ParseFile(fileName);
        if (data is null)
        {
            MessageBox.Show("Could not parse the log file.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            return false;
        }
        IProfilingWindow? profilingWindow;
        if (data is PerformanceDataModel)
        {
            profilingWindow = new PerformanceProfileWindow();
        }
        else
        {
            profilingWindow = new MemoryProfileWindow();
        }
        profilingWindow.SetModel(data);
        profilingWindow.Show();
        return true;
    }
}