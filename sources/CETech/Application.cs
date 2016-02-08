using CELib.Window;
using CETech.Renderer;
using SharpBgfx;

namespace CETech
{
    public static partial class Application
    {
        private struct Data
        {
            public IWindow Window;
            public long LastFrameTick;
            public bool Run;
        }

        private static Data _data;

        public static IWindow MainWindow
        {
            get { return _data.Window; }
        }

        public static void Init()
        {
            _data.Window = Window.CreateWindow(
                "Application",
                WindowPos.Centered, WindowPos.Centered,
                800, 600, 0);

            BgfxRenderer.Init(_data.Window, RendererBackend.Default);
        }

        public static void Shutdown()
        {
            _data.Window = null;
        }

        public static void Run()
        {
            _data.Run = true;


            while (_data.Run)
            {
                UpdateEvents();
                BgfxRenderer.BeginFrame();
                BgfxRenderer.EndFrame();
                _data.Window.Update();
            }

            Shutdown();
        }

        public static void Quit()
        {
            _data.Run = false;
        }
    }
}