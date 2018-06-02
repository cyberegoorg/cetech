#ifndef CETECH_ASSET_PROPERTY_H
#define CETECH_ASSET_PROPERTY_H




//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

struct ct_resource_id;

//==============================================================================
// Typedefs
//==============================================================================
typedef void (*ct_ap_on_asset)(struct ct_resource_id asset, const char *path);

//==============================================================================
// Api
//==============================================================================

struct ct_asset_property_a0 {
    void (*register_asset)(uint32_t type,
                           ct_ap_on_asset on_asset);
};


#endif //CETECH_ASSET_PROPERTY_H
