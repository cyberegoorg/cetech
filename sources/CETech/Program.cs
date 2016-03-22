using System;
#if CETECH_SDL2

#endif

namespace CETech
{
    internal static class Program
    {
        /// <summary>
        ///     The main entry point for the application.
        /// </summary>
        [STAThread]
        private static void Main(string[] args)
        {
            if (Application.Init(args))
            {
                Application.Run();
            }

            Application.Shutdown();
        }
    }
}