#ifndef CETECH_ACTION_MANAGER_H
#define CETECH_ACTION_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#define CT_ACTION_MANAGER_A0_STR "ct_action_manager_a0"

#define CT_ACTION_MANAGER_API \
    CE_ID64_0("ct_action_manager_a0", 0xcdad2ddf745f6c55ULL)

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

#ifdef __cplusplus
};
#endif

#endif //CETECH_ACTION_MANAGER_H
