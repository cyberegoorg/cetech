using CETech.World;
using SDL2;

// ReSharper disable once CheckNamespace
namespace CETech.Input
{
    public static partial class Mouse
    {
        private static int _buttonStates;
        private static int _buttonStatesLast;
        private static Vector2f _position;
        private static Vector2f _dt_position;


        private static void PlatformInit()
        {
        }

        private static void PlatformProcess()
        {
            _buttonStatesLast = _buttonStates;

            int _posX;
            int _posY;
            _buttonStates = (int) SDL.SDL_GetMouseState(out _posX, out _posY);

            _dt_position.X = _position.X - _posX;
            _dt_position.Y = _position.Y - _posY;

            _position.X = _posX;
            _position.Y = _posY;


        }

        private static int PlatformButtonIndex(string buttonName)
        {
            switch (buttonName)
            {
                case "left":
                    return (int) SDL.SDL_BUTTON_LMASK;
                case "middle":
                    return (int) SDL.SDL_BUTTON_MMASK;
                case "right":
                    return (int) SDL.SDL_BUTTON_RMASK;
                default:
                    return 0;
            }
        }

        private static string PlatformButtonName(int buttonIndex)
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

        private static bool PlatformButtonState(int buttonIndex)
        {
            return (_buttonStates & buttonIndex) > 0;
        }

        private static bool PlatformButtonPressed(int buttonIndex)
        {
            return (_buttonStates & buttonIndex) == 1 && (_buttonStatesLast & buttonIndex) == 0;
        }

        private static bool PlatformButtonReleased(int buttonIndex)
        {
            return (_buttonStates & buttonIndex) == 0 && (_buttonStatesLast & buttonIndex) == 1;
        }

        private static Vector2f AxisImpl(string name)
        {
            if (name == "abs")
            {
                return _position;
            }

            if (name == "delta")
            {
                return _dt_position;
            }

            return Vector2f.Zero;
        }
    }
}