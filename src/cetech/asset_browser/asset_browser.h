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

#define ASSET_BROWSER_ASSET_TYPE \
    CT_ID64_0("asset", 0xc6ed95f9933c4734ULL)

#define ASSET_BROWSER_ASSET \
    CT_ID64_0("asset", 0xc6ed95f9933c4734ULL)

#define ASSET_BROWSER_PATH \
    CT_ID64_0("path", 0xae70259f6415b584ULL)

#define ASSET_BROWSER_ROOT \
    CT_ID64_0("root", 0xbccf91e5b6d543e9ULL)

#define ASSET_BROWSER_SOURCE \
    SOURCE_ROOT


enum {
    ASSET_BROWSER_EBUS = 0x271fc190
};

enum {
    ASSET_INAVLID_EVENT = 0,
    ASSET_DCLICK_EVENT,
};

//==============================================================================
// Api
//==============================================================================

#endif //CETECH_ASSET_BROWSER_H
