using System.ComponentModel;
using System.Runtime.CompilerServices;
using OctoVis.Model;

namespace OctoVis.ViewModel;

public class SettingsViewModel : INotifyPropertyChanged
{
    private readonly SettingsDataModel _model;

    public SettingsViewModel(SettingsDataModel model)
    {
        _model = model;
        Filter = model.Filter;
        SelectedXResolution = model.TimelineXAxis.ToString();
        SelectedYResolution = model.TimelineYAxis.ToString();
    }
    public List<string> TimelineXValue { get; set; } = [nameof(SettingsDataModel.TimeSize.Decisecond), nameof(SettingsDataModel.TimeSize.Second), nameof(SettingsDataModel.TimeSize.Millisecond)];
    public List<string> TimelineYValue { get; set; } = [nameof(SettingsDataModel.DataSize.Bytes), nameof(SettingsDataModel.DataSize.KiloBytes), nameof(SettingsDataModel.DataSize.MegaBytes)];

    private string? _selectedXResolution;
    public string? SelectedXResolution
    {
        get => _selectedXResolution;
        set => SetField(ref _selectedXResolution, value);
    }

    private string _filter;
    public string Filter
    {
        get => _filter;
        set => SetField(ref _filter, value);
    }

    private string? _selectedYResolution;
    public string? SelectedYResolution
    {
        get => _selectedYResolution;
        set => SetField(ref _selectedYResolution, value);
    }

    public event PropertyChangedEventHandler? PropertyChanged;

    protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    protected bool SetField<T>(ref T field, T value, [CallerMemberName] string? propertyName = null)
    {
        if (EqualityComparer<T>.Default.Equals(field, value)) return false;
        field = value;
        OnPropertyChanged(propertyName);
        return true;
    }
}