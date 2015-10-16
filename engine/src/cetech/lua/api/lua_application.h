#include "celib/macros.h"

#include "cetech/lua/lua_enviroment.h"
#include "cetech/lua/lua_stack.h"

#include "cetech/application/application.h"

namespace cetech {
    static int application_quit(lua_State* L) {
	CE_UNUSED(L);
	
        application_globals::app().quit();
        return 0;
    }

    static int application_get_frame_id(lua_State* L) {
        LuaStack(L).push_uint32(application_globals::app().get_frame_id());
        return 1;
    }

    static int application_get_delta_time(lua_State* L) {
        LuaStack(L).push_uint32(application_globals::app().get_delta_time());
        return 1;
    }

    static int application_console_send(lua_State* L) {
        rapidjson::Document d;
        d.SetObject();
        LuaStack(L).to_json(1, d, d);

        application_globals::app().console_server().send_json_document(d);

        return 0;
    }

    static int application_resize(lua_State* L) {
        LuaStack s(L);

        application_globals::app().renderer().resize(s.to_int(1), s.to_int(2));

        return 0;
    }

    namespace lua_application {
        static const char* module_name = "Application";

        void load_libs(LuaEnviroment& env) {
            env.set_module_function(module_name, "quit", application_quit);
            env.set_module_function(module_name, "get_frame_id", application_get_frame_id);
            env.set_module_function(module_name, "get_delta_time", application_get_delta_time);

	    env.set_module_function(module_name, "console_send", application_console_send);
	    env.set_module_function(module_name, "resize", application_resize);
        }
    }
}