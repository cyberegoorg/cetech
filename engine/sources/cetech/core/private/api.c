//==============================================================================
// Includes
//==============================================================================

#include <cetech/core/os/path.h>
#include <cetech/core/hash.h>
#include <cetech/core/module.h>
#include <cetech/core/container/map.inl>
#include <cetech/core/api.h>

#include <cetech/core/config.h>
#include <cetech/core/os/private/hash.inl>


//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "api_system"

//==============================================================================
// Globals
//==============================================================================

#define _G ApiSystemGlobals

static struct G {
    MAP_T(void) api_map;
} _G = {0};

IMPORT_API(hash_api_v0)

//==============================================================================
// Private
//==============================================================================

void _register_api(const char *name,
                   void *api) {
    uint64_t name_id = stringid64_from_string(name);

    MAP_SET(void, &_G.api_map, name_id, api);
}

void *_first(const char *name) {
    uint64_t name_id = stringid64_from_string(name);

    void *api = MAP_GET(void, &_G.api_map, name_id, NULL);

    return api;
}

static struct api_v0 api_v0 = {
        .register_api = _register_api,
        .first = _first
};

void api_init(struct allocator *allocator) {
    MAP_INIT(void, &_G.api_map, allocator);

    _register_api("api_v0", &api_v0);
}

void api_shutdown() {
    MAP_DESTROY(void, &_G.api_map);
}

struct api_v0 *api_get_v0() {
    return &api_v0;
}