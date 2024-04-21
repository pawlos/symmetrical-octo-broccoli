﻿// See https://aka.ms/new-console-template for more information

using HelloProfiler;

bool waitForKey = args.Contains("--wait");
Console.WriteLine("Profile test program - start");
WaitForAKey(waitForKey);

Console.WriteLine("Profiler - GC collect test");
GC.Collect();
WaitForAKey(waitForKey);

Console.WriteLine("Profiler - GC allocations test");
var allocations = new List<TestClass>();
for (int i = 0; i < 40000; i++)
{
    allocations.Add(new TestClass());
}
WaitForAKey(waitForKey);

Console.WriteLine("Profiler - Call stack test");
await Function1();
Console.WriteLine("Profile test program - done");
WaitForAKey(waitForKey);

Console.WriteLine("Profiler - Allocations test");
var test = new int[256];
var test2 = new Dictionary<string, int> { { "test", 12 } };
Console.WriteLine("Print value {0} {1}", test[0], test2["test"]);

WaitForAKey(waitForKey);
Console.WriteLine("Profiler - Exceptions test");
for (int i = 0; i < 10; i++)
{
    try
    {
        if (i % 2 == 0)
            throw new ArgumentException("testing");
        await Task.Delay(200);
        throw new InvalidOperationException();
    }
    catch
    {
        // swallow
    }
}

WaitForAKey(waitForKey);
return;

void WaitForAKey(bool wait)
{
    if (wait)
    {
        Console.WriteLine("Press [Enter]");
        Console.ReadLine();
    }
    Console.WriteLine();
    Console.WriteLine("-----------------------------");
}

// Few functions to have some stack traces
async Task Function1()
{
    await Function2();
}

async Task Function2()
{
    await Function3();
}

async Task Function3()
{
    await Task.Delay(200);
    Console.WriteLine("Hello, Profiler");
}