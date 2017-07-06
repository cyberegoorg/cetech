

#include "cetech/modules/renderer.h"
#include "../luasys_private.h"

#include <cetech/kernel/module.h>
#include <cetech/modules/luasys.h>
#include <cetech/kernel/hash.h>
#include <cetech/kernel/api_system.h>
#include <cetech/modules/package.h>

#define API_NAME "Package"

CETECH_DECL_API(ct_package_a0);
CETECH_DECL_API(ct_hash_a0);

static int _load(lua_State *l) {
    uint64_t package_name = ct_hash_a0.id64_from_str(luasys_to_string(l, 1));

    ct_package_a0.load(package_name);
    return 0;
}

static int _unload(lua_State *l) {
    uint64_t package_name = ct_hash_a0.id64_from_str(luasys_to_string(l, 1));


    ct_package_a0.unload(package_name);
    return 0;
}

static int _is_loaded(lua_State *l) {
    uint64_t package_name = ct_hash_a0.id64_from_str(luasys_to_string(l, 1));

    int is_loaded = ct_package_a0.is_loaded(package_name);

    luasys_push_bool(l, is_loaded);

    return 1;
}

static int _flush(lua_State *l) {
    uint64_t package_name = ct_hash_a0.id64_from_str(luasys_to_string(l, 1));

    ct_package_a0.flush(package_name);

    return 0;
}


void _register_lua_package_api(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_package_a0);
    CETECH_GET_API(api, ct_hash_a0);

    luasys_add_module_function(API_NAME, "load", _load);
    luasys_add_module_function(API_NAME, "unload", _unload);
    luasys_add_module_function(API_NAME, "is_loaded", _is_loaded);
    luasys_add_module_function(API_NAME, "flush", _flush);
}