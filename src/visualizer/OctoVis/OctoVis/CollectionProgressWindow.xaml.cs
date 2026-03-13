using System.ComponentModel;
using System.Windows;

namespace OctoVis;

public partial class CollectionProgressWindow : Window
{
    private readonly CancellationTokenSource _cts;

    public CollectionProgressWindow(CancellationTokenSource cts)
    {
        InitializeComponent();
        _cts = cts;
    }

    public void UpdateEventCount(int count)
    {
        EventCountText.Text = $"Collected {count:N0} events...";
    }

    private void StopButton_OnClick(object sender, RoutedEventArgs e)
    {
        _cts.Cancel();
    }

    protected override void OnClosing(CancelEventArgs e)
    {
        base.OnClosing(e);
        _cts.Cancel();
    }
}
