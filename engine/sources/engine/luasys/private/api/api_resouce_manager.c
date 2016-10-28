#include "engine/resource/resource.h"
#include "engine/luasys/luasys.h"

#define API_NAME "ResourceManager"

static int _reload_all(lua_State *l) {
    resource_reload_all();
    return 0;
}

static int _compile_all(lua_State *l) {
    resource_compiler_compile_all();
    return 0;
}


void _register_lua_resource_manager_api() {
    luasys_add_module_function(API_NAME, "reload_all", _reload_all);
    luasys_add_module_function(API_NAME, "compile_all", _compile_all);
}