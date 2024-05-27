using System.IO;
using System.Text.RegularExpressions;
using OctoVis.Model;

namespace OctoVis.Parser.v0_0_2;

public class LogParser : IParser
{
    public record EnterExitEntry(
        string Module,
        string Class,
        string MethodName,
        bool TailCall,
        ulong StartTime,
        ulong? EndTime)
    {
        public ulong Duration => EndTime.HasValue ? EndTime.Value - StartTime : 0;

        public readonly List<EnterExitEntry> Children = [];
    }

    public ProfilerDataModel Parse(ulong startTicks, StreamReader stream)
    {
        var topNode = new EnterExitEntry(string.Empty, string.Empty, string.Empty, false, startTicks, null);
        var (entries, _) = CreateEnterExitNode(topNode, stream, startTicks);

        var model = new ProfilerDataModel
        {
            EnterExitModel = entries.Children
        };
        return model;
    }

    bool IsStelemRefMethod(string method)
    {
        return method.StartsWith("StelemRef");
    }

    private (EnterExitEntry,bool) CreateEnterExitNode(EnterExitEntry parent, StreamReader stream, ulong startTicks)
    {
        while (!stream.EndOfStream)
        {
            var line = stream.ReadLine() ?? string.Empty;
            if (line.Contains("OctoProfilerEnterLeave::Prepare for shutdown..."))
            {
                var endTicks = ParseTimestamp(line);
                parent = parent with { EndTime = CalculateTimestamp(endTicks, startTicks) };
                return (parent, true);
            }

            if (line.Contains("OctoProfilerEnterLeave::Enter"))
            {
                var ticks = ParseTimestamp(line);
                var (method, @class, module, isTailCall) = ParseEnterLeave(line);
                if (IsStelemRefMethod(method)) continue;
                if (!isTailCall)
                {
                    var newNode = new EnterExitEntry(module, @class, method, isTailCall,
                        CalculateTimestamp(ticks, startTicks), null);
                    (newNode, var matched) = CreateEnterExitNode(newNode, stream, startTicks);
                    if (!matched)
                    {
                        parent = parent with { EndTime = newNode.EndTime };
                    }
                    parent.Children.Add(newNode);
                }
                else
                {
                    if (parent.MethodName == method && parent.Class == @class && parent.Module == module)
                    {
                        parent = parent with { EndTime = CalculateTimestamp(ticks, startTicks) };
                        return (parent, true);
                    }

                    throw new InvalidOperationException("Wrong tail call");
                }
            }

            else if (line.Contains("OctoProfilerEnterLeave::Exit"))
            {
                var ticks = ParseTimestamp(line);
                var (method, @class, module, _) = ParseEnterLeave(line);
                if (IsStelemRefMethod(method)) continue;
                parent = parent with { EndTime = CalculateTimestamp(ticks, startTicks) };

                if (parent.MethodName == method && parent.Class == @class && parent.Module == module)
                {
                    return (parent, true);
                }

                return (parent, false);

            }
            else
            {
                if (line.Contains("OctoProfilerEnterLeave::Detected") ||
                    line.Contains("OctoProfilerEnterLeave::Initialize initialized...") ||
                    line.Contains("OctoProfilerEnterLeave::Shutdown..."))
                    continue;
                throw new InvalidOperationException("Invalid line");
            }
        }

        return (parent, true);
    }

    private (string method, string @class, string module, bool tailCall) ParseEnterLeave(string line)
    {
        var match = Regex.Match(line, @".*(Enter|Enter\(tail\)|Exit) ([^;]*);([^;]*);([^;]*)");

        var isTalCall = line.Contains("Enter(tail)");
        return (match.Groups[4].Value, match.Groups[3].Value, match.Groups[2].Value, isTalCall);
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