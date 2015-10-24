#include "celib/macros.h"

#include "cetech/lua/lua_enviroment.h"
#include "cetech/lua/lua_stack.h"

namespace cetech {
    static int log_info(lua_State* L) {
        LuaStack s(L);
        const char* where = s.to_string(1);
        const char* msg = s.to_string(2);

        log_globals::log().info(where, "%s", msg);
        return 0;
    }

    static int log_warning(lua_State* L) {
        LuaStack s(L);
        const char* where = s.to_string(1);
        const char* msg = s.to_string(2);

        log_globals::log().warning(where, "%s", msg);
        return 0;
    }

    static int log_error(lua_State* L) {
        LuaStack s(L);
        const char* where = s.to_string(1);
        const char* msg = s.to_string(2);

        log_globals::log().error(where, "%s", msg);
        return 0;
    }

    static int log_debug(lua_State* L) {
        LuaStack s(L);
        const char* where = s.to_string(1);
        const char* msg = s.to_string(2);

        log_globals::log().debug(where, "%s", msg);
        return 0;
    }

    namespace lua_log {
        static const char* module_name = "Log";

        void load_libs(LuaEnviroment& env) {
            env.set_module_function(module_name, "_info", log_info);
            env.set_module_function(module_name, "_warning", log_warning);
            env.set_module_function(module_name, "_error", log_error);
            env.set_module_function(module_name, "_debug", log_debug);
        }
    }
}