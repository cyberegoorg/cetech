using System;
using System.IO;
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
//#if CETECH_DEVELOP
            ResourceCompiler.registerCompiler(PackageResource.Type, PackageResource.compile);
            ResourceCompiler.registerCompiler(new StringId("lua"), delegate { });
            ResourceCompiler.registerCompiler(new StringId("texture"), delegate { });
            ResourceCompiler.registerCompiler(new StringId("config"), delegate { });
//#endif
            ResourceManager.RegisterType(PackageResource.Type,
                PackageResource.ResourceLoader, PackageResource.ResourceUnloader,
                PackageResource.ResourceOnline, PackageResource.ResourceOffline);

            ResourceManager.RegisterType(new StringId("lua"), delegate { return null; }, delegate { }, delegate { },
                delegate { });
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
            FileSystem.MapRootDir("build", Path.Combine("data", "build"));

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