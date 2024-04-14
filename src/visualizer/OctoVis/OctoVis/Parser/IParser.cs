using System.IO;
using OctoVis.Model;

namespace OctoVis.Parser;

public interface IParser
{
    ProfilerDataModel? Parse(ulong startTicks, StreamReader stream);
}