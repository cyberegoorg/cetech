using System;
using SDL2;
using SharpBgfx;

namespace CETech
{
    public static partial class Application
    {
        private static long _lastFrameTick;
        private static bool _run;

        public static Window MainWindow { get; private set; }

        public static void Init()
        {
            MainWindow = new Window(
                "Application",
                WindowPos.Centered, WindowPos.Centered,
                800, 600, 0);

            Renderer.Init(MainWindow, RendererBackend.Default);
        }

        public static void Shutdown()
        {
            MainWindow = null;
        }

        public static void Run()
        {
            _run = true;


            while (_run)
            {
                PlaformUpdateEvents();
                Renderer.BeginFrame();
                Renderer.EndFrame();
                MainWindow.Update();
            }

            Shutdown();
        }

        public static void Quit()
        {
            _run = false;
        }

    }
}