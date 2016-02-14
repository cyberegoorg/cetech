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
            Config.CreateValue("boot_pacakge", "Boot package", "boot");
            Config.CreateValue("window.title", "main window title", "CETech application");
            Config.CreateValue("window.width", "main window width", 800);
            Config.CreateValue("window.height", "main window height", 600);

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

            Renderer.Init(MainWindow, Renderer.BackendType.Default);

            LuaEnviroment.DoResouece(new StringId("lua/boot"));

            var init_fce = LuaEnviroment.EnviromentScript.Globals.Get("init");
            var update_fce = LuaEnviroment.EnviromentScript.Globals.Get("update");
            var shutdown_fce = LuaEnviroment.EnviromentScript.Globals.Get("shutdown");

            LuaEnviroment.EnviromentScript.Call(init_fce);
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

                LuaEnviroment.EnviromentScript.Call(update_fce, 10);

                if (Keyboard.ButtonReleased(Keyboard.ButtonIndex("q")))
                {
                    Quit();
                }

                Renderer.BeginFrame();
                Renderer.EndFrame();
                MainWindow.Update();
            }

            LuaEnviroment.EnviromentScript.Call(shutdown_fce);

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