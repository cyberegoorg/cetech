#ifndef CETECH_KEYBOARD_H
#define CETECH_KEYBOARD_H

//==============================================================================
// Include
//==============================================================================

#include "celib/types.h"
#include "celib/math/types.h"

#include "engine/input/types.h"

//==============================================================================
// Keyboard
//==============================================================================

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
int keyboard_button_state(u32 idx,
                          const u32 button_index);

//! Is button pressed?
//! \param button_index Button index
//! \return 1 if button is in current frame pressed else 0
int keyboard_button_pressed(u32 idx,
                            const u32 button_index);

//! Is button released?
//! \param button_index Button index
//! \return 1 if button is in current frame released else 0
int keyboard_button_released(u32 idx,
                             const u32 button_index);

static const struct KeyboardApiV1 KeyboardApiV1 = {
        .button_index = keyboard_button_index,
        .button_name = keyboard_button_name,
        .button_state = keyboard_button_state,
        .button_pressed = keyboard_button_pressed,
        .button_released = keyboard_button_released,
};

#endif //CETECH_KEYBOARD_H
