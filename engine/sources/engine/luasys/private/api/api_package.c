
#include <engine/renderer/mesh_renderer.h>
#include <engine/resource/resource.h>
#include <engine/plugin/plugin.h>
#include "engine/luasys/luasys.h"

#define API_NAME "Package"

/*
void package_load(stringid64_t name);
void package_unload(stringid64_t name);
int package_is_loaded(stringid64_t name);
void package_flush(stringid64_t name);
*/

static int _load(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));

    struct PackageApiV1 PackageApiV1 = *(struct PackageApiV1*) plugin_get_engine_api(PACKAGE_API_ID, 0);

    PackageApiV1.load(package_name);
    return 0;
}

static int _unload(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));

    struct PackageApiV1 PackageApiV1 = *(struct PackageApiV1*) plugin_get_engine_api(PACKAGE_API_ID, 0);

    PackageApiV1.unload(package_name);
    return 0;
}

static int _is_loaded(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));

    struct PackageApiV1 PackageApiV1 = *(struct PackageApiV1*) plugin_get_engine_api(PACKAGE_API_ID, 0);

    int is_loaded = PackageApiV1.is_loaded(package_name);

    luasys_push_bool(l, is_loaded);

    return 1;
}

static int _flush(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));

    struct PackageApiV1 PackageApiV1 = *(struct PackageApiV1*) plugin_get_engine_api(PACKAGE_API_ID, 0);

    PackageApiV1.flush(package_name);

    return 0;
}


void _register_lua_package_api() {
    luasys_add_module_function(API_NAME, "load", _load);
    luasys_add_module_function(API_NAME, "unload", _unload);
    luasys_add_module_function(API_NAME, "is_loaded", _is_loaded);
    luasys_add_module_function(API_NAME, "flush", _flush);
}