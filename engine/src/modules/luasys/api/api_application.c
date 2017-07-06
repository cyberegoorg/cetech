
#include <cetech/modules/application.h>
#include <cetech/kernel/module.h>
#include <cetech/modules/luasys.h>
#include <cetech/kernel/api_system.h>
#include "../luasys_private.h"


#define API_NAME "Application"


CETECH_DECL_API(ct_app_a0);

static int _application_quit(lua_State *l) {
    ct_app_a0.quit();
    return 0;
}

static int _application_native_platform(lua_State *l) {
    const char *platform = ct_app_a0.native_platform();

    luasys_push_string(l, platform);
    return 1;
}

static int _application_platform(lua_State *l) {
    const char *platform = ct_app_a0.platform();

    luasys_push_string(l, platform);
    return 1;
}


void _register_lua_application_api(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_app_a0);

    luasys_add_module_function(API_NAME, "quit", _application_quit);
    luasys_add_module_function(API_NAME, "get_native_platform",
                               _application_native_platform);
    luasys_add_module_function(API_NAME, "get_platform", _application_platform);
}