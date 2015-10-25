#pragma once

#include "celib/math/math_types.h"

#include "luajit/lua.hpp"
#include "lua_resource.h"

namespace cetech {

    namespace lua_enviroment {
        void execute_resource(const resource_lua::Resource* res);
        void execute_string(const char* str);

        void set_module_function(const char* module, const char* name, const lua_CFunction func);
        void set_module_constructor(const char* module, const lua_CFunction func);

        void call_global(const char* func, const char* args = nullptr, ...);

        void clean_temp();
        Vector2& new_tmp_vector2();
        Vector3& new_tmp_vector3();
    }

    namespace lua_enviroment_globals {
        void init();
        void shutdown();
    }
}