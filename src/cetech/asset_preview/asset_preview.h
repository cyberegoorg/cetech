#ifndef CETECH_ASSET_PREVIEW_H
#define CETECH_ASSET_PREVIEW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cetech/asset/asset.h>
#include <stddef.h>

typedef struct ct_world_t0 ct_world_t0;
typedef struct ct_entity_t0 ct_entity_t0;

#define CT_ASSET_PREVIEW_A0_STR "ct_asset_preview_a0"

#define CT_ASSET_PREVIEW_API \
    CE_ID64_0("ct_asset_preview_a0", 0x28baa6853c1c1a01ULL)

#define CT_PREVIEW_ASPECT \
    CE_ID64_0("preview_aspect", 0x3bd694cda186a250ULL)

typedef struct ct_asset_preview_i0 {
    void (*draw_raw)(uint64_t obj,
                     ce_vec2_t size);

    ct_entity_t0 (*load)(uint64_t asset,
                         ct_world_t0 world);

    void (*unload)(uint64_t asset,
                   ct_world_t0 world,
                   ct_entity_t0 entity);

    void (*tooltip)(uint64_t asset,
                    ce_vec2_t size);
} ct_asset_preview_i0;


struct ct_asset_preview_a0 {
    void (*asset_tooltip)(ce_cdb_uuid_t0 assetid,
                          const char *path,
                          ce_vec2_t size);
};

CE_MODULE(ct_asset_preview_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_ASSET_PREVIEW_H
