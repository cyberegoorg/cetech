#ifndef CETECH_GAMEPAD_H
#define CETECH_GAMEPAD_H


#include <stdint.h>

#define GAMEPAD_EBUS_NAME "gamepad"

#define CONTROLER_GAMEPAD \
    CE_ID64_0("gamepad", 0x6aa699de487f71eeULL)

enum {
    GAMEPAD_EBUS = 0x3913b95f
};

enum {
    EVENT_GAMEPAD_NVALID = 0,   //!< Invalid type

    EVENT_GAMEPAD_MOVE,       //!< Gamepad move
    EVENT_GAMEPAD_UP,         //!< Gamepad button up
    EVENT_GAMEPAD_DOWN,       //!< Gamepad button down
    EVENT_GAMEPAD_CONNECT,    //!< Gamepad connected
    EVENT_GAMEPAD_DISCONNECT, //!< Gamepad disconected
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

#endif //CETECH_GAMEPAD_H
//! \}