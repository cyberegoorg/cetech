#include "lua/lua_enviroment.h"
#include "lua/lua_stack.h"

#include "runtime/runtime.h"

namespace cetech {
    static int keyboard_button_index(lua_State* L) {
        LuaStack stack(L);
        
        const char* scancode = stack.get_string(1);
        stack.push_uint32(runtime::keyboard::button_index(scancode));
        return 1;
    }

    static int keyboard_button_name(lua_State* L) {
        LuaStack stack(L);
        
        uint32_t index = stack.get_int(1);
        stack.push_string(runtime::keyboard::button_name(index));
        return 1;
    }

    static int keyboard_button_state(lua_State* L) {
        LuaStack stack(L);
        
        uint32_t index = stack.get_int(1);
        stack.push_bool(runtime::keyboard::button_state(index));
        return 1;
    }

    static int keyboard_button_pressed(lua_State* L) {
        LuaStack stack(L);
        
        uint32_t index = stack.get_int(1);
        stack.push_bool(runtime::keyboard::button_pressed(index));
        return 1;
    }

    static int keyboard_button_released(lua_State* L) {
        LuaStack stack(L);
        
        uint32_t index = stack.get_int(1);
        stack.push_bool(runtime::keyboard::button_released(index));
        return 1;
    }

    namespace lua_keyboard {
        static const char* module_name = "Keyboard";

        void load_libs(LuaEnviroment& env) {
            env.set_module_function(module_name, "button_index", keyboard_button_index);
            env.set_module_function(module_name, "button_name", keyboard_button_name);
            env.set_module_function(module_name, "button_state", keyboard_button_state);
            env.set_module_function(module_name, "button_pressed", keyboard_button_pressed);
            env.set_module_function(module_name, "button_released", keyboard_button_released);
        }
    }
}