#ifndef CETECH_MOUSE_H
#define CETECH_MOUSE_H

//==============================================================================
// Include
//==============================================================================

#include "celib/types.h"
#include "celib/math/types.h"

#include "engine/input/types.h"

//==============================================================================
// Mouse
//==============================================================================

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
int mouse_button_state(u32 idx, const u32 button_index);

//! Is button pressed?
//! \param button_index Button index
//! \return 1 if button is in current frame pressed else 0
int mouse_button_pressed(u32 idx,const u32 button_index);

//! Is button released?
//! \param button_index Button index
//! \return 1 if button is in current frame released else 0
int mouse_button_released(u32 idx,const u32 button_index);

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
cel_vec2f_t mouse_axis(u32 idx, const u32 axis_index);

static const struct MouseApiV1 MouseApiV1 = {
        .button_index = mouse_button_index,
        .button_name = mouse_button_name,
        .button_state = mouse_button_state,
        .button_pressed = mouse_button_pressed,
        .button_released = mouse_button_released,
        .axis_index = mouse_axis_index,
        .axis_name = mouse_axis_name,
        .axis = mouse_axis,
};


#endif //CETECH_MOUSE_H
