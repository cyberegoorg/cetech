#ifndef CETECH_ASSET_PREVIEW_H
#define CETECH_ASSET_PREVIEW_H

#include <cetech/engine/resource/resource.h>

#ifdef __cplusplus
extern "C" {
#endif

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

//==============================================================================
// Api
//==============================================================================

struct ct_asset_preview_a0 {
    void (*register_type_preview)(const char* type,
                                  struct ct_asset_preview_fce fce);

    void (*unregister_type_preview)(const char* type);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_ASSET_PREVIEW_H
