#ifndef CETECH_INPUT_H
#define CETECH_INPUT_H

//==============================================================================
// Include
//==============================================================================

#include "celib/types.h"
#include "celib/math/types.h"

#include "engine/input/types.h"

//==============================================================================
// Keyboard
//==============================================================================

//! Init keyboard system
//! \return 1 if init ok else 0
int keyboard_init();

//! Shutdown keyboard system
void keyboard_shutdown();

//! Process keyboard
void keyboard_process();

//! Return button index
//! \param button_name Button name
//! \return Button index
u32 keyboard_button_index(const char *button_name);

//! Return button name
//! \param button_index Button index
//! \return Button name
const char *keyboard_button_name(const u32 button_index);

//! Return button state
//! \param button_index Button index
//! \return 1 if button is in current frame down else 0
int keyboard_button_state(const u32 button_index);

//! Is button pressed?
//! \param button_index Button index
//! \return 1 if button is in current frame pressed else 0
int keyboard_button_pressed(const u32 button_index);

//! Is button released?
//! \param button_index Button index
//! \return 1 if button is in current frame released else 0
int keyboard_button_released(const u32 button_index);

//==============================================================================
// Mouse
//==============================================================================

//! Init mouse keyboard system
//! \return 1 if init ok else 0
int mouse_init();

//! Shutdown keyboard system
void mouse_shutdown();

//! Process mouse
void mouse_process();

//! Return button index
//! \param button_name Button name
//! \return Button index
u32 mouse_button_index(const char *button_name);

//! Return button name
//! \param button_index Button index
//! \return Button name
const char *mouse_button_name(const u32 button_index);

//! Return button state
//! \param button_index Button index
//! \return 1 if button is in current frame down else 0
int mouse_button_state(const u32 button_index);

//! Is button pressed?
//! \param button_index Button index
//! \return 1 if button is in current frame pressed else 0
int mouse_button_pressed(const u32 button_index);

//! Is button released?
//! \param button_index Button index
//! \return 1 if button is in current frame released else 0
int mouse_button_released(const u32 button_index);

//! Return axis index
//! \param axis_name Axis name
//! \return Axis index
u32 mouse_axis_index(const char *axis_name);

//! Return axis name
//! \param axis_index Axis index
//! \return Axis name
const char *mouse_axis_name(const u32 axis_index);

//! Return axis value
//! \param axis_index Axis index
//! \return Axis value
cel_vec2f_t mouse_axis(const u32 axis_index);

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
int gamepad_is_active(gamepad_t gamepad);

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
int gamepad_button_state(gamepad_t gamepad,
                         const u32 button_index);

//! Is button pressed?
//! \param button_index Button index
//! \return 1 if button is in current frame pressed else 0
int gamepad_button_pressed(gamepad_t gamepad,
                           const u32 button_index);

//! Is button released?
//! \param button_index Button index
//! \return 1 if button is in current frame released else 0
int gamepad_button_released(gamepad_t gamepad,
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
cel_vec2f_t gamepad_axis(gamepad_t gamepad,
                         const u32 axis_index);

//! Play rumble
//! \param gamepad Gamepad
//! \param strength Rumble strength
//! \param length Rumble length
void gamepad_play_rumble(gamepad_t gamepad,
                         float strength,
                         u32 length);

#endif //CETECH_INPUT_H
