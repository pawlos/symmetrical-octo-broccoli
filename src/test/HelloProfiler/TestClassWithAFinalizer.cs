namespace HelloProfiler;

public class TestClassWithAFinalizer
{
    ~TestClassWithAFinalizer()
    {
        Console.WriteLine("Destructor has been called");
        _buffer = null;
    }
    private byte[]? _buffer = new byte[1024];
}