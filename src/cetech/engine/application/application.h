//! \defgroup Application
//! Main application
//! \{
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


struct ct_game_fce {
    void (*on_init)();

    void (*on_shutdown)();

    void (*on_update)(float dt);

    void (*on_ui)();
};

typedef void (*ct_app_on_init)();
typedef void (*ct_app_on_shutdown)();
typedef void (*ct_app_on_update)(float dt);

//==============================================================================
// Api
//==============================================================================

//! Application API V0
struct ct_app_a0 {
    //! Stop main loop and quit.
    void (*quit)();

    void (*start)();

    void (*register_game)(uint64_t name,
                          struct ct_game_fce game);

    void (*unregister_game)(uint64_t name);

    void (*set_active_game)(uint64_t name);

    void (*register_on_init)(ct_app_on_init on_init);

    void (*unregister_on_init)(ct_app_on_init on_init);

    void (*register_on_shutdown)(ct_app_on_shutdown on_shutdown);

    void (*unregister_on_shutdown)(ct_app_on_shutdown on_shutdown);

    void (*register_on_update)(ct_app_on_update on_update);

    void (*unregister_on_update)(ct_app_on_update on_update);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_APPLICATION_H
// \}