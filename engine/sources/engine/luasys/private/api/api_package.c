
#include <engine/renderer/mesh_renderer.h>
#include <engine/resource/resource.h>
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
    package_load(package_name);
    return 0;
}

static int _unload(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));
    package_unload(package_name);
    return 0;
}

static int _is_loaded(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));

    int is_loaded = package_is_loaded(package_name);
    luasys_push_bool(l, is_loaded);

    return 1;
}

static int _flush(lua_State *l) {
    stringid64_t package_name = stringid64_from_string(luasys_to_string(l, 1));
    package_flush(package_name);
    return 0;
}


void _register_lua_package_api() {
    luasys_add_module_function(API_NAME, "load", _load);
    luasys_add_module_function(API_NAME, "unload", _unload);
    luasys_add_module_function(API_NAME, "is_loaded", _is_loaded);
    luasys_add_module_function(API_NAME, "flush", _flush);
}