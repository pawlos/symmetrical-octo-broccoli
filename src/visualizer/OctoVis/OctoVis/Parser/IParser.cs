using System.IO;
using OctoVis.Model;

namespace OctoVis.Parser;

public interface IParser
{
    IDataModel? Parse(ulong startTicks, StreamReader stream);
}