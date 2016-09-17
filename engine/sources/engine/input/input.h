#ifndef CETECH_INPUT_H
#define CETECH_INPUT_H

//==============================================================================
// Include
//==============================================================================

#include "celib/types.h"
#include "celib/math/types.h"

//==============================================================================
// Keyboard
//==============================================================================

int keyboard_init();

void keyboard_shutdown();

void keyboard_process();

u32 keyboard_button_index(const char *button_name);

const char *keyboard_button_name(const u32 button_index);

int keyboard_button_state(const u32 button_index);

int keyboard_button_pressed(const u32 button_index);

int keyboard_button_released(const u32 button_index);

//==============================================================================
// Mouse
//==============================================================================

int mouse_init();

void mouse_shutdown();

void mouse_process();

u32 mouse_button_index(const char *button_name);

const char *mouse_button_name(const u32 button_index);

int mouse_button_state(const u32 button_index);

int mouse_button_pressed(const u32 button_index);

int mouse_button_released(const u32 button_index);

u32 mouse_axis_index(const char *axis_name);

const char *mouse_axis_name(const u32 axis_index);

vec2f_t mouse_axis(const u32 axis_index);

//==============================================================================
// Gamepad
//==============================================================================

typedef struct {
    u8 idx;
} gamepad_t;

int gamepad_init();

void gamepad_shutdown();

void gamepad_process();

int gamepad_is_active(gamepad_t gamepad);

u32 gamepad_button_index(const char *button_name);

const char *gamepad_button_name(const u32 button_index);

int gamepad_button_state(gamepad_t gamepad,
                         const u32 button_index);

int gamepad_button_pressed(gamepad_t gamepad,
                           const u32 button_index);

int gamepad_button_released(gamepad_t gamepad,
                            const u32 button_index);

u32 gamepad_axis_index(const char *axis_name);

const char *gamepad_axis_name(const u32 axis_index);

vec2f_t gamepad_axis(gamepad_t gamepad,
                     const u32 axis_index);

void gamepad_play_rumble(gamepad_t gamepad,
                         float strength,
                         u32 length);

#endif //CETECH_INPUT_H
