using CETech.CEMath;
using CETech.Input;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class GamepadApi
    {
        public static bool IsActive(int gamepad)
        {
            return Gamepad.IsActive(gamepad);
        }

        public static int ButtonIndex(string buttonName)
        {
            return Gamepad.ButtonIndex(buttonName);
        }

        public static int AxisIndex(string axisName)
        {
            return Gamepad.AxisIndex(axisName);
        }

        public static bool ButtonState(int gamepad, int buttonIndex)
        {
            return Gamepad.ButtonState(gamepad, buttonIndex);
        }

        public static bool ButtonPressed(int gamepad, int buttonIndex)
        {
            return Gamepad.ButtonPressed(gamepad, buttonIndex);
        }

        public static bool ButtonReleased(int gamepad, int buttonIndex)
        {
            return Gamepad.ButtonReleased(gamepad, buttonIndex);
        }

        public static Vec3f Axis(int gamepad, int axisIndex)
        {
            return Gamepad.Axis(gamepad, axisIndex);
        }

        public static void PlayRumble(int gamepad, float strength, uint length)
        {
            Gamepad.PlayRumble(gamepad, strength, length);
        }
    }
}