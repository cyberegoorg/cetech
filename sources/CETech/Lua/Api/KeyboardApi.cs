using CETech.Input;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    class KeyboardApi
    {
        public static int ButtonIndex(string buttonName)
        {
            return Keyboard.ButtonIndex(buttonName);
        }

        public static string ButtonName(int buttonIndex)
        {
            return Keyboard.ButtonName(buttonIndex);
        }

        public static bool ButtonState(int buttonIndex)
        {
            return Keyboard.ButtonState(buttonIndex);
        }

        public static bool ButtonPressed(int buttonIndex)
        {
            return Keyboard.ButtonPressed(buttonIndex);
        }

        public static bool ButtonReleased(int buttonIndex)
        {
            return Keyboard.ButtonReleased(buttonIndex);
        }
    }
}