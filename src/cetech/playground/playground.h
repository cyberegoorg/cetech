#ifndef CETECH_PLAYGROUND_H
#define CETECH_PLAYGROUND_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

struct ct_playground_module_fce {
    void (*on_init)();

    void (*on_shutdown)();

    void (*on_update)(float dt);

    void (*on_render)();

    void (*on_ui)();

    void (*on_menu_window)();
};

//==============================================================================
// Api
//==============================================================================

//! Playground API V0
struct ct_playground_a0 {
    void (*register_module)(uint64_t name,
                            ct_playground_module_fce game);

    void (*unregister_module)(uint64_t name);

    void (*reload_layout)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_PLAYGROUND_H
