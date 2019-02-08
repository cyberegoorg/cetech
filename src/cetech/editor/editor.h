#ifndef CETECH_PLAYGROUND_H
#define CETECH_PLAYGROUND_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <stdint.h>

#define EDITOR_MODULE_INTERFACE \
    CE_ID64_0("ct_editor_module_i0", 0x761dbf8cf91061a1ULL)

#define CT_EDITOR_TASK \
    CE_ID64_0("editor_task", 0x5eb9f56061f23532ULL)

struct ct_editor_module_i0 {
    bool (*init)();

    bool (*shutdown)();

    void (*update)(float dt);

};

#ifdef __cplusplus
};
#endif

#endif //CETECH_PLAYGROUND_H
