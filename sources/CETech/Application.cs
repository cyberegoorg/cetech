using System.Diagnostics;
using CETech.Input;
using CETech.Utils;

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
        }

        /// <summary>
        ///     Shutdown application
        /// </summary>
        public static void Shutdown()
        {
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

            while (_run)
            {
                Debug.Assert(TaskManager.OpenTaskCount == 0);

                PlaformUpdateEvents();

                var frameTask = TaskManager.AddBegin("frame", delegate(object data) { }, null);
                var keyboardTask = TaskManager.AddBegin("keyboard", delegate(object data) { Keyboard.Process(); }, null, parent:frameTask);
                var mouseTask = TaskManager.AddBegin("mouseTask", delegate(object data) { Mouse.Process(); }, null, parent: frameTask);

                TaskManager.AddEnd( new int[] { frameTask, keyboardTask, mouseTask });
                TaskManager.Wait(frameTask);

                if (Keyboard.ButtonReleased(Keyboard.ButtonIndex("q")))
                {
                    Quit();
                }

                Renderer.BeginFrame();
                Renderer.EndFrame();
                MainWindow.Update();
            }
            
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