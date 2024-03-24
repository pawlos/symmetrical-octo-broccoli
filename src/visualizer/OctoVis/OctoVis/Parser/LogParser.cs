using System.IO;
using System.Text.RegularExpressions;
using LiveChartsCore.Defaults;
using OctoVis.Model;

namespace OctoVis.Parser;

public static class LogParser
{
    public static ProfilerDataModel? ParseFile(string fileName)
    {
        if (!File.Exists(fileName))
        {
            return null;
        }

        uint totalMemoryAllocated = 0;
        ProfilerDataModel model = new();
        ulong? startTicks = null;
        foreach (var line in File.ReadAllLines(fileName))
        {
            if (line.Contains("OctoProfiler::QueryInterface"))
            {
                startTicks = ParseTimestamp(line);
            }
            else if (line.Contains("OctoProfiler::ObjectAllocated"))
            {
                var currentTicks = ParseTimestamp(line);
                (string type, uint bytes) = ParseTypeAndBytesAllocated(line);
                totalMemoryAllocated += bytes;
                if (model.TypeAllocationInfo.TryGetValue(type, out uint v))
                {
                    model.TypeAllocationInfo[type] = v + bytes;
                }
                else
                {
                    model.TypeAllocationInfo.Add(type, bytes);
                }
                model.MemoryData.Add(new DataPoint(CalculateTimestamp(currentTicks, startTicks!.Value), totalMemoryAllocated));
            }
            else if (line.Contains("OctoProfiler::ExceptionThrown"))
            {
                var currentTicks = ParseTimestamp(line);
                var exceptionType = ParseException(line);

                var point = new DataPoint(CalculateTimestamp(currentTicks, startTicks!.Value), 0);
                model.ExceptionData.Add(point);
                model.ExceptionInfo.Add(point.Time, exceptionType);
            }
        }

        return model;
    }

    private static ulong ParseTimestamp(string line)
    {
        var match = Regex.Match(line, @"\[(\d+)ns\].*");
        return ulong.Parse(match.Groups[1].Value);
    }

    private static (string, uint) ParseTypeAndBytesAllocated(string line)
    {
        var match = Regex.Match(line, @"\[[^ ]+\] OctoProfiler::ObjectAllocated (\d+) \[B\] for (.*)");
        return (match.Groups[2].Value, uint.Parse(match.Groups[1].Value));
    }

    private static string ParseException(string line)
    {
        var match = Regex.Match(line, @"\[[^ ]+\] OctoProfiler::ExceptionThrown (.*)");
        return match.Groups[1].Value;
    }

    private static ulong CalculateTimestamp(ulong currentTicks, ulong startTicks)
    {
        return (currentTicks - startTicks) / 1000;
    }

    public static void Deconstruct(this (ObservablePoint[], ObservablePoint[], Dictionary<double, string>) p,
            out ObservablePoint[] memoryData,
            out ObservablePoint[] exceptionData,
            out Dictionary<double,string> exceptionInfo)
        {
            memoryData = p.Item1;
            exceptionData = p.Item2;
            exceptionInfo = p.Item3;
        }
}
