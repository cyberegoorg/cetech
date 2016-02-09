using SDL2;

// ReSharper disable once CheckNamespace

namespace CETech.Input
{
    public static partial class Mouse
    {
        private static int _buttonStates;
        private static int _buttonStatesLast;
        private static int _posX;
        private static int _posY;

        public static void PlatformInit()
        {
        }

        public static void PlatformProcess()
        {
            _buttonStatesLast = _buttonStates;
            _buttonStates = (int) SDL.SDL_GetMouseState(out _posX, out _posY);
        }

        public static int PlatformButtonIndex(string buttonName)
        {
            switch (buttonName)
            {
                case "left":
                    return (int) SDL.SDL_BUTTON_LMASK;
                case "middle":
                    return (int) SDL.SDL_BUTTON_MMASK;
                case "right":
                    return (int) SDL.SDL_BUTTON_RMASK;
            }

            return 0;
        }

        public static string PlatformButtonName(int buttonIndex)
        {
            if (buttonIndex == SDL.SDL_BUTTON_LMASK)
            {
                return "left";
            }

            if (buttonIndex == SDL.SDL_BUTTON_MMASK)
            {
                return "middle";
            }

            return buttonIndex == SDL.SDL_BUTTON_RMASK ? "right" : "";
        }

        public static bool PlatformButtonState(int buttonIndex)
        {
            return (_buttonStates & buttonIndex) > 0;
        }

        public static bool PlatformButtonPressed(int buttonIndex)
        {
            return (_buttonStates & buttonIndex) == 1 && (_buttonStatesLast & buttonIndex) == 0;
        }

        public static bool PlatformButtonReleased(int buttonIndex)
        {
            return (_buttonStates & buttonIndex) == 0 && (_buttonStatesLast & buttonIndex) == 1;
        }
    }
}