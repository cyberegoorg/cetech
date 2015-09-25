#include "lua/lua_enviroment.h"
#include "lua/lua_stack.h"

#include "device.h"

namespace cetech {
    static int device_quit(lua_State* L) {
        device_globals::device().quit();
        return 0;
    }

    static int device_get_frame_id(lua_State* L) {
        LuaStack(L).push_uint32(device_globals::device().get_frame_id());
        return 1;
    }

    static int device_get_delta_time(lua_State* L) {
        LuaStack(L).push_uint32(device_globals::device().get_delta_time());
        return 1;
    }

    namespace lua_device {
        static const char* module_name = "Device";

        void load_libs(LuaEnviroment& env) {
            env.set_module_function(module_name, "quit", device_quit);
            env.set_module_function(module_name, "get_frame_id", device_get_frame_id);
            env.set_module_function(module_name, "get_delta_time", device_get_delta_time);
        }
    }
}