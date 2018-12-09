#ifndef CETECH_EDITOR_H
#define CETECH_EDITOR_H

#include <stdint.h>

#define _ASSET_TYPE \
    CE_ID64_0("asset_type", 0x1f1f05db4e4dabbaULL)

#define _ASSET_NAME \
    CE_ID64_0("asset_name", 0xf82d0a5475e3d5eaULL)

#define RESOURCE_EDITOR_I \
    CE_ID64_0("ct_resource_editor_i0", 0x1271656210cd7528ULL)

#define RESOURCE_EDITOR_OBJ \
    CE_ID64_0("asset_obj", 0x755d033f781be94ULL)

#define RESOURCE_EDITOR_EBUS \
    CE_ID64_0("asset_editor", 0xd7ecca607e454ce9ULL)

struct ct_resource_editor_i0 {
    uint64_t (*cdb_type)();

    const char* (*display_icon)();
    const char* (*display_name)();

    void (*open)(uint64_t context_obj);
    void (*close)(uint64_t context_obj);
    void (*update)(uint64_t context_obj, float dt);
    void (*draw_ui)(uint64_t context_obj);
};

#endif //CETECH_EDITOR_H
