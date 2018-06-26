#ifndef CETECH_ASSET_BROWSER_H
#define CETECH_ASSET_BROWSER_H



//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

struct ct_resource_id;

//==============================================================================
// Typedefs
//==============================================================================

struct ct_asset_browser_click_ev {
    uint64_t asset;
    uint64_t root;
    const char *path;
};

#define ASSET_BROWSER_EBUS_NAME "asset_browser"

enum {
    ASSET_BROWSER_EBUS = 0x271fc190
};

enum {
    ASSET_INAVLID_EVENT = 0,
    ASSET_CLICK_EVENT,
    ASSET_DCLICK_EVENT,
};

//==============================================================================
// Api
//==============================================================================

struct ct_asset_browser_a0 {
    uint64_t (*get_selected_asset_type)();

    void (*get_selected_asset_name)(char *asset_name);
};

CT_MODULE(ct_asset_browser_a0);

#endif //CETECH_ASSET_BROWSER_H
