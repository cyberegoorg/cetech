using CETech.Input;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class KeyboardApi
    {
        public static int ButtonIndex(string buttonName)
        {
            return Keyboard.ButtonIndex(buttonName);
        }

        public static string ButtonName(int buttonIndex)
        {
            return Keyboard.ButtonName(buttonIndex);
        }

        public static bool State(int buttonIndex)
        {
            return Keyboard.State(buttonIndex);
        }

        public static bool Pressed(int buttonIndex)
        {
            return Keyboard.Pressed(buttonIndex);
        }

        public static bool Released(int buttonIndex)
        {
            return Keyboard.Released(buttonIndex);
        }
    }
}