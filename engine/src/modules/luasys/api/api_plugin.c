

#include <cetech/kernel/module.h>
#include <cetech/modules/luasys.h>
#include <cetech/kernel/api_system.h>

#define API_NAME "Module"

CETECH_DECL_API(ct_module_a0);

static int _reload_all(lua_State *l) {
    ct_module_a0.module_reload_all();
    return 0;
}

static int _reload(lua_State *l) {
    const char *path = luasys_to_string(l, 1);
    ct_module_a0.module_reload(path);
    return 0;
}


void _register_lua_module_api(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_module_a0);

    luasys_add_module_function(API_NAME, "reload_all", _reload_all);
    luasys_add_module_function(API_NAME, "_reload", _reload);
}