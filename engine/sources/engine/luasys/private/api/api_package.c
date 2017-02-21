
#include <engine/renderer/mesh_renderer.h>
#include "engine/resource/types.h"
#include <engine/plugin/plugin.h>
#include "../luasys.h"
#include <engine/plugin/plugin_api.h>

#define API_NAME "Package"


struct PackageApiV1 PackageApiV1;

static int _load(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));

    PackageApiV1.load(package_name);
    return 0;
}

static int _unload(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));


    PackageApiV1.unload(package_name);
    return 0;
}

static int _is_loaded(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));

    int is_loaded = PackageApiV1.is_loaded(package_name);

    luasys_push_bool(l, is_loaded);

    return 1;
}

static int _flush(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));

    PackageApiV1.flush(package_name);

    return 0;
}


void _register_lua_package_api(get_api_fce_t get_engine_api) {
    PackageApiV1 = *(struct PackageApiV1 *) get_engine_api(PACKAGE_API_ID, 0);

    luasys_add_module_function(API_NAME, "load", _load);
    luasys_add_module_function(API_NAME, "unload", _unload);
    luasys_add_module_function(API_NAME, "is_loaded", _is_loaded);
    luasys_add_module_function(API_NAME, "flush", _flush);
}