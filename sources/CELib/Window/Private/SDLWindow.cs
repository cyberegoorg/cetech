using System;
using SDL2;

namespace CELib.Window.Private
{
    public class SDLWindow : IWindow
    {
        private readonly IntPtr _windowPtr;
        private string _title;

        public SDLWindow(string title, WindowPos x, WindowPos y, int width, int height, int flags)
        {
            _windowPtr = SDL.SDL_CreateWindow(title, sdl_pos(x), sdl_pos(x), width, height, sdl_flags(flags));

            if (_windowPtr == IntPtr.Zero)
            {
                throw new NullReferenceException();
            }
        }

        public string Title
        {
            get { return _title; }
            set
            {
                _title = value;
                SDL.SDL_SetWindowTitle(_windowPtr, value);
            }
        }

        public int Width
        {
            get
            {
                int w, h;
                SDL.SDL_GetWindowSize(_windowPtr, out w, out h);
                return w;
            }
        }

        public int Height
        {
            get
            {
                int w, h;
                SDL.SDL_GetWindowSize(_windowPtr, out w, out h);
                return h;
            }
        }

        public IntPtr NativeWindowPtr
        {
            get
            {
                SDL.SDL_SysWMinfo wmi = new SDL.SDL_SysWMinfo();
                SDL.SDL_GetWindowWMInfo(_windowPtr, ref wmi);

                return wmi.info.win.window;
            }
        }

        public void Update()
        {
            SDL.SDL_UpdateWindowSurface(_windowPtr);
        }

        public void Resize(int width, int height)
        {
            SDL.SDL_SetWindowSize(_windowPtr, width, height);
        }

        ~SDLWindow()
        {
            SDL.SDL_DestroyWindow(_windowPtr);
        }

        private static int sdl_pos(WindowPos pos)
        {
            switch (pos)
            {
                case WindowPos.Centered:
                    return SDL.SDL_WINDOWPOS_CENTERED;

                case WindowPos.Undefined:
                    return SDL.SDL_WINDOWPOS_UNDEFINED;

                default:
                    throw new ArgumentOutOfRangeException("pos", pos, null);
            }
        }

        private static SDL.SDL_WindowFlags sdl_flags(int flags)
        {
            SDL.SDL_WindowFlags sdlFlags = 0;

            if ((flags & (int) WindowFlags.Fullscreen) != 0)
            {
                sdlFlags |= SDL.SDL_WindowFlags.SDL_WINDOW_FULLSCREEN;
            }

            return sdlFlags;
        }
    }
}