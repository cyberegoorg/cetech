#pragma once

#include "lua_types.h"
#include "resources/lua.h"
#include "luajit/lua.hpp"

namespace cetech {
    namespace lua_enviroment_globals {
        const LuaEnviroment& global_env();
    }

    namespace lua_enviroment {
        void execute_resource(const LuaEnviroment& env, const resource_lua::Resource* res);

    }

}