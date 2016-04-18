using System;
using System.Collections.Generic;
using System.Globalization;
using CETech.CEMath;
using CETech.Utils;
using SDL2;

// ReSharper disable once CheckNamespace

namespace CETech.Input
{
    public static partial class Gamepad
    {
        public static List<IntPtr> GameControlers;
        public static List<IntPtr> Haptics;
        public static List<char[]> LastButtonState;
        public static List<char[]> ActualButtonState;

        private static void InitImpl()
        {
            GameControlers = new List<IntPtr>();
            Haptics = new List<IntPtr>();
            LastButtonState = new List<char[]>();
            ActualButtonState = new List<char[]>();

            var add_maping = SDL.SDL_GameControllerAddMapping(CONTROLERDB);
            if (add_maping < 0)
            {
                Log.Error("input.gamepad", "Maping error");
            }

            var num_joy = SDL.SDL_NumJoysticks();
            for (var i = 0; i < num_joy; ++i)
            {
                if (SDL.SDL_IsGameController(i) == SDL.SDL_bool.SDL_FALSE)
                {
                    continue;
                }

                var controler = SDL.SDL_GameControllerOpen(i);
                var joy = SDL.SDL_GameControllerGetJoystick(controler);

                GameControlers.Add(controler);
                LastButtonState.Add(new char[(int) GamepadButton.Max]);
                ActualButtonState.Add(new char[(int) GamepadButton.Max]);

                if (SDL.SDL_JoystickIsHaptic(joy) == 1)
                {
                    var haptic = SDL.SDL_HapticOpenFromJoystick(joy);
                    Haptics.Add(haptic);
                    Log.Info("input.gamepad", "Gamepad {0} has haptic support", GameControlers.Count);
                }
                else
                {
                    Haptics.Add(IntPtr.Zero);
                }
            }

            Log.Info("input.gamepad", "Num gamepads: {0}", GameControlers.Count);
        }

        private static void ShutdownImpl()
        {
            for (var i = 0; i < GameControlers.Count; ++i)
            {
                SDL.SDL_GameControllerClose(GameControlers[i]);
            }
        }

        private static void ProcessImpl()
        {
            for (int i = 0; i < GameControlers.Count; ++i)
            {
                var controler_id = GameControlers[i];

                for (int j = 0; j < (int) GamepadButton.Max; ++j)
                {
                    LastButtonState[i][j] = ActualButtonState[i][j];
                    ActualButtonState[i][j] = (char) SDL.SDL_GameControllerGetButton(controler_id, GamepadButton2SDL((GamepadButton)j));
                }
            }
        }

        private static int ButtonIndexImpl(string buttonName)
        {
            switch (buttonName)
            {
                case "a": return (int) GamepadButton.A;
                case "b": return (int) GamepadButton.B;
                case "x": return (int) GamepadButton.X;
                case "y": return (int) GamepadButton.Y;
                case "back": return (int) GamepadButton.Back;
                case "guide": return (int) GamepadButton.Guide;
                case "start": return (int) GamepadButton.Start;
                case "left_stick": return (int) GamepadButton.Leftstick;
                case "right_stick": return (int) GamepadButton.Rightstick;
                case "left_shoulder": return (int) GamepadButton.Leftshoulder;
                case "right_shoulder": return (int) GamepadButton.Rightshoulder;
                case "dpad_up": return (int) GamepadButton.DpadUp;
                case "dpad_down": return (int) GamepadButton.DpadDown;
                case "dpad_left": return (int) GamepadButton.DpadLeft;
                case "dpad_right": return (int) GamepadButton.DpadRight;
                default: return (int) GamepadButton.INVALID;
            }
        }

        private static SDL.SDL_GameControllerButton GamepadButton2SDL(GamepadButton btn)
        {
            switch (btn)
            {
                case GamepadButton.A:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_A;

                case GamepadButton.B:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_B;

                case GamepadButton.X:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_X;

                case GamepadButton.Y:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_Y;

                case GamepadButton.Back:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_BACK;

                case GamepadButton.Guide:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_GUIDE;

                case GamepadButton.Start:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_START;

                case GamepadButton.Leftstick:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_LEFTSTICK;

                case GamepadButton.Rightstick:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_RIGHTSTICK;

                case GamepadButton.Leftshoulder:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_LEFTSHOULDER;

                case GamepadButton.Rightshoulder:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;

                case GamepadButton.DpadUp:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_DPAD_UP;

                case GamepadButton.DpadDown:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_DPAD_DOWN;

                case GamepadButton.DpadLeft:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_DPAD_LEFT;

                case GamepadButton.DpadRight:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_DPAD_RIGHT;

                default:
                    return SDL.SDL_GameControllerButton.SDL_CONTROLLER_BUTTON_INVALID;
            }
        }

        private static bool ButtonStateImpl(int gamepad, int buttonIndex)
        {
            return ActualButtonState[gamepad][buttonIndex] == 1;
        }

        private static int AxisIndexImpl(string axisName)
        {
            switch (axisName)
            {
                case "left":
                    return (int) GamepadAxis.Left;

                case "right":
                    return (int) GamepadAxis.Right;

                case "triger":
                    return (int) GamepadAxis.Triger;

                default:
                    return (int) GamepadAxis.Invalid;
            }
        }

        private static Vec3f AxisImpl(int gamepad, int axisIndex)
        {
            var controler_id = GameControlers[gamepad];

            int axis_x, axis_y;

            switch ((GamepadAxis) axisIndex)
            {
                case GamepadAxis.Left:
                    axis_x = SDL.SDL_GameControllerGetAxis(controler_id,
                        SDL.SDL_GameControllerAxis.SDL_CONTROLLER_AXIS_LEFTX);
                    axis_y = SDL.SDL_GameControllerGetAxis(controler_id,
                        SDL.SDL_GameControllerAxis.SDL_CONTROLLER_AXIS_LEFTY);
                    break;

                case GamepadAxis.Right:
                    axis_x = SDL.SDL_GameControllerGetAxis(controler_id,
                        SDL.SDL_GameControllerAxis.SDL_CONTROLLER_AXIS_RIGHTX);
                    axis_y = SDL.SDL_GameControllerGetAxis(controler_id,
                        SDL.SDL_GameControllerAxis.SDL_CONTROLLER_AXIS_RIGHTY);
                    break;

                case GamepadAxis.Triger:
                    axis_x = SDL.SDL_GameControllerGetAxis(controler_id,
                        SDL.SDL_GameControllerAxis.SDL_CONTROLLER_AXIS_TRIGGERLEFT);
                    axis_y = SDL.SDL_GameControllerGetAxis(controler_id,
                        SDL.SDL_GameControllerAxis.SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
                    break;

                default:
                    return Vec3f.Zero;
            }


            var axis_x_norm = axis_x/(float) short.MaxValue;
            var axis_y_norm = axis_y/(float) short.MaxValue;

            var dead_zone = 0.2f;
            var start_low_zone = 1.0f - dead_zone;
            var start_high_zone = 1.0f + dead_zone;

            var pos_axis_x = axis_x_norm + 1.0f;
            if ((pos_axis_x >= start_low_zone) && (pos_axis_x <= start_high_zone))
            {
                axis_x_norm = 0.0f;
            }

            var pos_axis_y = axis_y_norm + 1.0f;
            if ((pos_axis_y >= start_low_zone) && (pos_axis_y <= start_high_zone))
            {
                axis_y_norm = 0.0f;
            }

            return new Vec3f(axis_x_norm, axis_y_norm, 0.0f);
        }

        private static int GetNumGamepadImpl()
        {
            return GameControlers.Count;
        }

        private static bool IsActiveImpl(int gamepad)
        {
            return GameControlers.Count != 0 && gamepad < GameControlers.Count; // TODO:
        }

        private static void PlayRumbleimpl(int gamepad, float strength, uint length)
        {
            var haptic = Haptics[gamepad];
            SDL.SDL_HapticRumblePlay(haptic, strength, length);
        }

        private enum GamepadButton
        {
            INVALID = 0,
            A,
            B,
            X,
            Y,
            Back,
            Guide,
            Start,
            Leftstick,
            Rightstick,
            Leftshoulder,
            Rightshoulder,
            DpadUp,
            DpadDown,
            DpadLeft,
            DpadRight,
            Max
        }

        private enum GamepadAxis
        {
            Invalid = 0,
            Left = 1,
            Right = 2,
            Triger = 3
        }

        private static bool ButtonPressedImpl(int gamepad, int buttonIndex)
        {
            return ActualButtonState[gamepad][buttonIndex] == 1 && LastButtonState[gamepad][buttonIndex] == 0;
        }

        private static bool ButtonReleasedImpl(int gamepad, int buttonIndex)
        {
            return ActualButtonState[gamepad][buttonIndex] == 0 && LastButtonState[gamepad][buttonIndex] == 1;
        }
    }
}