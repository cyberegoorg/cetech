using CETech.CEMath;

namespace CETech.Input
{
    /// <summary>
    ///     Keyboard system
    /// </summary>
    public static partial class Gamepad
    {
        public static void Init()
        {
            InitImpl();
        }

        public static void Shutdown()
        {
            ShutdownImpl();
        }

        public static void Process()
        {
            ProcessImpl();
        }

        public static int ButtonIndex(string buttonName)
        {
            return ButtonIndexImpl(buttonName);
        }

        public static int AxisIndex(string axisName)
        {
            return AxisIndexImpl(axisName);
        }

        public static int GetNumGamepad()
        {
            return GetNumGamepadImpl();
        }

        public static bool IsActive(int gamepad)
        {
            return IsActiveImpl(gamepad);
        }

        public static Vec3f Axis(int gamepad, int axisIndex)
        {
            return AxisImpl(gamepad, axisIndex);
        }

        public static bool ButtonPressed(int gamepad, int buttonIndex)
        {
            return ButtonPressedImpl(gamepad, buttonIndex);
        }

        public static bool ButtonReleased(int gamepad, int buttonIndex)
        {
            return ButtonReleasedImpl(gamepad, buttonIndex);
        }

        public static bool ButtonState(int gamepad, int buttonIndex)
        {
            return ButtonStateImpl(gamepad, buttonIndex);
        }

        public static void PlayRumble(int gamepad, float strength, uint length)
        {
            PlayRumbleimpl(gamepad, strength, length);
        }
    }
}