#ifndef CETECH_INPUT_TYPES_H
#define CETECH_INPUT_TYPES_H

#include "celib/types.h"
#include "celib/math/types.h"

//==============================================================================
// Keyboard
//==============================================================================

struct KeyboardApiV1 {
    u32 (*button_index)(const char *button_name);
    const char *(*button_name)(const u32 button_index);
    int (*button_state)(u32 idx,const u32 button_index);
    int (*button_pressed)(u32 idx,const u32 button_index);
    int (*button_released)(u32 idx,const u32 button_index);
};


//==============================================================================
// Mouse
//==============================================================================

struct MouseApiV1 {
    int (*is_active)();
    u32 (*button_index)(const char *button_name);
    const char *(*button_name)(const u32 button_index);
    int (*button_state)(u32 idx,const u32 button_index);
    int (*button_pressed)(u32 idx,const u32 button_index);
    int (*button_released)(u32 idx,const u32 button_index);
    u32 (*axis_index)(const char *axis_name);
    const char *(*axis_name)(const u32 axis_index);
    cel_vec2f_t (*axis)(u32 idx,const u32 axis_index);
};


//==============================================================================
// Gamepad
//==============================================================================

struct GamepadApiV1 {
    int (*is_active)(u32 idx);

    u32 (*button_index)(const char *button_name);

    const char *(*button_name)(const u32 button_index);

    int (*button_state)(u32 idx,
                        const u32 button_index);

    int (*button_pressed)(u32 idx,
                          const u32 button_index);

    int (*button_released)(u32 idx,
                           const u32 button_index);

    u32 (*axis_index)(const char *axis_name);

    const char *(*axis_name)(const u32 axis_index);

    cel_vec2f_t (*axis)(u32 idx,
                        const u32 axis_index);

    void (*play_rumble)(u32 idx,
                        float strength,
                        u32 length);
};


#endif //CETECH_INPUT_TYPES_H
