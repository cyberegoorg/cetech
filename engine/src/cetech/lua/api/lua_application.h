#include "celib/macros.h"

#include "cetech/lua/lua_enviroment.h"
#include "cetech/lua/lua_stack.h"

#include "cetech/application/application.h"
#include "cetech/renderer/renderer.h"
#include "cetech/develop/console_server.h"

namespace cetech {
    static int application_quit(lua_State* L) {
        CE_UNUSED(L);

        application::quit();
        return 0;
    }

    static int application_get_frame_id(lua_State* L) {
        LuaStack(L).push_uint32(application::get_frame_id());
        return 1;
    }

    static int application_get_delta_time(lua_State* L) {
        LuaStack(L).push_uint32(application::get_delta_time());
        return 1;
    }

    static int application_console_send(lua_State* L) {
        LuaStack stack(L);
        
        Array<char> buffer(memory_globals::default_allocator());
        
        char buf[256];      
        array::push(buffer, buf, snprintf(buf, 256, "#%s\n", stack.to_string(1)));

        stack.to_yaml(2, buffer, 0);

        console_server::send_msg(buffer);

        return 0;
    }

    static int application_resize(lua_State* L) {
        LuaStack s(L);
        renderer::resize(s.to_int(1), s.to_int(2));
        return 0;
    }

    namespace lua_application {
        static const char* module_name = "Application";

        void load_libs() {
            lua_enviroment::set_module_function(module_name, "quit", application_quit);
            lua_enviroment::set_module_function(module_name, "get_frame_id", application_get_frame_id);
            lua_enviroment::set_module_function(module_name, "get_delta_time", application_get_delta_time);

            lua_enviroment::set_module_function(module_name, "console_send", application_console_send);
            lua_enviroment::set_module_function(module_name, "resize", application_resize);
        }
    }
}
