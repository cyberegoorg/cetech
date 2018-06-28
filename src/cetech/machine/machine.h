#ifndef CETECH_MACHINE_H
#define CETECH_MACHINE_H

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <stddef.h>
#include <corelib/module.inl>

//==============================================================================
// Machine
//==============================================================================


//! Machine API V0
struct ct_machine_a0 {
    void (*update)(float dt);

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
