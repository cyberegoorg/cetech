#include <stddef.h>
#include "cetech/allocator.h"
#include <cetech/vio.h>

#include <cetech/application/module.h>
#include <cetech/resource.h>
#include "cetech/luasys/luasys.h"

#define API_NAME "ResourceManager"

IMPORT_API(resource_api_v0);

static int _reload_all(lua_State *l) {

    resource_api_v0.reload_all();
    return 0;
}

static int _compile_all(lua_State *l) {
    resource_api_v0.compiler_compile_all();
    return 0;
}


void _register_lua_resource_manager_api(get_api_fce_t get_engine_api) {
    resource_api_v0 = *(struct resource_api_v0 *) get_engine_api(RESOURCE_API_ID);

    luasys_add_module_function(API_NAME, "reload_all", _reload_all);
    luasys_add_module_function(API_NAME, "compile_all", _compile_all);
}