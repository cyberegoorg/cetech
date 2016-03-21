using System;
using System.Collections.Generic;
using System.IO;
using CETech.Develop;
using CETech.Input;
using CETech.Lua;
using CETech.Utils;
using Mono.Options;
using SharpBgfx;
#if CETECH_SDL2
using SDL2;

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
            
            if (!Application.ParseCmdLine(args))
            {
                return;
            }

            if (Application.Init())
            {
                Application.Run();
            }

            Application.Shutdown();
        }
    }
}