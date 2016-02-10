namespace CETech.Input
{
    /// <summary>
    ///     Keyboard system
    /// </summary>
    public static partial class Keyboard
    {
        /// <summary>
        ///     Init keyboard system.
        /// </summary>
        public static void Init()
        {
            PlatformInit();
        }

        /// <summary>
        ///     Process keyboard
        /// </summary>
        public static void Process()
        {
            PlatformProcessKeyboard();
        }


        /// <summary>
        ///     Get button index.
        /// </summary>
        /// <param name="buttonName">Button name</param>
        /// <returns>Button index</returns>
        public static int ButtonIndex(string buttonName)
        {
            return PlatformButtonIndex(buttonName);
        }

        /// <summary>
        ///     Get button name
        /// </summary>
        /// <param name="buttonIndex">Button index</param>
        /// <returns>Button name</returns>
        public static string ButtonName(int buttonIndex)
        {
            return PlatformButtonName(buttonIndex);
        }

        /// <summary>
        ///     Button state in curent frame
        /// </summary>
        /// <param name="buttonIndex">Button index</param>
        /// <returns>True if button is now down</returns>
        public static bool ButtonState(int buttonIndex)
        {
            return PlatformButtonState(buttonIndex);
        }

        /// <summary>
        ///     Is button pressed in curent frame. Last frame: 0, Curent: 1
        /// </summary>
        /// <param name="buttonIndex">Button index</param>
        /// <returns>True if button is pressed</returns>
        public static bool ButtonPressed(int buttonIndex)
        {
            return PlatformButtonPressed(buttonIndex);
        }

        /// <summary>
        ///     Is button released in curent frame. Last frame: 1, Curent: 0
        /// </summary>
        /// <param name="buttonIndex">Button index</param>
        /// <returns>True if button is released</returns>
        public static bool ButtonReleased(int buttonIndex)
        {
            return PlatformButtonReleased(buttonIndex);
        }
    }
}