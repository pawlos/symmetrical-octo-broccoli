namespace OctoVis.Model;

public class FlameGraphNode
{
    public ProfilerDataModel.StackFrame? Frame { get; init; }
    public int SampleCount { get; set; }
    public List<FlameGraphNode> Children { get; } = [];

    public static FlameGraphNode BuildTree(List<List<ProfilerDataModel.StackFrame>> stacks)
    {
        var root = new FlameGraphNode { Frame = null, SampleCount = stacks.Count };

        foreach (var stack in stacks)
        {
            var current = root;
            foreach (var frame in stack)
            {
                var existing = current.Children.FirstOrDefault(c => c.Frame?.FrameInfo == frame.FrameInfo);
                if (existing is null)
                {
                    existing = new FlameGraphNode { Frame = frame, SampleCount = 0 };
                    current.Children.Add(existing);
                }
                existing.SampleCount++;
                current = existing;
            }
        }

        return root;
    }
}
