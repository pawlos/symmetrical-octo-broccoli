using System.Windows;
using OctoVis.Model;
using OctoVis.Parser;

namespace OctoVis;

public partial class PerformanceProfileWindow : Window
{
    private ProfilerDataModel _data;

    public PerformanceProfileWindow()
    {
        InitializeComponent();
    }

    public void ParseFile(string fileName)
    {
        var data = LogParser.ParseFile(fileName);
        if (data is null)
        {
            return;
        }

        _data = data;
        var d = _data;
        DataContext = d;
    }
}