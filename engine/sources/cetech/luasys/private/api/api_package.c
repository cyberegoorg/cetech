#include <stddef.h>
#include <cetech/memory/allocator.h>

#include <cetech/renderer/renderer.h>
#include <cetech/resource/resource.h>
#include <cetech/application/private/module.h>
#include "../luasys.h"

#define API_NAME "Package"

IMPORT_API(package_api_v0);

static int _load(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));

    package_api_v0.load(package_name);
    return 0;
}

static int _unload(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));


    package_api_v0.unload(package_name);
    return 0;
}

static int _is_loaded(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));

    int is_loaded = package_api_v0.is_loaded(package_name);

    luasys_push_bool(l, is_loaded);

    return 1;
}

static int _flush(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));

    package_api_v0.flush(package_name);

    return 0;
}


void _register_lua_package_api(get_api_fce_t get_engine_api) {
    package_api_v0 = *(struct package_api_v0 *) get_engine_api(PACKAGE_API_ID);

    luasys_add_module_function(API_NAME, "load", _load);
    luasys_add_module_function(API_NAME, "unload", _unload);
    luasys_add_module_function(API_NAME, "is_loaded", _is_loaded);
    luasys_add_module_function(API_NAME, "flush", _flush);
}