#ifndef CETECH_ASSET_BROWSER_H
#define CETECH_ASSET_BROWSER_H

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

//==============================================================================
// Typedefs
//==============================================================================

#define ASSET_BROWSER_EBUS_NAME "asset_browser"

#define ASSET_BROWSER_ASSET_TYPE \
    CE_ID64_0("asset", 0xc6ed95f9933c4734ULL)

// TODO: RENAME
#define ASSET_BROWSER_ASSET_TYPE2 \
    CE_ID64_0("asset_type", 0x1f1f05db4e4dabbaULL)

#define ASSET_BROWSER_ASSET_NAME \
    CE_ID64_0("asset_name", 0xf82d0a5475e3d5eaULL)

#define ASSET_BROWSER_PATH \
    CE_ID64_0("path", 0xae70259f6415b584ULL)

#define ASSET_BROWSER_ROOT \
    CE_ID64_0("root", 0xbccf91e5b6d543e9ULL)

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
