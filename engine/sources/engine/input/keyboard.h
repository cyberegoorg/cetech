//! \ingroup Input
//! \{
#ifndef CETECH_KEYBOARD_H
#define CETECH_KEYBOARD_H

//==============================================================================
// Include
//==============================================================================

#include "celib/types.h"
#include "celib/math/types.h"

#include "engine/input/_group.h"

//==============================================================================
// Keyboard
//==============================================================================

//! Keyboard API V1
struct KeyboardApiV1 {
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
};
//! \}
#endif //CETECH_KEYBOARD_H
