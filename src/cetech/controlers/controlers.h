#ifndef CETECH_CONTROLERS_H
#define CETECH_CONTROLERS_H


//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <celib/module.inl>


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


//==============================================================================
// Api
//==============================================================================

struct ct_controlers_i0 {
    uint64_t (*name)();

    //! Is gamepad active?
    //! \param gamepad Gamepad
    //! \return 1 if active else 0
    int (*is_active)(uint32_t idx);

    //! Return button index
    //! \param button_name Button name
    //! \return Button index
    uint32_t (*button_index)(const char *button_name);

    //! Return button name
    //! \param button_index Button index
    //! \return Button name
    const char *(*button_name)(const uint32_t button_index);

    //! Return button state
    //! \param button_index Button index
    //! \return 1 if button is in current frame down else 0
    int (*button_state)(uint32_t idx,
                        const uint32_t button_index);

    //! Is button pressed?
    //! \param button_index Button index
    //! \return 1 if button is in current frame pressed else 0
    int (*button_pressed)(uint32_t idx,
                          const uint32_t button_index);

    //! Is button released?
    //! \param button_index Button index
    //! \return 1 if button is in current frame released else 0
    int (*button_released)(uint32_t idx,
                           const uint32_t button_index);

    //! Return axis index
    //! \param axis_name Axis name
    //! \return Axis index
    uint32_t (*axis_index)(const char *axis_name);

    //! Return axis name
    //! \param axis_index Axis index
    //! \return Axis name
    const char *(*axis_name)(const uint32_t axis_index);

    //! Return axis value
    //! \param axis_index Axis index
    //! \return Axis value
    void (*axis)(uint32_t idx,
                 const uint32_t axis_index,
                 float *value);

    //! Play rumble
    //! \param gamepad Gamepad
    //! \param strength Rumble strength
    //! \param length Rumble length
    void (*play_rumble)(uint32_t idx,
                        float strength,
                        uint32_t length);

    char *(*text)(uint32_t idx);
};

struct ct_controlers_a0 {
    struct ct_controlers_i0* (*get)(uint64_t name);
};

CE_MODULE(ct_controlers_a0);

#endif //CETECH_CONTROLERS_H
