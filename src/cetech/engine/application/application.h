#ifndef CETECH_APPLICATION_API_H
#define CETECH_APPLICATION_API_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Include
//==============================================================================

#include <stdint.h>

struct ct_window;

//==============================================================================
// Struct
//==============================================================================

#define APPLICATION_EBUS_NAME "application"

enum {
    APPLICATION_EBUS = 0x6a0c4eb6
};

enum {
    APP_INVALID_EVNT = 0,
    APP_INI_EVENT,
    APP_UPDATE_EVENT,
    APP_SHUTDOWN_EVENT,
    APP_QUIT_EVENT,

    APP_GAME_INIT_EVENT,
    APP_GAME_UPDATE_EVENT,
    APP_GAME_SHUTDOWN_EVENT,
};

struct ct_app_update_ev {
    float dt;
};

//==============================================================================
// Api
//==============================================================================

//! Application API V0
struct ct_app_a0 {
    void (*quit)();

    void (*start)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_APPLICATION_H
// \}