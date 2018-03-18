#ifndef CETECH_PLAYGROUND_H
#define CETECH_PLAYGROUND_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

#define PLAYGROUND_EBUS_NAME "playground"

enum {
    PLAYGROUND_EBUS = 0x7f3091f7
};


enum {
    PLAYGROUND_INAVLID_EVENT = 0,
    PLAYGROUND_INIT_EVENT,
    PLAYGROUND_SHUTDOWN_EVENT,
    PLAYGROUND_UPDATE_EVENT,
    PLAYGROUND_UI_EVENT,
    PLAYGROUND_UI_MAINMENU_EVENT,
};


struct ct_playground_update_ev {
    float dt;
};

//==============================================================================
// Api
//==============================================================================

//! Playground API V0
struct ct_playground_a0 {
    void (*reload_layout)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_PLAYGROUND_H
