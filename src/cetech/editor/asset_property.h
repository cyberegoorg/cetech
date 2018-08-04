#ifndef CETECH_ASSET_PROPERTY_H
#define CETECH_ASSET_PROPERTY_H

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

struct ct_resource_id;

#define ASSET_PROPERTY \
    CT_ID64_0("ct_asset_property_i0", 0x4f214fca2968dbc1ULL)

struct ct_asset_property_i0 {
    const char* (*display_name)();
    void (*draw)(uint64_t obj);
};

//==============================================================================
// Api
//==============================================================================

struct ct_asset_property_a0 {
};

CT_MODULE(ct_asset_property_a0);

#endif //CETECH_ASSET_PROPERTY_H
