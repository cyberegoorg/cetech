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
};

struct ct_app_update_ev {
    float dt;
};

struct ct_game_fce {
    void (*on_init)();

    void (*on_shutdown)();

    void (*on_update)(float dt);

    void (*on_ui)();
};

//==============================================================================
// Api
//==============================================================================

//! Application API V0
struct ct_app_a0 {
    void (*quit)();

    void (*start)();

    void (*register_game)(uint64_t name,
                          struct ct_game_fce game);

    void (*unregister_game)(uint64_t name);

    void (*set_active_game)(uint64_t name);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_APPLICATION_H
// \}