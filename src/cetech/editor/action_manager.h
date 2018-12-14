#ifndef CETECH_ACTION_MANAGER_H
#define CETECH_ACTION_MANAGER_H

#include <stdint.h>
#include <stddef.h>

struct ct_action_manager_a0 {
    void (*register_action)(uint64_t name,
                            const char *shortcut,
                            void (*fce)());

    void (*unregister_action)(uint64_t name);

    const char *(*shortcut_str)(uint64_t name);

    void (*execute)(uint64_t name);

    void (*check)();
};

CE_MODULE(ct_action_manager_a0);

#endif //CETECH_ACTION_MANAGER_H
