#ifndef CETECH_MOUSE_TYPES_H
#define CETECH_MOUSE_TYPES_H

#include <celib/types.h>
#include <celib/containers/eventstream.h>
#include <celib/math/types.h>

struct mouse_event {
    struct event_header h;
    u32 button;
};

struct mouse_move_event {
    struct event_header h;
    vec2f_t pos;
};

enum {
    MOUSE_BTN_UNKNOWN = 0,

    MOUSE_BTN_LEFT = 1,
    MOUSE_BTN_MIDLE = 2,
    MOUSE_BTN_RIGHT = 3,

    MOUSE_BTN_MAX = 8
};

enum {
    MOUSE_AXIS_UNKNOWN = 0,

    MOUSE_AXIS_ABSOULTE = 1,
    MOUSE_AXIS_RELATIVE = 2,

    MOUSE_AXIS_MAX = 8
};

#endif //CETECH_MOUSE_TYPES_H
