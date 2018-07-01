#ifndef CETECH_PLAYGROUND_H
#define CETECH_PLAYGROUND_H



//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>

#define PLAYGROUND_EBUS_NAME "playground"

enum {
    PLAYGROUND_EBUS = 0x7f3091f7
};


enum {
    PLAYGROUND_INAVLID_EVENT = 0,
    PLAYGROUND_INIT_EVENT,
    PLAYGROUND_SHUTDOWN_EVENT,
    PLAYGROUND_UPDATE_EVENT,
    PLAYGROUND_RENDER_EVENT,
};

struct ct_dock_i {
    uint64_t id;
    bool visible;

    const char* (*title)(struct ct_dock_i* dock);

    void (*draw_ui)(struct ct_dock_i* dock);
    void (*draw_main_menu)();
};

//==============================================================================
// Api
//==============================================================================

//! Playground API V0
struct ct_playground_a0 {
    void (*reload_layout)();
};

CT_MODULE(ct_playground_a0);

#endif //CETECH_PLAYGROUND_H
