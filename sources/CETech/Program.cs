using System;
using CETech.Input;
using CETech.Utils;

namespace CETech
{
    internal static class Program
    {
        /// <summary>
        ///     The main entry point for the application.
        /// </summary>
        [STAThread]
        private static void Main()
        {
            if (BigInit())
            {
                Application.Run();
            }

            BigShutdown();
        }

        private static bool BigInit()
        {
            Log.LogEvent += LogHandler.ConsoleLog;

            Keyboard.Init();
            Mouse.Init();

            Application.Init();

            return true;
        }

        private static void BigShutdown()
        {
            Application.Shutdown();
        }
    }
}