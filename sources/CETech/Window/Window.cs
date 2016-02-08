using CELib.Window.Private;

namespace CELib.Window
{
    public static class Window
    {
        public static IWindow CreateWindow(string title, WindowPos x, WindowPos y, int width, int height, int flags)
        {

#if PLATFORM_SDL2
            return new SDLWindow(title, x, y, width, height, flags);
#else
#error Invalid platform
#endif
        }
    }
}