#ifndef CETECH_GAMEPAD_H
#define CETECH_GAMEPAD_H

//==============================================================================
// Include
//==============================================================================

#include "celib/types.h"
#include "celib/math/types.h"

#include "engine/input/types.h"

//==============================================================================
// Gamepad
//==============================================================================

//! Init mouse gamepad system
//! \return 1 if init ok else 0
int gamepad_init();

//! Shutdown gamepad system
void gamepad_shutdown();

//! Process gamepad
void gamepad_process();

//! Is gamepad active?
//! \param gamepad Gamepad
//! \return 1 if active else 0
int gamepad_is_active(u32 idx );

//! Return button index
//! \param button_name Button name
//! \return Button index
u32 gamepad_button_index(const char *button_name);

//! Return button name
//! \param button_index Button index
//! \return Button name
const char *gamepad_button_name(const u32 button_index);

//! Return button state
//! \param button_index Button index
//! \return 1 if button is in current frame down else 0
int gamepad_button_state(u32 idx ,
                         const u32 button_index);

//! Is button pressed?
//! \param button_index Button index
//! \return 1 if button is in current frame pressed else 0
int gamepad_button_pressed(u32 idx ,
                           const u32 button_index);

//! Is button released?
//! \param button_index Button index
//! \return 1 if button is in current frame released else 0
int gamepad_button_released(u32 idx ,
                            const u32 button_index);

//! Return axis index
//! \param axis_name Axis name
//! \return Axis index
u32 gamepad_axis_index(const char *axis_name);

//! Return axis name
//! \param axis_index Axis index
//! \return Axis name
const char *gamepad_axis_name(const u32 axis_index);

//! Return axis value
//! \param axis_index Axis index
//! \return Axis value
cel_vec2f_t gamepad_axis(u32 idx ,
                         const u32 axis_index);

//! Play rumble
//! \param gamepad Gamepad
//! \param strength Rumble strength
//! \param length Rumble length
void gamepad_play_rumble(u32 idx ,
                         float strength,
                         u32 length);

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

static const struct GamepadApiV1 GamepadApiV1 = {
        .is_active = gamepad_is_active,
        .button_index = gamepad_button_index,
        .button_name = gamepad_button_name,
        .button_state = gamepad_button_state,
        .button_pressed = gamepad_button_pressed,
        .button_released = gamepad_button_released,
        .axis_index = gamepad_axis_index,
        .axis_name = gamepad_axis_name,
        .axis = gamepad_axis,
        .play_rumble = gamepad_play_rumble,
};

#endif //CETECH_GAMEPAD_H
