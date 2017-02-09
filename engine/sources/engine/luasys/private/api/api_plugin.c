#include <engine/plugin/plugin.h>
#include "engine/luasys/luasys.h"

#define API_NAME "Plugin"

static int _reload_all(lua_State *l) {
    plugin_reload_all();
    return 0;
}

static int _reload(lua_State *l) {
    const char* path = luasys_to_string(l, 1);
    plugin_reload(path);
    return 0;
}


void _register_lua_plugin_api() {
    luasys_add_module_function(API_NAME, "reload_all", _reload_all);
    luasys_add_module_function(API_NAME, "_reload", _reload);
}