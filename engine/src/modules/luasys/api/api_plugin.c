

#include <cetech/kernel/module.h>
#include <cetech/modules/luasys.h>
#include <cetech/kernel/api.h>

#define API_NAME "Module"

CETECH_DECL_API(module_api_v0);

static int _reload_all(lua_State *l) {
    module_api_v0.module_reload_all();
    return 0;
}

static int _reload(lua_State *l) {
    const char *path = luasys_to_string(l, 1);
    module_api_v0.module_reload(path);
    return 0;
}


void _register_lua_module_api(struct api_v0 *api) {
    CETECH_GET_API(api, module_api_v0);

    luasys_add_module_function(API_NAME, "reload_all", _reload_all);
    luasys_add_module_function(API_NAME, "_reload", _reload);
}