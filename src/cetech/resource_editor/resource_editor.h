#ifndef CETECH_EDITOR_H
#define CETECH_EDITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CT_RESOURCE_EDITOR_I0_STR "ct_resource_editor_i0"

#define CT_RESOURCE_EDITOR_I0 \
    CE_ID64_0("ct_resource_editor_i0", 0x1271656210cd7528ULL)

#define CT_RESOURCE_EDITOR_CONTEXT \
    CE_ID64_0("ct_resource_editor_context", 0x9d16b1cf38ff0c4cULL)

typedef struct ct_resource_editor_i0 {
    uint64_t (*cdb_type)();

    const char *(*display_icon)();

    const char *(*display_name)();

    uint64_t (*open)(uint64_t obj);

    void (*close)(uint64_t context_obj);

    void (*update)(uint64_t context_obj,
                   float dt);

    void (*draw_ui)(uint64_t context_obj,
                    uint64_t context);

    void (*draw_menu)(uint64_t context_obj);
} ct_resource_editor_i0;

#ifdef __cplusplus
};
#endif

#endif //CETECH_EDITOR_H
