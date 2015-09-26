#pragma once

namespace cetech {
    class LuaEnviroment;

    namespace lua_package {
        void load_libs(LuaEnviroment& env);
    }
}