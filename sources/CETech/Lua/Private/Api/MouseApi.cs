using CETech.Input;
using CETech.World;
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

        public static Vector2f Axis(string name)
        {
            return Mouse.Axis(name);
        }
    }
}