//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <memory.h>

#include <cetech/core/fs.h>
#include <cetech/core/memory.h>
#include <cetech/core/string.h>
#include <cetech/core/hash.h>
#include <cetech/core/module.h>
#include <cetech/core/map.inl>
#include <cetech/core/api.h>

#include <cetech/kernel/config.h>
#include <cetech/core/hash.h>


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

//==============================================================================
// Private
//==============================================================================

void _register_api(const char* name, void* api) {
    stringid64_t name_id = stringid64_from_string(name);

    MAP_SET(void, &_G.api_map, name_id.id, api);
}

void* _first(const char* name) {
    stringid64_t name_id = stringid64_from_string(name);

    void* api = MAP_GET(void, &_G.api_map, name_id.id, NULL);

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

void api_shutdown(){
    MAP_DESTROY(void, &_G.api_map);
}

struct api_v0* api_get_v0() {
    return &api_v0;
}