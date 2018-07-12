#ifndef CETECH_EDITOR_H
#define CETECH_EDITOR_H

#include <stdint.h>


#define _ASSET_TYPE \
    CT_ID64_0("asset_type", 0x1f1f05db4e4dabbaULL)

#define _ASSET_NAME \
    CT_ID64_0("asset_name", 0xf82d0a5475e3d5eaULL)


struct ct_asset_editor_i0 {
    uint64_t (*asset_type)();
    void (*open)(uint64_t context_obj);
    void (*close)(uint64_t context_obj);
    void (*update)(uint64_t context_obj, float dt);
    void (*render)(uint64_t context_obj);
    void (*draw)(uint64_t context_obj);
};

#endif //CETECH_EDITOR_H
