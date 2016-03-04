using CETech.Input;
using CETech.Lua;

namespace CETech
{
    /// <summary>
    ///     Application
    /// </summary>
    public static partial class Application
    {
        //private static long _lastFrameTick;
        private static bool _run;

        /// <summary>
        ///     Get application main window
        /// </summary>
        public static Window MainWindow { get; private set; }

        /// <summary>
        ///     Init application
        /// </summary>
        public static void Init()
        {
            LuaEnviroment.Init();
        }

        /// <summary>
        ///     Shutdown application
        /// </summary>
        public static void Shutdown()
        {
            LuaEnviroment.Shutdown();
        }

        /// <summary>
        ///     Run application
        /// </summary>
        public static void Run()
        {
            _run = true;

            MainWindow = new Window(
                Config.GetValueString("window.title"),
                WindowPos.Centered, WindowPos.Centered,
                Config.GetValueInt("window.width"), Config.GetValueInt("window.height"), 0);

            RenderSystem.Init(MainWindow, RenderSystem.BackendType.Default);

            LuaEnviroment.BootScriptInit(StringId.FromString("lua/boot"));

            LuaEnviroment.BootScriptCallInit();
            while (_run)
            {
                //Debug.Assert(TaskManager.OpenTaskCount < 2);

                PlaformUpdateEvents();

                var frameTask = TaskManager.AddNull("frame");
                var keyboardTask = TaskManager.AddBegin("keyboard", delegate { Keyboard.Process(); }, null,
                    parent: frameTask);
                var mouseTask = TaskManager.AddBegin("mouseTask", delegate { Mouse.Process(); }, null, parent: frameTask);

                TaskManager.AddEnd(new[] {frameTask, keyboardTask, mouseTask});
                TaskManager.Wait(frameTask);

                LuaEnviroment.BootScriptCallUpdate(10.0f);

                RenderSystem.BeginFrame();
                RenderSystem.EndFrame();
                MainWindow.Update();
            }

            LuaEnviroment.BootScriptCallShutdown();

            MainWindow = null;

            Shutdown();
        }

        /// <summary>
        ///     Quit application
        /// </summary>
        public static void Quit()
        {
            _run = false;
        }
    }
}