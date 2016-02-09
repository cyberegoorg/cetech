using System;
using SDL2;

// ReSharper disable once CheckNamespace

namespace CETech
{
    public static partial class Keyboard
    {
        private static IntPtr _keyboardStates;
        private static IntPtr _keyboardStatesLast;

        public static void PlatformProcessKeyboard()
        {
            int numKeys;

            _keyboardStatesLast = _keyboardStates;
            _keyboardStates = SDL.SDL_GetKeyboardState(out numKeys);
        }

        public static int PlatformButtonIndex(string buttonName)
        {
            return (int) SDL.SDL_GetScancodeFromName(buttonName);
        }

        public static string PlatformButtonName(int buttonIndex)
        {
            return SDL.SDL_GetScancodeName((SDL.SDL_Scancode) buttonIndex);
        }

        public static bool PlatformButtonState(int buttonIndex)
        {
            unsafe
            {
                var ptr = (int*) _keyboardStates.ToPointer();
                return ptr[buttonIndex] != 0;
            }
        }

        public static bool PlatformButtonPressed(int buttonIndex)
        {
            unsafe
            {
                var ptr = (int*) _keyboardStates.ToPointer();
                var ptrLast = (int*) _keyboardStatesLast.ToPointer();

                return ptrLast[buttonIndex] == 0 && ptr[buttonIndex] != 0;
            }
        }

        public static bool PlatformButtonReleased(int buttonIndex)
        {
            unsafe
            {
                var ptr = (int*) _keyboardStates.ToPointer();
                var ptrLast = (int*) _keyboardStatesLast.ToPointer();

                return ptrLast[buttonIndex] != 0 && ptr[buttonIndex] == 0;
            }
        }
    }
}