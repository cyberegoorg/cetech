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
            if (BigInit())
            {
                ResourceCompiler.CompileAll();

                ResourceManager.LoadNow(PackageResource.Type, new[] {new StringId("boot")});
                PackageManager.Load(new StringId("boot"));
                PackageManager.Flush(new StringId("boot"));

                Application.Run();
            }

            BigShutdown();
        }

        private static void InitResouce()
        {
#if CETECH_DEVELOP
            ResourceCompiler.registerCompiler(PackageResource.Type, PackageResource.Compile);
            ResourceCompiler.registerCompiler(LuaResource.Type, LuaResource.Compile);
            ResourceCompiler.registerCompiler(new StringId("texture"), delegate { });
            ResourceCompiler.registerCompiler(new StringId("config"), delegate { });
#endif

            ResourceManager.RegisterType(PackageResource.Type,
                PackageResource.ResourceLoader, PackageResource.ResourceUnloader,
                PackageResource.ResourceOnline, PackageResource.ResourceOffline);

            ResourceManager.RegisterType(LuaResource.Type, LuaResource.ResourceLoader, LuaResource.ResourceUnloader,
                LuaResource.ResourceOnline, LuaResource.ResourceOffline);

            ResourceManager.RegisterType(new StringId("texture"), delegate { return null; }, delegate { },
                delegate { }, delegate { });
            ResourceManager.RegisterType(new StringId("config"), delegate { return null; }, delegate { }, delegate { },
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