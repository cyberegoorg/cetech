namespace CETech
{
    public static partial class Keyboard
    {
        public static void Init()
        {
            PlatformInit();
        }

        public static void ProcessKeyboard()
        {
            PlatformProcessKeyboard();
        }

        public static int ButtonIndex(string buttonName)
        {
            return PlatformButtonIndex(buttonName);
        }

        public static string ButtonName(int buttonIndex)
        {
            return PlatformButtonName(buttonIndex);
        }

        public static bool ButtonState(int buttonIndex)
        {
            return PlatformButtonState(buttonIndex);
        }

        public static bool ButtonPressed(int buttonIndex)
        {
            return PlatformButtonPressed(buttonIndex);
        }

        public static bool ButtonReleased(int buttonIndex)
        {
            return PlatformButtonReleased(buttonIndex);
        }
    }
}