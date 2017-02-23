//! \ingroup Input
//! \{
#ifndef CETECH_GAMEPAD_H
#define CETECH_GAMEPAD_H

//==============================================================================
// Include
//==============================================================================

#include "celib/types.h"
#include "celib/math/types.h"

#include "engine/input/_group.h"

//==============================================================================
// Gamepad
//==============================================================================

//! Gamepad API V0
struct GamepadApiV0 {
    //! Is gamepad active?
    //! \param gamepad Gamepad
    //! \return 1 if active else 0
    int (*is_active)(u32 idx);

    //! Return button index
    //! \param button_name Button name
    //! \return Button index
    u32 (*button_index)(const char *button_name);

    //! Return button name
    //! \param button_index Button index
    //! \return Button name
    const char *(*button_name)(const u32 button_index);

    //! Return button state
    //! \param button_index Button index
    //! \return 1 if button is in current frame down else 0
    int (*button_state)(u32 idx,
                        const u32 button_index);

    //! Is button pressed?
    //! \param button_index Button index
    //! \return 1 if button is in current frame pressed else 0
    int (*button_pressed)(u32 idx,
                          const u32 button_index);

    //! Is button released?
    //! \param button_index Button index
    //! \return 1 if button is in current frame released else 0
    int (*button_released)(u32 idx,
                           const u32 button_index);

    //! Return axis index
    //! \param axis_name Axis name
    //! \return Axis index
    u32 (*axis_index)(const char *axis_name);

    //! Return axis name
    //! \param axis_index Axis index
    //! \return Axis name
    const char *(*axis_name)(const u32 axis_index);

    //! Return axis value
    //! \param axis_index Axis index
    //! \return Axis value
    cel_vec2f_t (*axis)(u32 idx,
                        const u32 axis_index);

    //! Play rumble
    //! \param gamepad Gamepad
    //! \param strength Rumble strength
    //! \param length Rumble length
    void (*play_rumble)(u32 idx,
                        float strength,
                        u32 length);
};


#endif //CETECH_GAMEPAD_H
//! \}