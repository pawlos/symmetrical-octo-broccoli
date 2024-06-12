using System.ComponentModel;
using System.Windows;
using OctoVis.Model;
using OctoVis.Parser;

namespace OctoVis;

public partial class PerformanceProfileWindow : Window, IProfilingWindow
{
    private PerformanceDataModel? _data;

    public PerformanceProfileWindow()
    {
        InitializeComponent();
    }

    public void SetModel(IDataModel model)
    {
        _data = model as PerformanceDataModel;
        var d = _data;
        DataContext = d;
    }

    protected override void OnClosing(CancelEventArgs e)
    {
        base.OnClosing(e);
        Application.Current.MainWindow?.Close();
    }
}