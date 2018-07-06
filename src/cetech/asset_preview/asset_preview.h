#ifndef CETECH_ASSET_PREVIEW_H
#define CETECH_ASSET_PREVIEW_H

#include <cetech/resource/resource.h>



//==============================================================================
// Includes
//==============================================================================
#include <stddef.h>

struct ct_world;

struct ct_asset_preview_fce {
    void (*load)(const char *filename,
                 struct ct_resource_id resourceid,
                 struct ct_world world);

    void (*unload)(const char *filename,
                   struct ct_resource_id resourceid,
                   struct ct_world world);
};


#define ASSET_PREVIEW CT_ID64_0("ct_asset_preview_i0")

struct ct_asset_preview_i0 {
    struct ct_entity (*load)(struct ct_resource_id resourceid,
                             struct ct_world world);

    void (*unload)(struct ct_resource_id resourceid,
                   struct ct_world world,
                   struct ct_entity entity);
};


//==============================================================================
// Api
//==============================================================================

struct ct_asset_preview_a0 {
};

CT_MODULE(ct_asset_preview_a0);


#endif //CETECH_ASSET_PREVIEW_H
