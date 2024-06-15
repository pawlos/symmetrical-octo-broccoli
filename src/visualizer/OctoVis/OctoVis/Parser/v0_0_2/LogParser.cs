using System.IO;
using System.Text.RegularExpressions;
using OctoVis.Model;

namespace OctoVis.Parser.v0_0_2;

public class LogParser : IParser
{
    public record EnterExitEntry(
        string ThreadId,
        string Module,
        string Class,
        string MethodName,
        bool IsTailCall,
        ulong Ticks);

    public record ThreadPerfInfo(
        string ThreadId,
        List<ulong> Ticks);

    public record EnterExitEntryStacked(
        string TreadId,
        string Module,
        string Class,
        string MethodName,
        bool TailCall,
        ulong StartTime,
        ulong? EndTime)
    {
        public ulong Duration => EndTime.HasValue ? EndTime.Value - StartTime : 0;

        public readonly List<EnterExitEntryStacked> Children = [];
    }

    public IDataModel Parse(ulong startTicks, StreamReader stream)
    {
        var topNode = CreateTopNode(startTicks);
        var entries = ParseLog(stream, out var endTicks, out var netVersion);
        var threads = CreateEnterExitNode(topNode, entries);
        var groupByThreads = entries.GroupBy(x => x.ThreadId)
            .ToDictionary(x => x.Key, x => x.Select(y => y.Ticks).ToList());

        var model = new PerformanceDataModel
        {
            EnterExitModel = threads,
            StartMarker = startTicks,
            EndMarker = endTicks,
            NetVersion = netVersion,
            ThreadPerfInfo = groupByThreads.Select(x => new ThreadPerfInfo(x.Key, x.Value)).ToList()
        };
        return model;
    }

    private List<EnterExitEntry> ParseLog(StreamReader stream, out ulong endTicks, out string netVersion)
    {
        var entries = new List<EnterExitEntry>();
        endTicks = 0;
        netVersion = string.Empty;
        while (!stream.EndOfStream)
        {
            var line = stream.ReadLine() ?? string.Empty;
            if (line.Contains("OctoProfilerEnterLeave::Prepare for shutdown..."))
            {
                endTicks = ParseTimestamp(line);
            }
            else if (line.Contains("OctoProfilerEnterLeave::Enter") ||
                     line.Contains("OctoProfilerEnterLeave::Exit"))
            {
                var ticks = ParseTimestamp(line);
                var (threadId, method, @class, module, isTailCall) = ParseEnterLeave(line);
                if (IsStelemRefMethod(method)) continue;
                entries.Add(new EnterExitEntry(threadId, module, @class, module, isTailCall, ticks));
            }
            else if (line.Contains("OctoProfilerEnterLeave::Detected"))
            {
                var match = Regex.Match(line, @"\[[^ ]+\] OctoProfilerEnterLeave::Detected (.+)");
                netVersion = match.Groups[1].Value;
            }
            else if (line.Contains("OctoProfilerEnterLeave::Initialize initialized...") ||
                  line.Contains("OctoProfilerEnterLeave::Shutdown..."))
            {
            }
            else throw new InvalidOperationException("Invalid line");
        }

        return entries;
    }

    private static EnterExitEntryStacked CreateTopNode(ulong startTicks)
    {
        return new EnterExitEntryStacked(string.Empty, string.Empty, string.Empty, string.Empty, false, startTicks, null);
    }

    bool IsStelemRefMethod(string method)
    {
        return method.StartsWith("StelemRef");
    }

    private List<EnterExitEntryStacked> CreateEnterExitNode(
        EnterExitEntryStacked parent, List<EnterExitEntry> entries)
    {
        var entriesGroupedByThread = entries.GroupBy(x => x.ThreadId)
            .ToDictionary(x => x.Key, x => x.OrderBy(y => y.Ticks).ToList());

        var list = new List<EnterExitEntryStacked>();

        return list;
    }

    private (string ThreadId, string method, string @class, string module, bool tailCall) ParseEnterLeave(string line)
    {
        var match = Regex.Match(line, @".*(Enter|Enter\(tail\)|Exit) ([^;]*);([^;]*);([^;]*);([^;]*)");

        var isTalCall = line.Contains("Enter(tail)");
        return (match.Groups[5].Value, match.Groups[4].Value, match.Groups[3].Value, match.Groups[2].Value, isTalCall);
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