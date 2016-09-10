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

void mouse_axis(const u32 axis_index,
                vec2f_t position);

#endif //CETECH_INPUT_H
