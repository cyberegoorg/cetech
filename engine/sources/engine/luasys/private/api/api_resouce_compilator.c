#include "engine/resource/resource.h"
#include "engine/luasys/luasys.h"

#define API_NAME "ResourceCompilator"

static int _compile_all(lua_State *l) {
    resource_compiler_compile_all();
    return 0;
}

void _register_lua_resource_compilator_api() {
    luasys_add_module_function(API_NAME, "compile_all", _compile_all);
}