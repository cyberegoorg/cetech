#pragma once

struct lua_State;

namespace cetech {
    struct LuaEnviroment {
        lua_State* _state;

        LuaEnviroment();
        ~LuaEnviroment();
    };
}