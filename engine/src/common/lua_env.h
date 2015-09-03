#pragma once

#include "lua_types.h"
#include "resources/lua.h"
#include "luajit/lua.hpp"

namespace cetech {
    namespace lua_enviroment_globals {
        const LuaEnviroment& global_env();
    }
    
    namespace lua_enviroment {
        CE_INLINE void execute_resource(const LuaEnviroment& env, const resource_lua::Resource* res);
    }
        
    namespace lua_enviroment {
        void execute_resource(const LuaEnviroment& env, const resource_lua::Resource* res) {
            if( luaL_dostring(env._state, resource_lua::get_source(res)) ){
                const char *last_error = lua_tostring(env._state, -1);
                lua_pop(env._state, 1);
                log::error("lua", "%s", last_error);            
            
                //lua_pop(env._state, 1);
            }
        }
    }
}