#ifndef CELIB_MACHINE_H
#define CELIB_MACHINE_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/math/types.h"
#include "celib/thread/types.h"
#include "celib/containers/eventstream.h"
#include "engine/plugin/plugin.h"

//==============================================================================
// Enums
//==============================================================================

//! Machine event type enum
enum event {
    EVENT_INVALID = 0,   //!< Invalid type

    EVENT_KEYBOARD_UP,   //!< Keyboard button up
    EVENT_KEYBOARD_DOWN, //!< Keyboard button down

    EVENT_MOUSE_MOVE,    //!< Mouse move
    EVENT_MOUSE_UP,      //!< Mouse button up
    EVENT_MOUSE_DOWN,    //!< Mouse button down

    EVENT_GAMEPAD_MOVE,       //!< Gamepad move
    EVENT_GAMEPAD_UP,         //!< Gamepad button up
    EVENT_GAMEPAD_DOWN,       //!< Gamepad button down
    EVENT_GAMEPAD_CONNECT,    //!< Gamepad connected
    EVENT_GAMEPAD_DISCONNECT, //!< Gamepad disconected
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

    MOUSE_AXIS_MAX = 8 //!< Max axis
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


#include "_key.h"

//==============================================================================
// Structs
//==============================================================================

//! Mouse button status
struct mouse_event {
    struct event_header h; //!< Event header
    u32 button;            //!< Button state
};

struct mouse_move_event {
    struct event_header h; //!< Event header
    cel_vec2f_t pos;       //!< Actual position
};

//! Keyboard event
struct keyboard_event {
    struct event_header h; //!< Event header
    u32 keycode; //!< Key code
};


//! Gamepad move event
struct gamepad_move_event {
    struct event_header h; //!< Event header
    u8 gamepad_id;         //!< Gamepad id
    u32 axis;              //!< Axis id
    cel_vec2f_t position;  //!< Position
};

//! Gamepad button event
struct gamepad_btn_event {
    struct event_header h; //!< Event header
    u8 gamepad_id;         //!< Gamepad id
    u32 button;            //!< Button state
};

//! Gamepad device event
struct gamepad_device_event {
    struct event_header h; //!< Event header
    u8 gamepad_id;         //!< Gamepad id
};

//==============================================================================
// Api
//==============================================================================

//! Machine API V0
struct MachineApiV0 {

    //! Get eventstream begin
    //! \return Begin
    struct event_header *(*event_begin)();

    //! Get eventstream end
    //! \return End
    struct event_header *(*event_end)();

    //! Next event
    //! \param header Actual event header
    //! \return Next event header
    struct event_header *(*event_next)(struct event_header *header);

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
                                u32 length);
};

#endif //CELIB_MACHINE_H
