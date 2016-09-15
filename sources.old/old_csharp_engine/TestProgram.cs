using System;
using System.Linq;
using System.Reflection;
using NSpec;
using NSpec.Domain;
using NSpec.Domain.Formatters;

namespace CETech
{
    internal static class TestProgram
    {
        /// <summary>
        ///     The main entry point for the application.
        /// </summary>
        [STAThread]
        private static void Main(string[] args)
        {
            var ri = new RunnerInvocation(Assembly.GetExecutingAssembly().Location, "", false);
            ri.Run();

            Console.WriteLine("Done.");
            Console.ReadLine();
        }
    }
}