#ifndef CETECH_ASSET_PREVIEW_H
#define CETECH_ASSET_PREVIEW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cetech/resource/resource.h>
#include <stddef.h>

typedef struct ct_world_t0 ct_world_t0;
typedef struct ct_entity_t0 ct_entity_t0;

#define CT_ASSET_PREVIEW_API \
    CE_ID64_0("ct_resource_preview_a0", 0x8498c030ac811f8eULL)

#define RESOURCE_PREVIEW_I \
    CE_ID64_0("ct_resource_preview_i0", 0x5e8a61eef5a43b61ULL)

typedef struct ct_resource_preview_i0 {
    void (*draw_raw)(uint64_t obj,
                     ce_vec2_t size);

    struct ct_entity_t0 (*load)(uint64_t resource,
                                ct_world_t0 world);

    void (*unload)(uint64_t resource,
                   ct_world_t0 world,
                   ct_entity_t0 entity);

    void (*tooltip)(uint64_t resource,
                    ce_vec2_t size);
}ct_resource_preview_i0;


struct ct_resource_preview_a0 {
    void (*set_background_resource)(struct ct_resource_id_t0 resource);

    void (*draw_background_texture)(ce_vec2_t size);
};

CE_MODULE(ct_resource_preview_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_ASSET_PREVIEW_H
