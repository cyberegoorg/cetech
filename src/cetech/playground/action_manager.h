#ifndef CETECH_ACTION_MANAGER_H
#define CETECH_ACTION_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <stddef.h>

//==============================================================================
// Api
//==============================================================================

struct ct_action_manager_a0 {
    void (*register_action)(uint64_t name, const char* shortcut, void (*fce)());
    void (*unregister_action)(uint64_t name);

    const char* (*shortcut_str)(uint64_t name);
    void (*execute)(uint64_t name);
    void (*check)();

};

#ifdef __cplusplus
}
#endif

#endif //CETECH_ACTION_MANAGER_H
