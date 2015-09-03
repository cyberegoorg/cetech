#include "common/lua_env.h"

namespace cetech {
    namespace lua_enviroment_globals {
        static LuaEnviroment _global_env;
        
        const cetech::LuaEnviroment& global_env() {
            return _global_env;
        }
    }
    
    LuaEnviroment::LuaEnviroment() {
        _state = luaL_newstate();
        CE_CHECK_PTR(_state);
        
        luaL_openlibs(_state);
    }
    
    LuaEnviroment::~LuaEnviroment(){
        lua_close(_state);
    }
}