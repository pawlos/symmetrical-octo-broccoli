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

        public List<EnterExitEntry> Children = new();
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
                var (method, @class, module, isTailCall) = ParseEnterLeave(line);
                if (IsStelemRefMethod(method)) continue;
                _toProcess.Add(new EnterExitEntry(module, @class, method, isTailCall,
                    CalculateTimestamp(ticks, startTicks), null));
            }
            else if (line.Contains("OctoProfilerEnterLeave::Exit"))
            {
                var ticks = ParseTimestamp(line);
                var (method, @class, module, _) = ParseEnterLeave(line);
                if (IsStelemRefMethod(method)) continue;

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

    bool IsStelemRefMethod(string method)
    {
        return method.StartsWith("StelemRef");
    }

    private EnterExitEntry CreateEnterExitNode(EnterExitEntry parent, StreamReader stream, ulong startTicks)
    {
        while (!stream.EndOfStream)
        {
            var line = stream.ReadLine() ?? string.Empty;
            if (line.Contains("OctoProfilerEnterLeave::Prepare for shutdown..."))
            {
                var endTicks = ParseTimestamp(line);
                parent = parent with { EndTime = CalculateTimestamp(endTicks, startTicks) };
                return parent;
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
                    newNode = CreateEnterExitNode(newNode, stream, startTicks);
                    parent.Children.Add(newNode);
                }
                else
                {
                    if (parent.MethodName == method && parent.Class == @class && parent.Module == module)
                    {
                        parent = parent with { EndTime = CalculateTimestamp(ticks, startTicks) };
                        return parent;
                    }

                    throw new InvalidOperationException("Wrong tail call");
                }
            }

            else if (line.Contains("OctoProfilerEnterLeave::Exit"))
            {
                var ticks = ParseTimestamp(line);
                var (method, @class, module, _) = ParseEnterLeave(line);
                if (IsStelemRefMethod(method)) continue;
                if (parent.MethodName == method && parent.Class == @class && parent.Module == module)
                {
                    parent = parent with { EndTime = CalculateTimestamp(ticks, startTicks) };
                }
                else
                {
                    throw new InvalidOperationException("Wrong parent end block");
                }

                return parent;
            }
            else
            {
                if (line.Contains("OctoProfilerEnterLeave::Detected") ||
                    line.Contains("OctoProfilerEnterLeave::Initialize initialized..."))
                    continue;
                throw new InvalidOperationException("Invalid line");
            }
        }

        return parent;
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