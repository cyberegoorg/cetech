#ifndef CETECH_MACHINE_H
#define CETECH_MACHINE_H

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <stddef.h>
#include <corelib/module.inl>

#define CT_MACHINE_WINDOW_ID \
    CT_ID64_0("window_id", 0x78420ff4a407edc5ULL)

#define CT_MACHINE_WINDOW_WIDTH \
    CT_ID64_0("width", 0x12926f04f655d5d9ULL)

#define CT_MACHINE_WINDOW_HEIGHT \
    CT_ID64_0("height", 0xef1c29a0b6d24e92ULL)

//==============================================================================
// Machine
//==============================================================================

//! Machine API V0
struct ct_machine_a0 {
    //! Is gamepad active?
    //! \param gamepad Gamepad ID
    //! \return 1 if is active else 0
    int (*gamepad_is_active)(int gamepad);

    //!
    //! \param gamepad
    //! \param strength
    //! \param length
    void (*gamepad_play_rumble)(int gamepad,
                                float strength,
                                uint32_t length);
};

CT_MODULE(ct_machine_a0);

#endif //CETECH_MACHINE_H
