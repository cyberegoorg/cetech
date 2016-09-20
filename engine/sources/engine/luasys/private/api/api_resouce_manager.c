#include "engine/core/resource_manager.h"
#include "engine/luasys/lua_system.h"

#define API_NAME "ResourceManager"

static int _reload_all(lua_State *l) {
    resource_reload_all();
    return 0;
}

void _register_lua_resource_manager_api() {
    luasys_add_module_function(API_NAME, "reload_all", _reload_all);
}