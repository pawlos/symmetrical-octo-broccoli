using System.IO;
using System.Text.RegularExpressions;
using OctoVis.Model;

namespace OctoVis.Parser.v0_0_2;

public class LogParser : IParser
{
    public record EnterExitEntry(string Module, string Class, string MethodName, ulong StartTime, ulong? EndTime)
    {
        public ulong Duration => EndTime.HasValue ? EndTime.Value - StartTime : 0;
    }

    private readonly List<EnterExitEntry> _toProcess = new(40_000);
    private readonly List<EnterExitEntry> _processed = new(40_000);
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
            else if (line.Contains("OctoProfilerEnterLeave::Enter"))
            {
                var ticks = ParseTimestamp(line);
                var (method, @class, module) = ParseEnterLeave(line);
                if (method == "StelemRef") continue;
                _toProcess.Add(new EnterExitEntry(module, @class, method, CalculateTimestamp(ticks, startTicks), null));
            }
            else if (line.Contains("OctoProfilerEnterLeave::Exit"))
            {
                var ticks = ParseTimestamp(line);
                var (method, @class, module) = ParseEnterLeave(line);
                if (method == "StelemRef") continue;

                var item = _toProcess.FindLast(x => x.MethodName == method && x.Module == module && x.Class == @class);
                if (item is null)
                    throw new Exception("Could not find previous entry");
                var index = _toProcess.IndexOf(item);
                _toProcess.RemoveAt(index);
                _processed.Add(item with { EndTime = CalculateTimestamp(ticks, startTicks) });
            }
        }

        model.EnterExitModel = _processed;
        return model;
    }

    private (string method, string @class, string module) ParseEnterLeave(string line)
    {
        var match = Regex.Match(line, ".*(Enter|Exit) ([^;]*);([^;]*);([^;]*)");

        return (match.Groups[4].Value, match.Groups[3].Value, match.Groups[2].Value);
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