using CETech.Input;

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
            MainWindow = new Window(
                "Application",
                WindowPos.Centered, WindowPos.Centered,
                800, 600, 0);

            Renderer.Init(MainWindow, Renderer.BackendType.Default);
        }

        /// <summary>
        ///     Shutdown application
        /// </summary>
        public static void Shutdown()
        {
            MainWindow = null;
        }

        /// <summary>
        ///     Run application
        /// </summary>
        public static void Run()
        {
            _run = true;


            while (_run)
            {
                PlaformUpdateEvents();

                Keyboard.Process();
                Mouse.Process();

                if (Keyboard.ButtonReleased(Keyboard.ButtonIndex("q")))
                {
                    Quit();
                }

                Renderer.BeginFrame();
                Renderer.EndFrame();
                MainWindow.Update();
            }

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