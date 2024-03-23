using System.IO;
using System.Text.RegularExpressions;
using LiveChartsCore.Defaults;

namespace OctoVis.Parser;

public static class LogParser
{
    public static (ObservablePoint[], ObservablePoint[], Dictionary<double, string>)? ParseFile(string fileName)
    {
        if (!File.Exists(fileName))
        {
            throw new Exception();
        }

        int totalMemoryAllocated = 0;
        var memoryData = new List<ObservablePoint>();
        var exceptionData = new List<ObservablePoint>();
        var exceptionInfo = new Dictionary<double, string>();
        DateTime? startDt = null;
        foreach (var line in File.ReadAllLines(fileName))
        {
            if (line.Contains("OctoProfiler::QueryInterface"))
            {
                var match = Regex.Match(line, @"\[(\d{4})(\d{2})(\d{2})(\d{2})(\d{2})(\d{2})(\d{4})\].*");
                startDt = new DateTime(
                    int.Parse(match.Groups[1].Value),
                    int.Parse(match.Groups[2].Value),
                    int.Parse(match.Groups[3].Value),
                    int.Parse(match.Groups[4].Value),
                    int.Parse(match.Groups[5].Value),
                    int.Parse(match.Groups[6].Value),
                    int.Parse(match.Groups[7].Value));
            }
            else if (line.Contains("OctoProfiler::ObjectAllocated"))
            {
                var match = Regex.Match(line, @"\[(\d{4})(\d{2})(\d{2})(\d{2})(\d{2})(\d{2})(\d{4})\] OctoProfiler::ObjectAllocated (\d+).*");
                var dt = new DateTime(
                    int.Parse(match.Groups[1].Value),
                    int.Parse(match.Groups[2].Value),
                    int.Parse(match.Groups[3].Value),
                    int.Parse(match.Groups[4].Value),
                    int.Parse(match.Groups[5].Value),
                    int.Parse(match.Groups[6].Value),
                    int.Parse(match.Groups[7].Value));
                var value = int.Parse(match.Groups[8].Value);
                totalMemoryAllocated += value;
                memoryData.Add(new ObservablePoint((dt - startDt)?.TotalMilliseconds, totalMemoryAllocated));
            }
            else if (line.Contains("OctoProfiler::ExceptionThrown"))
            {
                var match = Regex.Match(line, @"\[(\d{4})(\d{2})(\d{2})(\d{2})(\d{2})(\d{2})(\d{4})\] OctoProfiler::ExceptionThrown (.*)");
                var dt = new DateTime(
                    int.Parse(match.Groups[1].Value),
                    int.Parse(match.Groups[2].Value),
                    int.Parse(match.Groups[3].Value),
                    int.Parse(match.Groups[4].Value),
                    int.Parse(match.Groups[5].Value),
                    int.Parse(match.Groups[6].Value),
                    int.Parse(match.Groups[7].Value));
                var point = new ObservablePoint((dt - startDt)?.TotalMilliseconds, 0);
                exceptionData.Add(point);
                exceptionInfo.Add(point.X!.Value, match.Groups[8].Value);
            }
        }

        return (memoryData.ToArray(), exceptionData.ToArray(), exceptionInfo);
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
