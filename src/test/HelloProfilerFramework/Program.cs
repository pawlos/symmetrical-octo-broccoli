using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace HelloProfilerFramework
{
    internal class Program
    {
        public static void Main(string[] args)
        {
            Console.WriteLine($"Waiting for attach. PID: {Process.GetCurrentProcess().Id}");
            Console.ReadKey();

            List<byte[]> listOfAllocations = new List<byte[]>(); 
            for (var i = 0; i < 1000; i++)
            {
                listOfAllocations.Add(new byte[20]);
            }

            Console.WriteLine("List size: " + listOfAllocations.Sum(x => x.Length));
            Console.WriteLine("Press enter to close the program.");
            Console.ReadKey();
        }
    }
}