#include <engine/application/private/module.h>
#include "../luasys.h"

#define API_NAME "Plugin"

static int _reload_all(lua_State *l) {
    module_reload_all();
    return 0;
}

static int _reload(lua_State *l) {
    const char *path = luasys_to_string(l, 1);
    module_reload(path);
    return 0;
}


void _register_lua_module_api(get_api_fce_t get_engine_api) {
    luasys_add_module_function(API_NAME, "reload_all", _reload_all);
    luasys_add_module_function(API_NAME, "_reload", _reload);
}