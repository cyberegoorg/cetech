#ifndef CETECH_ASSET_BROWSER_H
#define CETECH_ASSET_BROWSER_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

//==============================================================================
// Typedefs
//==============================================================================
typedef void (*ct_ab_on_asset_click)(uint64_t type, uint64_t name, uint64_t root, const char* path);
typedef void (*ct_ab_on_asset_double_click)(uint64_t type, uint64_t name, uint64_t root, const char* path);

//==============================================================================
// Api
//==============================================================================

struct ct_asset_browser_a0 {
    void (*register_on_asset_click)(ct_ab_on_asset_click fce);
    void (*unregister_on_asset_click)(ct_ab_on_asset_click fce);

    void (*register_on_asset_double_click)(ct_ab_on_asset_double_click fce);
    void (*unregister_on_asset_double_click)(ct_ab_on_asset_double_click fce);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_ASSET_BROWSER_H
