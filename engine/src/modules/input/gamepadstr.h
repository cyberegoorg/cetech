//==============================================================================
// Includes
//==============================================================================

#include <cetech/kernel/sdl2_machine.h>

static const char *_btn_to_str[GAMEPAD_BTN_MAX] = {
        [GAMEPAD_BTN_INVALID] = "invalid",
        [GAMEPAD_BTN_A] = "a",
        [GAMEPAD_BTN_B] = "b",
        [GAMEPAD_BTN_X] = "x",
        [GAMEPAD_BTN_Y] = "y",
        [GAMEPAD_BTN_BACK] = "back",
        [GAMEPAD_BTN_GUIDE] = "guide",
        [GAMEPAD_BTN_START] = "start",
        [GAMEPAD_BTN_LEFTSTICK] = "left_stick",
        [GAMEPAD_BTN_RIGHTSTICK] = "right_stick",
        [GAMEPAD_BTN_LEFTSHOULDER] = "left_shoulder",
        [GAMEPAD_BTN_RIGHTSHOULDER] = "right_shoulder",
        [GAMEPAD_BTN_DPADUP] = "dpad_up",
        [GAMEPAD_BTN_DPADDOWN] = "dpad_down",
        [GAMEPAD_BTN_DPADLEFT] = "dpad_left",
        [GAMEPAD_BTN_DPADRIGHT] = "dpad_right"
};

static const char *_axis_to_str[GAMEPAD_AXIX_MAX] = {
        [GAMEPAD_AXIS_INVALID] =  "invalid",
        [GAMEPAD_AXIS_LEFT] =  "left",
        [GAMEPAD_AXIS_RIGHT] =  "right",
        [GAMEPAD_AXIS_TRIGER] =  "triger"
};