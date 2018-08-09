#ifndef CETECH_ASSET_PREVIEW_H
#define CETECH_ASSET_PREVIEW_H

#include <cetech/resource/resource.h>
#include <stddef.h>

struct ct_world;

#define ASSET_PREVIEW \
    CE_ID64_0("ct_asset_preview_i0", 0xb3cba443e3ad0e60ULL)

struct ct_asset_preview_i0 {
    struct ct_entity (*load)(struct ct_resource_id resourceid,
                             struct ct_world world);

    void (*unload)(struct ct_resource_id resourceid,
                   struct ct_world world,
                   struct ct_entity entity);

    void (*tooltip)(struct ct_resource_id resourceid);
};


struct ct_asset_preview_a0 {
};

CE_MODULE(ct_asset_preview_a0);


#endif //CETECH_ASSET_PREVIEW_H
