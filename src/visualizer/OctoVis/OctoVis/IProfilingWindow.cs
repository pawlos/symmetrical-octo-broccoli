using OctoVis.Model;

namespace OctoVis;

public interface IProfilingWindow
{
    void SetModel(IDataModel model);
    void Show();
}