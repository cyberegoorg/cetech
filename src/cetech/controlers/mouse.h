#ifndef CETECH_MOUSE_H
#define CETECH_MOUSE_H



//==============================================================================
// Includes
//==============================================================================
#include <stdint.h>
#include <corelib/module.inl>

#define MOUSE_EBUS_NAME "mouse"

enum {
    MOUSE_EBUS = 0x3ddd354d
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

#endif //CETECH_MOUSE_H
//! \}