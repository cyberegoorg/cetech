using SharpBgfx;

namespace CETech
{
    public static partial class Renderer
    {
        public static void Init(Window window, RendererBackend type)
        {
            PlatformInit(window, type);
        }

        public static void BeginFrame()
        {
            PlatformBeginFrame();
        }

        public static void EndFrame()
        {
            PlatformEndFrame();
        }

        public static void Resize(int width, int height)
        {
            PlatformResize(width, height);
        }

    }
}