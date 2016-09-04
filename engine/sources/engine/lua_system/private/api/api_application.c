#include <engine/application/application.h>
#include "engine/lua_system/lua_system.h"

#define API_NAME "Application"

static int _application_quit(lua_State *l) {
    application_quit();
    return 0;
}

void _register_lua_application_api() {
    luasys_add_module_function(API_NAME, "quit", _application_quit);
}