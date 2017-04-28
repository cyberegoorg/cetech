#include <stddef.h>
#include "cetech/memory/allocator.h"
#include <cetech/filesystem/vio.h>

#include <cetech/application/private/module.h>
#include <cetech/resource/resource.h>
#include "../luasys.h"

#define API_NAME "ResourceManager"

struct ResourceApiV0 ResourceApiV0;

static int _reload_all(lua_State *l) {

    ResourceApiV0.reload_all();
    return 0;
}

static int _compile_all(lua_State *l) {
    ResourceApiV0.compiler_compile_all();
    return 0;
}


void _register_lua_resource_manager_api(get_api_fce_t get_engine_api) {
    ResourceApiV0 = *(struct ResourceApiV0 *) get_engine_api(RESOURCE_API_ID,
                                                             0);

    luasys_add_module_function(API_NAME, "reload_all", _reload_all);
    luasys_add_module_function(API_NAME, "compile_all", _compile_all);
}