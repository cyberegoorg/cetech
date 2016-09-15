using CETech.CEMath;
using SDL2;

// ReSharper disable once CheckNamespace

namespace CETech.Input
{
    public static partial class Mouse
    {
        private static int _buttonStates;
        private static int _buttonStatesLast;
        private static Vec3f _position;
        private static Vec3f _dt_position;

        private static void InitImpl()
        {
        }

        private static void ProcessImpl()
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

        private static int ButtonIndexImpl(string buttonName)
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

        private static string ButtonNameImpl(int buttonIndex)
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

        private static bool ButtonStateImpl(int buttonIndex)
        {
            return (_buttonStates & buttonIndex) > 0;
        }

        private static bool ButtonPressedImpl(int buttonIndex)
        {
            return (_buttonStates & buttonIndex) == 1 && (_buttonStatesLast & buttonIndex) == 0;
        }

        private static bool ButtonReleasedImpl(int buttonIndex)
        {
            return (_buttonStates & buttonIndex) == 0 && (_buttonStatesLast & buttonIndex) == 1;
        }

        private static Vec3f AxisImpl(int axisIndex)
        {
            switch ((MouseAxis) axisIndex)
            {
                case MouseAxis.Absolute:
                    return _position;

                case MouseAxis.Delta:
                    return _dt_position;

                default:
                    return Vec3f.Zero;
            }
        }

        public static int AxisIndex(string axisName)
        {
            switch (axisName)
            {
                case "absolute":
                    return (int) MouseAxis.Absolute;

                case "delta":
                    return (int) MouseAxis.Delta;

                default:
                    return (int) MouseAxis.Invalid;
            }
        }

        private enum MouseAxis
        {
            Invalid = 0,
            Absolute = 1,
            Delta = 2
        }
    }
}