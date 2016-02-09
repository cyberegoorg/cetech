using System;
using SDL2;

// ReSharper disable once CheckNamespace

namespace CETech
{
    public partial class Window
    {
        private IntPtr _windowPtr;

        private void PlatformCtor(string title, WindowPos x, WindowPos y, int width, int height, int flags)
        {
            _windowPtr = SDL.SDL_CreateWindow(title, sdl_pos(x), sdl_pos(x), width, height, sdl_flags(flags));

            if (_windowPtr == IntPtr.Zero)
            {
                throw new NullReferenceException();
            }
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

        private void PlarformSetTitle(string value)
        {
            SDL.SDL_SetWindowTitle(_windowPtr, value);
        }

        private IntPtr PlatformNativePtr()
        {
            var wmi = new SDL.SDL_SysWMinfo();
            SDL.SDL_GetWindowWMInfo(_windowPtr, ref wmi);

            return wmi.info.win.window;
        }

        private void PlatformResize(int width, int height)
        {
            SDL.SDL_SetWindowSize(_windowPtr, width, height);
        }

        private void PlatformDtor()
        {
            SDL.SDL_DestroyWindow(_windowPtr);
        }

        private void PlatformUpdate()
        {
            SDL.SDL_UpdateWindowSurface(_windowPtr);
        }
    }
}