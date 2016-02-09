using System.Runtime.InteropServices;
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

        private static int PlatformButtonIndex(string buttonName)
        {
            return (int) SDL.SDL_GetScancodeFromName(buttonName);
        }

        private static string PlatformButtonName(int buttonIndex)
        {
            return SDL.SDL_GetScancodeName((SDL.SDL_Scancode) buttonIndex);
        }

        private static bool PlatformButtonState(int buttonIndex)
        {
            return _keyboardStates[buttonIndex] != 0;
        }

        private static bool PlatformButtonPressed(int buttonIndex)
        {
            return _keyboardStates[buttonIndex] == 1 && _keyboardStatesLast[buttonIndex] == 0;
        }

        private static bool PlatformButtonReleased(int buttonIndex)
        {
            return _keyboardStates[buttonIndex] == 0 && _keyboardStatesLast[buttonIndex] == 1;
        }
    }
}