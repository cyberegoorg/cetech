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
                PlaformUpdateEvents();

                Keyboard.Process();
                Mouse.Process();

                if (Keyboard.ButtonReleased(Keyboard.ButtonIndex("q")))
                {
                    Quit();
                }

                int[] tasks = new[] { TaskManager.AddBegin("task1", delegate(object data) {  }, null) };
                TaskManager.AddEnd(tasks);

                Renderer.BeginFrame();
                Renderer.EndFrame();
                MainWindow.Update();

                TaskManager.Wait(tasks[0]);
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