#include "lua/lua_enviroment.h"
#include "lua/lua_stack.h"

#include "application.h"

namespace cetech {
    static int application_quit(lua_State* L) {
        application_globals::device().quit();
        return 0;
    }

    static int application_get_frame_id(lua_State* L) {
        LuaStack(L).push_uint32(application_globals::device().get_frame_id());
        return 1;
    }

    static int application_get_delta_time(lua_State* L) {
        LuaStack(L).push_uint32(application_globals::device().get_delta_time());
        return 1;
    }

    namespace lua_application {
        static const char* module_name = "Application";

        void load_libs(LuaEnviroment& env) {
            env.set_module_function(module_name, "quit", application_quit);
            env.set_module_function(module_name, "get_frame_id", application_get_frame_id);
            env.set_module_function(module_name, "get_delta_time", application_get_delta_time);
        }
    }
}