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

        using var stream = File.OpenText(fileName);

        while (!stream.EndOfStream)
        {
            var line = stream.ReadLine() ?? string.Empty;
            if (line.Contains("::Initialize started..."))
            {
                var version = Regex.Match(line, @"\[[^ ]+\] .*::Initialize started...(.+)");
                var startTicks = ParseTimestamp(line);
                var parser = CreateParser(version.Groups[1].Value);
                return parser.Parse(startTicks, stream);
            }
        }

        return null;
    }

    private static ulong ParseTimestamp(string line)
    {
        var match = Regex.Match(line, @"\[(\d+)ns\].*");
        return ulong.Parse(match.Groups[1].Value);
    }

    private static IParser CreateParser(string version)
    {
        return version switch
        {
            "v0.0.1" => new v0_0_1.LogParser(),
            "v0.0.2" => new v0_0_2.LogParser(),
            _ => throw new ArgumentOutOfRangeException(nameof(version))
        };
    }

    public static void Deconstruct(this (ObservablePoint[], ObservablePoint[], Dictionary<double, string>) p,
        out ObservablePoint[] memoryData,
        out ObservablePoint[] exceptionData,
        out Dictionary<double, string> exceptionInfo)
    {
        memoryData = p.Item1;
        exceptionData = p.Item2;
        exceptionInfo = p.Item3;
    }
}