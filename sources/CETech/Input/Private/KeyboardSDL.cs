using System.Runtime.InteropServices;
using CETech.Utils;
using SDL2;

// ReSharper disable once CheckNamespace

namespace CETech.Input
{
    public static partial class Keyboard
    {
        private static readonly byte[] _keyboardStates = new byte[512];
        private static readonly byte[] _keyboardStatesLast = new byte[512];

        private static void PlatformInit()
        {
        }

        private static void PlatformProcessKeyboard()
        {
            int numKeys;

            _keyboardStates.CopyTo(_keyboardStatesLast, 0);
            Marshal.Copy(SDL.SDL_GetKeyboardState(out numKeys), _keyboardStates, 0, 512);
        }

        private static int ButtonIndexImpl(string buttonName)
        {
            return (int) SDL.SDL_GetScancodeFromName(buttonName);
        }

        private static string ButtonNameImpl(int buttonIndex)
        {
            return SDL.SDL_GetScancodeName((SDL.SDL_Scancode) buttonIndex);
        }

        private static bool ButtonStateImpl(int buttonIndex)
        {
            return _keyboardStates[buttonIndex] != 0;
        }

        private static bool ButtonPressedImpl(int buttonIndex)
        {
            return _keyboardStates[buttonIndex] == 1 && _keyboardStatesLast[buttonIndex] == 0;
        }

        private static bool ButtonReleasedImpl(int buttonIndex)
        {
            return _keyboardStates[buttonIndex] == 0 && _keyboardStatesLast[buttonIndex] == 1;
        }
    }
}