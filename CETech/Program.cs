using System;
using System.IO;
using CETech.Input;
using CETech.Lua;
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
            Config.CreateValue("boot_pacakge", "Boot package", "boot");
            Config.CreateValue("window.title", "main window title", "CETech application");
            Config.CreateValue("window.width", "main window width", 800);
            Config.CreateValue("window.height", "main window height", 600);

            if (BigInit())
            {
                ResourceCompiler.CompileAll();

                ResourceManager.LoadNow(PackageResource.Type, new[] {StringId.FromString("boot")});
                PackageManager.Load(StringId.FromString("boot"));
                PackageManager.Flush(StringId.FromString("boot"));

                Application.Run();
            }

            BigShutdown();
        }

        private static void InitResouce()
        {
#if CETECH_DEVELOP
            ResourceCompiler.RegisterCompiler(PackageResource.Type, PackageResource.Compile);
            ResourceCompiler.RegisterCompiler(LuaResource.Type, LuaResource.Compile);
            ResourceCompiler.RegisterCompiler(StringId.FromString("texture"), delegate { });
            ResourceCompiler.RegisterCompiler(StringId.FromString("config"), delegate { });
#endif

            ResourceManager.RegisterType(PackageResource.Type,
                PackageResource.ResourceLoader, PackageResource.ResourceUnloader,
                PackageResource.ResourceOnline, PackageResource.ResourceOffline);

            ResourceManager.RegisterType(LuaResource.Type, LuaResource.ResourceLoader, LuaResource.ResourceUnloader,
                LuaResource.ResourceOnline, LuaResource.ResourceOffline);

            ResourceManager.RegisterType(StringId.FromString("texture"), delegate { return null; }, delegate { },
                delegate { }, delegate { });
            ResourceManager.RegisterType(StringId.FromString("config"), delegate { return null; }, delegate { },
                delegate { },
                delegate { });
        }

        private static bool BigInit()
        {
            Log.LogEvent += LogHandler.ConsoleLog;

            FileSystem.MapRootDir("core", "core");
            FileSystem.MapRootDir("src", Path.Combine("data", "src"));
            FileSystem.MapRootDir("build", Path.Combine("data", "build", "win"));

            TaskManager.Init();

            InitResouce();

            Keyboard.Init();
            Mouse.Init();

            Application.Init();

            return true;
        }

        private static void BigShutdown()
        {
            Application.Shutdown();
            TaskManager.Shutdown();
        }
    }
}