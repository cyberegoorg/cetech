

#include <cetech/kernel/module.h>
#include <cetech/modules/luasys.h>

#include <cetech/modules/resource.h>

#include <cetech/kernel/api_system.h>
#include "../luasys_private.h"

#define API_NAME "ResourceManager"

CETECH_DECL_API(ct_resource_a0);

static int _reload_all(lua_State *l) {

    ct_resource_a0.reload_all();
    return 0;
}

#ifdef CETECH_CAN_COMPILE

static int _compile_all(lua_State *l) {
    ct_resource_a0.compiler_compile_all();
    return 0;
}

#endif

void _register_lua_resource_manager_api(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_resource_a0);

    luasys_add_module_function(API_NAME, "reload_all", _reload_all);

#ifdef CETECH_CAN_COMPILE
    luasys_add_module_function(API_NAME, "compile_all", _compile_all);
#endif

}