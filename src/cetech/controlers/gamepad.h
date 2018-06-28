#ifndef CETECH_GAMEPAD_H
#define CETECH_GAMEPAD_H


//==============================================================================
// Includes
//==============================================================================
#include <stdint.h>

#define GAMEPAD_EBUS_NAME "gamepad"

enum {
    GAMEPAD_EBUS = 0x3913b95f
};

//! Gamepad move event
struct ct_gamepad_move_event {
    uint8_t gamepad_id;         //!< Gamepad id
    uint32_t axis;              //!< Axis id
    float position[2];  //!< Position
};

//! Gamepad button event
struct ct_gamepad_btn_event {
    uint8_t gamepad_id;         //!< Gamepad id
    uint32_t button;            //!< Button state
};

//! Gamepad device event
struct ct_gamepad_device_event {
    uint8_t gamepad_id;         //!< Gamepad id
};

enum {
    EVENT_GAMEPAD_NVALID = 0,   //!< Invalid type

    EVENT_GAMEPAD_MOVE,       //!< Gamepad move
    EVENT_GAMEPAD_UP,         //!< Gamepad button up
    EVENT_GAMEPAD_DOWN,       //!< Gamepad button down
    EVENT_GAMEPAD_CONNECT,    //!< Gamepad connected
    EVENT_GAMEPAD_DISCONNECT, //!< Gamepad disconected
};

//! Gampead id enum
enum {
    GAMEPAD_UNKNOWN = 0,     //!< Invalid gamepad
    GAMEPAD_1 = 1,           //!< Gamepad 1
    GAMEPAD_2 = 2,           //!< Gamepad 2
    GAMEPAD_3 = 3,           //!< Gamepad 3
    GAMEPAD_4 = 4,           //!< Gamepad 4
    GAMEPAD_MAX = GAMEPAD_4, //!< Max gamepads
};

//! Gamepad button enum
enum {
    GAMEPAD_BTN_INVALID = 0,    //!< Invalid button
    GAMEPAD_BTN_A,              //!< Button A
    GAMEPAD_BTN_B,              //!< Button B
    GAMEPAD_BTN_X,              //!< Button X
    GAMEPAD_BTN_Y,              //!< Button Y
    GAMEPAD_BTN_BACK,           //!< Back
    GAMEPAD_BTN_GUIDE,          //!< Guide
    GAMEPAD_BTN_START,          //!< Start
    GAMEPAD_BTN_LEFTSTICK,      //!< Left stick
    GAMEPAD_BTN_RIGHTSTICK,     //!< Right stick
    GAMEPAD_BTN_LEFTSHOULDER,   //!< Left shoulder
    GAMEPAD_BTN_RIGHTSHOULDER,  //!< Right shoulder
    GAMEPAD_BTN_DPADUP,         //!< Dpad up
    GAMEPAD_BTN_DPADDOWN,       //!< Dpad down
    GAMEPAD_BTN_DPADLEFT,       //!< Dpad left
    GAMEPAD_BTN_DPADRIGHT,      //!< Dpad right
    GAMEPAD_BTN_MAX             //!< Max buttons
};

//! Gamepad axis type enum
enum {
    GAMEPAD_AXIS_INVALID = 0, //!< Invalid axis
    GAMEPAD_AXIS_LEFT = 1,    //!< Left axis
    GAMEPAD_AXIS_RIGHT = 2,   //!< Right axis
    GAMEPAD_AXIS_TRIGER = 3,  //!< Triger axis
    GAMEPAD_AXIX_MAX          //!< Max axis
};

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
                 const uint32_t axis_index,
                 float *value);

    //! Play rumble
    //! \param gamepad Gamepad
    //! \param strength Rumble strength
    //! \param length Rumble length
    void (*play_rumble)(uint32_t idx,
                        float strength,
                        uint32_t length);
};

CT_MODULE(ct_gamepad_a0);

#endif //CETECH_GAMEPAD_H
//! \}