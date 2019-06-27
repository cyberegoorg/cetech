#ifndef CETECH_PLAYGROUND_H
#define CETECH_PLAYGROUND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define CT_EDITOR_MODULE_I \
    CE_ID64_0("ct_editor_module_i0", 0x761dbf8cf91061a1ULL)

#define CT_EDITOR_TASK \
    CE_ID64_0("editor_task", 0x5eb9f56061f23532ULL)

typedef struct ct_editor_module_i0 {
    bool (*init)();

    bool (*shutdown)();

    void (*draw_menu)();

    void (*update)(float dt);

} ct_editor_module_i0;

#ifdef __cplusplus
};
#endif

#endif //CETECH_PLAYGROUND_H
