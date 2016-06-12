using CETech.CEMath;
using CETech.Input;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class MouseApi
    {
        public static int ButtonIndex(string buttonName)
        {
            return Mouse.ButtonIndex(buttonName);
        }

        public static string ButtonName(int buttonIndex)
        {
            return Mouse.ButtonName(buttonIndex);
        }

        public static bool ButtonState(int buttonIndex)
        {
            return Mouse.ButtonState(buttonIndex);
        }

        public static bool ButtonPressed(int buttonIndex)
        {
            return Mouse.ButtonPressed(buttonIndex);
        }

        public static bool ButtonReleased(int buttonIndex)
        {
            return Mouse.ButtonReleased(buttonIndex);
        }

        public static int AxisIndex(string buttonName)
        {
            return Mouse.AxisIndex(buttonName);
        }

        public static Vec3f Axis(int axisIndex)
        {
            return Mouse.Axis(axisIndex);
        }
    }
}