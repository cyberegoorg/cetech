using System;
using CETech.CEMath;
using SDL2;

// ReSharper disable once CheckNamespace

namespace CETech
{
    public partial class Window
    {
        private IntPtr _windowPtr;

        private void CtorImpl(IntPtr id)
        {
            //Width = width;
            //Height = height;
            _windowPtr = SDL.SDL_CreateWindowFrom(id);

            if (_windowPtr == IntPtr.Zero)
            {
                throw new Exception(string.Format("Could not create window: {0}", SDL.SDL_GetError()));
            }


            int w, h;
            SDL.SDL_GetWindowSize(_windowPtr, out w, out h);

            Width = w;
            Height = h;
        }

        private void CtorImpl(string title, WindowPos x, WindowPos y, int width, int height, int flags)
        {
            _windowPtr = SDL.SDL_CreateWindow(title, sdl_pos(x), sdl_pos(x), width, height, sdl_flags(flags));

            if (_windowPtr == IntPtr.Zero)
            {
                throw new Exception(string.Format("Could not create window: {0}", SDL.SDL_GetError()));
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

        private void SetTitleImpl(string value)
        {
            SDL.SDL_SetWindowTitle(_windowPtr, value);
        }

        private IntPtr GetNativePtrImpl()
        {
            var wmi = new SDL.SDL_SysWMinfo();
            SDL.SDL_VERSION(out wmi.version);
            SDL.SDL_GetWindowWMInfo(_windowPtr, ref wmi);

#if PLATFORM_WINDOWS
            return wmi.info.win.window;
#elif PLATFORM_LINUX
            return wmi.info.x11.window;
#elif PLATFORM_MACOS
            return wmi.info.cocoa.window;
#endif
        }

        private IntPtr GetNativeDisplayPtrImpl()
        {
            var wmi = new SDL.SDL_SysWMinfo();
            SDL.SDL_VERSION(out wmi.version);
            SDL.SDL_GetWindowWMInfo(_windowPtr, ref wmi);

#if PLATFORM_WINDOWS
            return wmi.info.win.hdc;
#elif PLATFORM_LINUX
			return wmi.info.x11.display;
#else
            return IntPtr.zero;
#endif
        }

        private void ResizeImpl(int width, int height)
        {
            SDL.SDL_SetWindowSize(_windowPtr, width, height);
        }

        private void DtorImpl()
        {
            SDL.SDL_DestroyWindow(_windowPtr);
        }

        private void UpdateImpl()
        {
            SDL.SDL_UpdateWindowSurface(_windowPtr);
        }

        private void SetCursorPositionImpl(Vec2f pos)
        {
            SDL.SDL_WarpMouseInWindow(_windowPtr, (int)pos.X, (int)pos.Y);
        }
    }
}
