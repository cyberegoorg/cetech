

#include <cetech/kernel/module.h>
#include <cetech/modules/luasys.h>

#include <cetech/modules/resource.h>

#include <cetech/kernel/api_system.h>

#define API_NAME "ResourceManager"

CETECH_DECL_API(resource_api_v0);

static int _reload_all(lua_State *l) {

    resource_api_v0.reload_all();
    return 0;
}

#ifdef CETECH_CAN_COMPILE

static int _compile_all(lua_State *l) {
    resource_api_v0.compiler_compile_all();
    return 0;
}

#endif

void _register_lua_resource_manager_api(struct api_v0 *api) {
    CETECH_GET_API(api, resource_api_v0);

    luasys_add_module_function(API_NAME, "reload_all", _reload_all);

#ifdef CETECH_CAN_COMPILE
    luasys_add_module_function(API_NAME, "compile_all", _compile_all);
#endif

}