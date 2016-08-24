#ifndef CETECH_MACHINE_TYPES_H
#define CETECH_MACHINE_TYPES_H

#include <stdio.h>

#include "celib/types.h"
#include "celib/math/types.h"
#include "celib/containers/eventstream.h"

//==============================================================================
// Enums
//==============================================================================

enum open_mode {
    VFILE_OPEN_READ,
    VFILE_OPEN_WRITE,
};

enum vfile_seek {
    VFILE_SEEK_SET,
    VFILE_SEEK_CUR,
    VFILE_SEEK_END
};


//==============================================================================
// Structs
//==============================================================================

struct vfile {
    i64 (*size)(struct vfile *vfile);

    i64 (*seek)(struct vfile *vfile, i64 offset, enum vfile_seek whence);

    size_t (*read)(struct vfile *vfile, void *ptr, size_t size, size_t maxnum);

    size_t (*write)(struct vfile *vfile, const void *ptr, size_t size, size_t num);

    int (*close)(struct vfile *vfile);
};

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
// Keyboard
//==============================================================================

struct keyboard_event {
    struct event_header h;
    u32 keycode;
};

#include "_key.h"

//==============================================================================
// Mouse
//==============================================================================


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

typedef int (*thread_fce_t)(void *data);

typedef struct {
    void *t;
} thread_t;

typedef struct {
    int lock;
} spinlock_t;

#endif //CETECH_MACHINE_TYPES_H
