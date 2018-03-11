#ifndef CETECH_MOUSE_H
#define CETECH_MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================
#include <stdint.h>

#define MOUSE_EBUS_NAME "mouse"
#define MOUSE_EBUS CT_ID64_0(MOUSE_EBUS_NAME)

//! Mouse button status
struct ct_mouse_event {
    uint32_t button;            //!< Button state
};

struct ct_mouse_wheel_event {
    float pos[2];               //!< Actual position
};

struct ct_mouse_move_event {
    float pos[2];       //!< Actual position
};

enum {
    EVENT_MOUSE_INVALID = 0,   //!< Invalid type

    EVENT_MOUSE_MOVE,    //!< Mouse move
    EVENT_MOUSE_UP,      //!< Mouse button up
    EVENT_MOUSE_DOWN,    //!< Mouse button down
    EVENT_MOUSE_WHEEL,    //!< Mouse wheel move

};

//! Mouse button enum
enum {
    MOUSE_BTN_UNKNOWN = 0, //!< Invalid button

    MOUSE_BTN_LEFT = 1,    //!< Left mouse button
    MOUSE_BTN_MIDLE = 2,   //!< Midle mouse button
    MOUSE_BTN_RIGHT = 3,   //!< Right mouse button

    MOUSE_BTN_MAX = 8      //!< Max mouse button
};

//! Mouse axis enum
enum {
    MOUSE_AXIS_UNKNOWN = 0, //!< Invalid axis

    MOUSE_AXIS_ABSOULTE = 1, //!< Absolute mouse position
    MOUSE_AXIS_RELATIVE = 2, //!< Relative mouse position
    MOUSE_AXIS_WHEEL = 3, //!< Wheel relative move

    MOUSE_AXIS_MAX = 8 //!< Max axis
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
                 const uint32_t axis_index,
                 float *value);

};

#ifdef __cplusplus
}
#endif

#endif //CETECH_MOUSE_H
//! \}