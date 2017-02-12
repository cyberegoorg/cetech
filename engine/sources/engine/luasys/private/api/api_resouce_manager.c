#include <engine/plugin/plugin.h>
#include "engine/resource/resource.h"
#include "engine/luasys/luasys.h"
#include <engine/plugin/plugin_api.h>

#define API_NAME "ResourceManager"

struct ResourceApiV1 ResourceApiV1;

static int _reload_all(lua_State *l) {

    ResourceApiV1.reload_all();
    return 0;
}

static int _compile_all(lua_State *l) {
    ResourceApiV1.compiler_compile_all();
    return 0;
}


void _register_lua_resource_manager_api(get_api_fce_t get_engine_api) {
    ResourceApiV1 = *(struct ResourceApiV1 *) get_engine_api(RESOURCE_API_ID, 0);

    luasys_add_module_function(API_NAME, "reload_all", _reload_all);
    luasys_add_module_function(API_NAME, "compile_all", _compile_all);
}