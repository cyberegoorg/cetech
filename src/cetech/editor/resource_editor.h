#ifndef CETECH_EDITOR_H
#define CETECH_EDITOR_H

#include <stdint.h>

#define RESOURCE_EDITOR_I \
    CE_ID64_0("ct_resource_editor_i0", 0x1271656210cd7528ULL)

#define RESOURCE_EDITOR_OBJ \
    CE_ID64_0("asset_obj", 0x755d033f781be94ULL)

struct ct_resource_editor_i0 {
    uint64_t (*cdb_type)();

    const char* (*display_icon)();
    const char* (*display_name)();

    void (*open)(uint64_t context_obj);
    void (*close)(uint64_t context_obj);
    void (*update)(uint64_t context_obj, float dt);
    void (*draw_ui)(uint64_t context_obj);
    void (*draw_menu)(uint64_t context_obj);
};

#endif //CETECH_EDITOR_H
