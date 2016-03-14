using System;
using System.Collections.Generic;
using System.IO;
using CETech.Develop;
using CETech.Input;
using CETech.Lua;
using CETech.Utils;
using Mono.Options;
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
            Config.CreateValue("platform", "Platform", GetPlatform());
            Config.CreateValue("resource_compiler.core", "Path to core dir", "core");
            Config.CreateValue("resource_compiler.src", "Path to source dir", Path.Combine("data", "src"));
            Config.CreateValue("resource_manager.build", "Path to build dir", Path.Combine("data", "build"));

            Config.CreateValue("boot.pkg", "Boot package", "boot");
            Config.CreateValue("boot.script", "Boot script", "lua/boot");

            Config.CreateValue("window.title", "main window title", "CETech application");
            Config.CreateValue("window.width", "main window width", 800);
            Config.CreateValue("window.height", "main window height", 600);

            if (!ParseCmdLine(args))
            {
                return;
            }

            if (BigInit())
            {
                Run();
            }

            BigShutdown();
        }

        private static string GetPlatform()
        {
#if PLATFORM_WINDOWS
            return "windows";
#elif PLATFORM_LINUX
            return "linux";
#endif
        }

        private static bool ParseCmdLine(string[] args)
        {
            var show_help = false;
            string build_dir = null;

#if CETECH_DEVELOP
            string core_dir = null;
            string source_dir = null;

            DevelopFlags.wid = IntPtr.Zero;
#endif
            var p = new OptionSet
            {
                {
                    "h|help", "show this message and exit",
                    v => show_help = v != null
                },

                {
                    "b|build=", "Build dir.",
                    v => build_dir = v
                },
#if CETECH_DEVELOP
                {
                    "s|source=", "Source dir.",
                    v => source_dir = v
                },

                {
                    "core=", "Core dir.",
                    v => core_dir = v
                },

                {
                    "compile", "Compile resource",
                    v => { DevelopFlags.compile = v != null; }
                },

                {
                    "continue", "Continue after compilation.",
                    v => { DevelopFlags.ccontinue = v != null; }
                },

                {
                    "wid=", "Window ptr.",
                    (long v) => DevelopFlags.wid = new IntPtr(v)
                }
#endif
            };

            List<string> extra;
            try
            {
                extra = p.Parse(args);
            }
            catch (OptionException e)
            {
                Console.Write("greet: ");
                Console.WriteLine(e.Message);
                Console.WriteLine("Try `greet --help' for more information.");
                return false;
            }

            if (show_help)
            {
                p.WriteOptionDescriptions(Console.Out);
                return false;
            }

#if CETECH_DEVELOP
            if (source_dir != null)
            {
                Config.SetValue("resource_compiler.src", source_dir);
            }

            if (core_dir != null)
            {
                Config.SetValue("resource_compiler.core", core_dir);
            }
#endif

            if (build_dir != null)
            {
                Config.SetValue("resource_manager.build", build_dir);
            }


            return true;
        }

        private static void Run()
        {
            Window main_window = null;

#if CETECH_DEVELOP
            if (DevelopFlags.wid == IntPtr.Zero)
            {
                main_window = new Window(
                    Config.GetValueString("window.title"),
                    WindowPos.Centered, WindowPos.Centered,
                    Config.GetValueInt("window.width"), Config.GetValueInt("window.height"), 0);
            }
            else
            {
                Log.Debug("ddd", "wid {0}", DevelopFlags.wid);
                main_window = new Window(DevelopFlags.wid);
            }

#else
            main_window = new Window(
                Config.GetValueString("window.title"),
                WindowPos.Centered, WindowPos.Centered,
                Config.GetValueInt("window.width"), Config.GetValueInt("window.height"), 0);
#endif
            Application.MainWindow = main_window;

            ResourceManager.LoadNow(PackageResource.Type, new[] {StringId.FromString(Config.GetValueString("boot.pkg"))});
            PackageManager.Load(StringId.FromString(Config.GetValueString("boot.pkg")));
            PackageManager.Flush(StringId.FromString(Config.GetValueString("boot.pkg")));

            Application.Run();
        }

        private static void InitResouce()
        {
#if CETECH_DEVELOP
            ResourceCompiler.RegisterCompiler(PackageResource.Type, PackageResource.Compile);
            ResourceCompiler.RegisterCompiler(LuaResource.Type, LuaResource.Compile);
            ResourceCompiler.RegisterCompiler(StringId.FromString("texture"), delegate { });
            ResourceCompiler.RegisterCompiler(ConfigResource.Type, ConfigResource.Compile);

            if (DevelopFlags.compile)
            {
                ResourceCompiler.CompileAll();

                if (!DevelopFlags.ccontinue)
                {
                    return;
                }
            }
#endif



            ResourceManager.RegisterType(
                ConfigResource.Type,
                ConfigResource.ResourceLoader, ConfigResource.ResourceUnloader,
                ConfigResource.ResourceOnline, ConfigResource.ResourceOffline);

            ResourceManager.RegisterType(
                PackageResource.Type,
                PackageResource.ResourceLoader, PackageResource.ResourceUnloader,
                PackageResource.ResourceOnline, PackageResource.ResourceOffline);

            ResourceManager.RegisterType(
                LuaResource.Type,
                LuaResource.ResourceLoader, LuaResource.ResourceUnloader,
                LuaResource.ResourceOnline, LuaResource.ResourceOffline);

            ResourceManager.RegisterType(
                StringId.FromString("texture"),
                delegate { return null; }, delegate { },
                delegate { }, delegate { });

        }

        private static bool BigInit()
        {
            Log.LogEvent += LogHandler.ConsoleLog;

#if CETECH_SDL2
            SDL.SDL_Init(SDL.SDL_INIT_EVERYTHING);
#endif

#if CETECH_DEVELOP
            ConsoleServer.Init();
            ResourceCompiler.Init();
            FileSystem.MapRootDir("core", Config.GetValueString("resource_compiler.core"));
            FileSystem.MapRootDir("src", Config.GetValueString("resource_compiler.src"));
#endif

            FileSystem.MapRootDir("build",
                Path.Combine(Config.GetValueString("resource_manager.build"), Config.GetValueString("platform")));

            TaskManager.Init();

            InitResouce();

            ResourceManager.LoadNow(ConfigResource.Type, new []{StringId.FromString("global")});

            Keyboard.Init();
            Mouse.Init();

            Application.Init();

            return true;
        }

        private static void BigShutdown()
        {
            Application.Shutdown();

#if CETECH_DEVELOP
            ResourceCompiler.Shutdown();
            ConsoleServer.Shutdown();
#endif
            TaskManager.Shutdown();
        }

#if CETECH_DEVELOP
        private struct DevelopCmdFlags
        {
            public bool compile;
            public bool ccontinue;
            public IntPtr wid;
        }

        private static DevelopCmdFlags DevelopFlags;
#endif
    }
}