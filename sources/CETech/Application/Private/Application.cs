using System;
using System.Collections.Generic;
using System.IO;
using CETech.Develop;
using CETech.EntCom;
using CETech.Input;
using CETech.Lua;
using CETech.Resource;
using CETech.Utils;
using CETech.World;
using Mono.Options;
using SDL2;

namespace CETech
{
    /// <summary>
    ///     Application
    /// </summary>
    public static partial class Application
    {
        private static bool _run;
        private static float _deltaTime;
        private static Window _mainWindow;

        private static List<SystemInitConfig> initConfigsDelegates = new List<SystemInitConfig>();
        private static List<SystemInitDelegate> InitDelegates = new List<SystemInitDelegate>();
        private static List<SystemShutdownDelegate> ShutdownDelegates = new List<SystemShutdownDelegate>();

        /// <summary>
        ///     Init application
        /// </summary>
        private static bool InitImpl(string[] args)
        {
            ConfigSystem.Init();

#if CETECH_DEVELOP
            ResourceCompiler.InitConfig();
            ConsoleServer.InitConfig();
#endif

            Resource.Resource.InitConfig();

            ConfigSystem.CreateValue("application.platform", "Platform", GetPlatform());
            ConfigSystem.CreateValue("application.log_file", "Log file", "");

            ConfigSystem.CreateValue("boot.pkg", "Boot package", "boot");
            ConfigSystem.CreateValue("boot.script", "Boot script", "lua/boot");

            ConfigSystem.CreateValue("window.title", "main window title", "CETech application");
            ConfigSystem.CreateValue("window.width", "main window width", 800);
            ConfigSystem.CreateValue("window.height", "main window height", 600);

            if (!ParseCmdLine(args))
            {
                return false;
            }

            if (!BigInit())
            {
                return false;
            }

            Boot();

            LuaEnviroment.Init();
            return true;
        }

        /// <summary>
        ///     Shutdown application
        /// </summary>
        private static void ShutdownImpl()
        {
            LuaEnviroment.Shutdown();

            BigShutdown();
        }

        /// <summary>
        ///     Run application
        /// </summary>
        private static void RunImpl()
        {
            _run = true;
            Game.Init();

            var last_frame_tick = DateTime.Now;
            DateTime curent_frame_tick;
            Log.Info("application.ready", "");

            var tasks = new int[2];

            var inv_fps = 1000.0f/120.0f;
            var frame_acum = 0.0f;
            while (_run)
            {
                //Debug.Assert(TaskManager.OpenTaskCount < 2);
                ConsoleServer.Tick();

                curent_frame_tick = DateTime.Now;
                _deltaTime = (float) (curent_frame_tick - last_frame_tick).TotalMilliseconds;
                last_frame_tick = curent_frame_tick;

                frame_acum += _deltaTime;
                if (frame_acum >= inv_fps)
                {
                    frame_acum = 0.0f;

                    DevelopSystem.FrameBegin();
                    DevelopSystem.PushRecordFloat("application.dt", _deltaTime);
                    var updateScope = DevelopSystem.EnterScope();

                    PlaformUpdateEvents();

                    tasks[0] = TaskManager.AddNull("frame");
                    tasks[1] = TaskManager.AddBegin("input", delegate
                    {
                        var scope = DevelopSystem.EnterScope();

                        Mouse.Process();
                        Keyboard.Process();
                        Gamepad.Process();

                        DevelopSystem.LeaveScope("input", scope);
                    }, null,
                        parent: tasks[0]);

                    TaskManager.AddEnd(tasks);
                    TaskManager.Wait(tasks[0]);

                    {
                        var scope = DevelopSystem.EnterScope();
                        Game.Update(_deltaTime);
                        DevelopSystem.LeaveScope("Game::Update", scope);
                    }

                    Game.Render();

                    _mainWindow.Update();

                    DevelopSystem.LeaveScope("Application::Update", updateScope);
                    DevelopSystem.PushRecordInt("gc.total_memory", (int) GC.GetTotalMemory(false));
                    DevelopSystem.Send();
                }
            }

            Game.Shutdown();

            _mainWindow = null;
        }

        /// <summary>
        ///     Quit application
        /// </summary>
        private static void QuitImpl()
        {
            _run = false;
        }

        private static string GetPlatformImpl()
        {
#if PLATFORM_WINDOWS
            return "windows";
#elif PLATFORM_LINUX
            return "linux";
#elif PLATFORM_MACOS
            return "darwin";
#endif
        }

        public static bool ParseCmdLineImpl(string[] args)
        {
            var show_help = false;
            string build_dir = null;
            string log_file = null;

#if CETECH_DEVELOP
            string core_dir = null;
            string source_dir = null;
            string bin_dir = null;
            string bootscript = null;
            int? first_port = null;

            DevelopFlags.wid = IntPtr.Zero;
            DevelopFlags.bootscript = null;
#endif

            var p = new OptionSet
            {
                {
                    "h|help", "show this message and exit",
                    v => show_help = v != null
                },


                {
                    "l|log-file=", "Log file",
                    v => log_file = v
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
                    "bin=", "Bin dir.",
                    v => bin_dir = v
                },

                {
                    "bootscript=", "Boot script.",
                    v => DevelopFlags.bootscript = v
                },

                {
                    "p|port=", "First port",
                    (int v) => first_port = v
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
                ConfigSystem.SetValue("resource_compiler.src", source_dir);
            }

            if (core_dir != null)
            {
                ConfigSystem.SetValue("resource_compiler.core", core_dir);
            }

            if (bin_dir != null)
            {
                ConfigSystem.SetValue("resource_compiler.bin", bin_dir);
            }

            if (first_port != null)
            {
                ConfigSystem.SetValue("console_server.base_port", (int) first_port);
            }

#endif

            if (log_file != null)
            {
                ConfigSystem.SetValue("application.log_file", log_file);
            }

            if (build_dir != null)
            {
                ConfigSystem.SetValue("resource_manager.build", build_dir);
            }


            return true;
        }

        private static void Boot()
        {
            var boot_pkg = StringId.FromString(ConfigSystem.GetValueString("boot.pkg"));

            Resource.Resource.LoadNow(PackageResource.Type, new[] {boot_pkg});

            Package.Load(boot_pkg);
            Package.Flush(boot_pkg);
        }

        private static void InitResouce()
        {
            Tranform.Init();
            MeshRenderer.Init();
            CameraSystem.Init();

#if CETECH_DEVELOP
            ResourceCompiler.RegisterCompiler(PackageResource.Type, PackageResource.Compile);
            ResourceCompiler.RegisterCompiler(LuaResource.Type, LuaResource.Compile);
            ResourceCompiler.RegisterCompiler(ConfigResource.Type, ConfigResource.Compile);
            ResourceCompiler.RegisterCompiler(UnitResource.Type, UnitResource.Compile);
            ResourceCompiler.RegisterCompiler(ShaderResource.Type, ShaderResource.Compile);
            ResourceCompiler.RegisterCompiler(MaterialResource.Type, MaterialResource.Compile);
            ResourceCompiler.RegisterCompiler(TextureResource.Type, TextureResource.Compile);
            ResourceCompiler.RegisterCompiler(LevelResource.Type, LevelResource.Compile);
            ResourceCompiler.RegisterCompiler(RenderConfig.Type, RenderConfig.Compile);
            ResourceCompiler.RegisterCompiler(MeshResource.Type, MeshResource.Compile);

            if (DevelopFlags.compile)
            {
                ResourceCompiler.CompileAll();
            }
#endif

            Resource.Resource.RegisterType(
                ConfigResource.Type,
                ConfigResource.ResourceLoader, ConfigResource.ResourceUnloader,
                ConfigResource.ResourceOnline, ConfigResource.ResourceOffline, ConfigResource.Reloader);

            Resource.Resource.RegisterType(
                PackageResource.Type,
                PackageResource.ResourceLoader, PackageResource.ResourceUnloader,
                PackageResource.ResourceOnline, PackageResource.ResourceOffline, PackageResource.Reloader);

            Resource.Resource.RegisterType(
                LuaResource.Type,
                LuaResource.ResourceLoader, LuaResource.ResourceUnloader,
                LuaResource.ResourceOnline, LuaResource.ResourceOffline, LuaResource.Reloader);

            Resource.Resource.RegisterType(
                UnitResource.Type,
                UnitResource.ResourceLoader, UnitResource.ResourceUnloader,
                UnitResource.ResourceOnline, UnitResource.ResourceOffline, UnitResource.Reloader);

            Resource.Resource.RegisterType(
                ShaderResource.Type,
                ShaderResource.ResourceLoader, ShaderResource.ResourceUnloader,
                ShaderResource.ResourceOnline, ShaderResource.ResourceOffline, ShaderResource.Reloader);

            Resource.Resource.RegisterType(
                MaterialResource.Type,
                MaterialResource.ResourceLoader, MaterialResource.ResourceUnloader,
                MaterialResource.ResourceOnline, MaterialResource.ResourceOffline, MaterialResource.ResourceReloader);

            Resource.Resource.RegisterType(
                TextureResource.Type,
                TextureResource.ResourceLoader, TextureResource.ResourceUnloader,
                TextureResource.ResourceOnline, TextureResource.ResourceOffline, TextureResource.ResourceReloader);

            Resource.Resource.RegisterType(
                LevelResource.Type,
                LevelResource.ResourceLoader, LevelResource.ResourceUnloader,
                LevelResource.ResourceOnline, LevelResource.ResourceOffline, LevelResource.ResourceReloader);

            Resource.Resource.RegisterType(
                RenderConfig.Type,
                RenderConfig.ResourceLoader, RenderConfig.ResourceUnloader,
                RenderConfig.ResourceOnline, RenderConfig.ResourceOffline, RenderConfig.ResourceReloader);


            Resource.Resource.RegisterType(
                MeshResource.Type,
                MeshResource.ResourceLoader, MeshResource.ResourceUnloader,
                MeshResource.ResourceOnline, MeshResource.ResourceOffline, MeshResource.ResourceReloader);
        }

        private static bool BigInit()
        {
            Log.LogEvent += LogHandler.ConsoleLog;

            var log_file = ConfigSystem.GetValueString("application.log_file");
            if (log_file.Length != 0)
            {
                Log.LogEvent += new LogHandler.FileLog(log_file).Log;
            }

#if CETECH_SDL2
            SDL.SDL_Init(SDL.SDL_INIT_EVERYTHING);
#endif

            EntityManager.Init();
            Unit.Init();
            World.World.Init();

#if CETECH_DEVELOP

            ConsoleServer.Init();
            DevelopSystem.Init();

            ResourceCompiler.Init();
            FileSystem.MapRootDir("core", ConfigSystem.GetValueString("resource_compiler.core"));
            FileSystem.MapRootDir("src", ConfigSystem.GetValueString("resource_compiler.src"));
#endif

            FileSystem.MapRootDir("build",
                Path.Combine(ConfigSystem.GetValueString("resource_manager.build"),
                    ConfigSystem.GetValueString("application.platform")));

            TaskManager.Init();

            InitResouce();

            Resource.Resource.Init();
            Resource.Resource.LoadNow(ConfigResource.Type, new[] {StringId.FromString("global")});

#if CETECH_DEVELOP
            if (DevelopFlags.compile && !DevelopFlags.ccontinue)
            {
                return false;
            }

            if (DevelopFlags.bootscript != null)
            {
                ConfigSystem.SetValue("boot.script", DevelopFlags.bootscript);
            }
#endif

            Keyboard.Init();
            Mouse.Init();
            Gamepad.Init();

            Window main_window;

#if CETECH_DEVELOP
            if (DevelopFlags.wid == IntPtr.Zero)
            {
                main_window = new Window(
                    ConfigSystem.GetValueString("window.title"),
                    WindowPos.Centered, WindowPos.Centered,
                    ConfigSystem.GetValueInt("window.width"), ConfigSystem.GetValueInt("window.height"), 0);
            }
            else
            {
                main_window = new Window(DevelopFlags.wid);
            }

#else
            main_window = new Window(
                ConfigSystem.GetValueString("window.title"),
                WindowPos.Centered, WindowPos.Centered,
                ConfigSystem.GetValueInt("window.width"), ConfigSystem.GetValueInt("window.height"), 0);
#endif
            _mainWindow = main_window;

            Renderer.Init(_mainWindow, "default");

            return true;
        }

        private static void BigShutdown()
        {
#if CETECH_DEVELOP
            ResourceCompiler.Shutdown();
            DevelopSystem.Shutdown();
            ConsoleServer.Shutdown();
#endif

            Unit.Shutdown();
            World.World.Shutdown();
            EntityManager.Shutdown();

            TaskManager.Shutdown();
            EntityManager.Shutdown();

            Renderer.Shutdown();
            ConfigSystem.Shutdown();
        }

        private static void RegisterSystemsImpl(SystemInitConfig[] initConfig, SystemInitDelegate[] init,
            SystemShutdownDelegate[] shutdown)
        {
        }

#if CETECH_DEVELOP
        private struct DevelopCmdFlags
        {
            public bool compile;
            public bool ccontinue;
            public IntPtr wid;
            public string bootscript;
        }

        private static DevelopCmdFlags DevelopFlags;
#endif
    }
}