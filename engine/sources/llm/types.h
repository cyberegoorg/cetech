#ifndef CETECH_MACHINE_TYPES_H
#define CETECH_MACHINE_TYPES_H

#include <stdio.h>

#include "celib/types.h"
#include "celib/math/types.h"
#include "celib/containers/eventstream.h"

//==============================================================================
// Base events
//==============================================================================

enum event {
    EVENT_KEYBOARD_UP = 1,
    EVENT_KEYBOARD_DOWN = 2,

    EVENT_MOUSE_MOVE = 3,
    EVENT_MOUSE_UP = 4,
    EVENT_MOUSE_DOWN = 5
};

//==============================================================================
// Mouse
//==============================================================================


typedef struct {
    void *w;
} window_t;

enum WindowFlags {
    WINDOW_NOFLAG = 0,
    WINDOW_FULLSCREEN = 1,
};

enum WindowPos {
    WINDOWPOS_CENTERED = 1,
    WINDOWPOS_UNDEFINED = 2
};

#endif //CETECH_MACHINE_TYPES_H
