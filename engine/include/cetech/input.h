//! \defgroup Input
//! Gamepad, mouse, keyboard
//! \{
#ifndef CETECH_GAMEPAD_H
#define CETECH_GAMEPAD_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

//==============================================================================
// Api
//==============================================================================

//! Gamepad API V0
struct ct_gamepad_a0 {
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
                 const uint32_t axis_index, float* value);

    //! Play rumble
    //! \param gamepad Gamepad
    //! \param strength Rumble strength
    //! \param length Rumble length
    void (*play_rumble)(uint32_t idx,
                        float strength,
                        uint32_t length);

    void (*update)();
};

//==============================================================================
// Api
//==============================================================================

//! Keyboard API V0
struct ct_keyboard_a0 {
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

    void (*update)();
};

//==============================================================================
// Api
//==============================================================================

//! Mouse API V0
struct ct_mouse_a0 {
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
                    const uint32_t axis_index, float* value);

    void (*update)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_GAMEPAD_H
//! \}