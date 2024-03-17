// See https://aka.ms/new-console-template for more information

Console.WriteLine("Waiting for a key");
GC.Collect();
Console.ReadKey();
Console.WriteLine("Start");
await Function1();
Console.WriteLine("Done");
return;


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