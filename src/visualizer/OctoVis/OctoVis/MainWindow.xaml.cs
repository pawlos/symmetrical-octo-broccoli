using System.Diagnostics;
using System.IO;
using System.Windows;
using Microsoft.Win32;

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
        Hide();
        var memoryProfileWindow = new MemoryProfileWindow();
        memoryProfileWindow.ParseFile(file);
        memoryProfileWindow.Show();

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

        var profile = result == MessageBoxResult.No;
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
                    ["OCTO_PROFILER_FILE"] = fileName,
                }
            }
        };
        if (profile)
        {
            p.StartInfo.EnvironmentVariables.Add("OCTO_MONITOR_ENTERLEAVE", "1");
        }

        Hide();
        p.Start();

        await p.WaitForExitAsync();
        if (profile)
        {
            var window = new PerformanceProfileWindow();
            window.ParseFile(fileName);
            window.Show();
        }
        else
        {
            var window = new MemoryProfileWindow();
            window.ParseFile(fileName);
            window.Show();
        }
    }
}