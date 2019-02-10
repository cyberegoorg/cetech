#ifndef CETECH_CONTROLERS_H
#define CETECH_CONTROLERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


#define CT_CONTROLERS_API \
    CE_ID64_0("ct_controlers_a0", 0x81e928c4fbba526dULL)

#define CONTROLERS_I \
    CE_ID64_0("ct_controlers_i0", 0x38a3a48646b9f277ULL)

#define CONTROLER_BUTTON \
    CE_ID64_0("button", 0x1e232360426fac96ULL)

#define CONTROLER_AXIS \
    CE_ID64_0("axis", 0xeb6778864cd804e1ULL)

#define CONTROLER_POSITION_X \
    CE_ID64_0("position.x", 0x46b9c080961061caULL)

#define CONTROLER_POSITION_Y \
    CE_ID64_0("position.y", 0xca45b3734c0b3e81ULL)

#define CONTROLER_POSITION_Z \
    CE_ID64_0("position.z", 0x1c9cfdf1ce5b0b7aULL)

#define CONTROLER_KEYCODE \
    CE_ID64_0("keycode", 0xe8153a02000cbab3ULL)

#define CONTROLER_TEXT \
    CE_ID64_0("text", 0x887e193c80929e7ULL)

#define CONTROLER_ID \
    CE_ID64_0("id", 0xb0d09cc783d6a0ecULL)


typedef struct ct_controlers_i0 {
    uint64_t (*name)();

    //! Is gamepad active?
    int (*is_active)(uint32_t idx);

    //! Return button index
    uint32_t (*button_index)(const char *button_name);

    //! Return button name
    const char *(*button_name)(const uint32_t button_index);

    //! Return button state
    int (*button_state)(uint32_t idx,
                        const uint32_t button_index);

    //! Is button pressed?
    int (*button_pressed)(uint32_t idx,
                          const uint32_t button_index);

    //! Is button released?
    int (*button_released)(uint32_t idx,
                           const uint32_t button_index);

    //! Return axis index
    uint32_t (*axis_index)(const char *axis_name);

    //! Return axis name
    const char *(*axis_name)(const uint32_t axis_index);

    //! Return axis value
    void (*axis)(uint32_t idx,
                 const uint32_t axis_index,
                 float *value);

    //! Play rumble
    void (*play_rumble)(uint32_t idx,
                        float strength,
                        uint32_t length);

    char *(*text)(uint32_t idx);
}ct_controlers_i0;

struct ct_controlers_a0 {
    struct ct_controlers_i0* (*get)(uint64_t name);
};

CE_MODULE(ct_controlers_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_CONTROLERS_H
