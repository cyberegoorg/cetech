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

        public static bool State(int buttonIndex)
        {
            return Mouse.State(buttonIndex);
        }

        public static bool Pressed(int buttonIndex)
        {
            return Mouse.Pressed(buttonIndex);
        }

        public static bool Released(int buttonIndex)
        {
            return Mouse.Released(buttonIndex);
        }

        public static Vec2f Axis(string name)
        {
            return Mouse.Axis(name);
        }
    }
}