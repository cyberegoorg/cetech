#ifndef CETECH_ASSET_PREVIEW_H
#define CETECH_ASSET_PREVIEW_H

#include <cetech/resource/resource.h>
#include <stddef.h>

struct ct_world;

#define CT_ASSET_PREVIEW_API \
    CE_ID64_0("ct_resource_preview_a0", 0x8498c030ac811f8eULL)

#define RESOURCE_PREVIEW_I \
    CE_ID64_0("ct_resource_preview_i0", 0x5e8a61eef5a43b61ULL)

struct ct_resource_preview_i0 {
    struct ct_entity (*load)(uint64_t resource,
                             struct ct_world world);

    void (*unload)(uint64_t resource,
                   struct ct_world world,
                   struct ct_entity entity);

    void (*tooltip)(uint64_t resource, float size[2]);
};


struct ct_resource_preview_a0 {
    void (*set_background_resource)(struct ct_resource_id resource);
    void (*draw_background_texture)(float size[2]);
};

CE_MODULE(ct_resource_preview_a0);


#endif //CETECH_ASSET_PREVIEW_H
