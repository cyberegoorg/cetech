
#include <cetech/kernel/application.h>
#include <cetech/kernel/module.h>
#include <cetech/modules/luasys.h>
#include <cetech/kernel/api_system.h>


#define API_NAME "Application"


CETECH_DECL_API(app_api_v0);

static int _application_quit(lua_State *l) {
    app_api_v0.quit();
    return 0;
}

static int _application_native_platform(lua_State *l) {
    const char *platform = app_api_v0.native_platform();

    luasys_push_string(l, platform);
    return 1;
}

static int _application_platform(lua_State *l) {
    const char *platform = app_api_v0.platform();

    luasys_push_string(l, platform);
    return 1;
}


void _register_lua_application_api(struct api_v0 *api) {
    CETECH_GET_API(api, app_api_v0);

    luasys_add_module_function(API_NAME, "quit", _application_quit);
    luasys_add_module_function(API_NAME, "get_native_platform",
                               _application_native_platform);
    luasys_add_module_function(API_NAME, "get_platform", _application_platform);
}