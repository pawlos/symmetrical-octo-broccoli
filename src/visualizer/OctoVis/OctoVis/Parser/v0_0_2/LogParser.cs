using System.IO;
using System.Text.RegularExpressions;
using OctoVis.Model;

namespace OctoVis.Parser.v0_0_2;

public class LogParser : IParser
{
    public ProfilerDataModel Parse(ulong startTicks, StreamReader stream)
    {
        var model = new ProfilerDataModel();
        while (!stream.EndOfStream)
        {
            var line = stream.ReadLine() ?? string.Empty;
            if (line.Contains("OctoProfilerEnterLeave::Prepare for shutdown..."))
            {
                var endTicks = ParseTimestamp(line);
                model.TotalTime = TimeSpan.FromMicroseconds(CalculateTimestamp(endTicks, startTicks));
                model.StartMarker = CalculateTimestamp(startTicks, startTicks);
                model.EndMarker = CalculateTimestamp(endTicks, startTicks);
            }
            else if (line.Contains("OctoProfilerEnterLeave::Detected"))
            {
                var match = Regex.Match(line, @"\[[^ ]+\] OctoProfilerEnterLeave::Detected (.+)");
                model.NetVersion = match.Groups[1].Value;
            }
            else if (line.Contains("OctoProfilerEnterLeave::"))
            {
                var ticks = ParseTimestamp(line);
            }
        }

        return model;
    }


    private static ulong ParseTimestamp(string line)
    {
        var match = Regex.Match(line, @"\[(\d+)ns\].*");
        return ulong.Parse(match.Groups[1].Value);
    }

    private static ulong CalculateTimestamp(ulong currentTicks, ulong startTicks)
    {
        // convert to microsecond
        return (currentTicks - startTicks) / 1000;
    }
}