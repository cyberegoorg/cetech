#ifndef CETECH_MACHINE_H
#define CETECH_MACHINE_H

#include <stdint.h>
#include <stddef.h>
#include <celib/module.inl>

#define CT_MACHINE_API \
    CE_ID64_0("ct_machine_a0", 0xce8b7f45bc305909ULL)

#define CT_MACHINE_WINDOW_ID \
    CE_ID64_0("window_id", 0x78420ff4a407edc5ULL)

#define CT_MACHINE_WINDOW_WIDTH \
    CE_ID64_0("width", 0x12926f04f655d5d9ULL)

#define CT_MACHINE_WINDOW_HEIGHT \
    CE_ID64_0("height", 0xef1c29a0b6d24e92ULL)

#define CT_MACHINE_TASK \
    CE_ID64_0("machine_task", 0x9497ae794e838591ULL)

//! Machine API V0
struct ct_machine_a0 {
    //! Is gamepad active?
    int (*gamepad_is_active)(int gamepad);

    void (*gamepad_play_rumble)(int gamepad,
                                float strength,
                                uint32_t length);

    const uint64_t * (*events)(uint64_t* n);
};


CE_MODULE(ct_machine_a0);

#endif //CETECH_MACHINE_H
