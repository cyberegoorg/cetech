

#include "cetech/modules/renderer.h"

#include <cetech/core/module.h>
#include <cetech/modules/luasys.h>
#include <cetech/core/hash.h>
#include <cetech/core/api.h>
#include <cetech/modules/package.h>

#define API_NAME "Package"

IMPORT_API(package_api_v0);
IMPORT_API(hash_api_v0);

static int _load(lua_State *l) {
    uint64_t package_name = hash_api_v0.id64_from_str(luasys_to_string(l, 1));

    package_api_v0.load(package_name);
    return 0;
}

static int _unload(lua_State *l) {
    uint64_t package_name = hash_api_v0.id64_from_str(luasys_to_string(l, 1));


    package_api_v0.unload(package_name);
    return 0;
}

static int _is_loaded(lua_State *l) {
    uint64_t package_name = hash_api_v0.id64_from_str(luasys_to_string(l, 1));

    int is_loaded = package_api_v0.is_loaded(package_name);

    luasys_push_bool(l, is_loaded);

    return 1;
}

static int _flush(lua_State *l) {
    uint64_t package_name = hash_api_v0.id64_from_str(luasys_to_string(l, 1));

    package_api_v0.flush(package_name);

    return 0;
}


void _register_lua_package_api(struct api_v0 *api) {
    GET_API(api, package_api_v0);
    GET_API(api, hash_api_v0);

    luasys_add_module_function(API_NAME, "load", _load);
    luasys_add_module_function(API_NAME, "unload", _unload);
    luasys_add_module_function(API_NAME, "is_loaded", _is_loaded);
    luasys_add_module_function(API_NAME, "flush", _flush);
}