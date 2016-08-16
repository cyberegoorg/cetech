#include <cetech/luasystem/luasystem.h>
#include <celib/memory/types.h>
#include <cetech/input/input.h>

#define API_NAME "Keyboard"

static int _keyboard_button_index(lua_State *l) {
    const char *name = LUA.to_string(l, 1);

    u32 idx = keyboard_button_index(name);
    LUA.push_float(l, idx);

    return 1;
}

static int _keyboard_button_name(lua_State *l) {
    u32 idx = (u32) (LUA.to_float(l, 1));

    LUA.push_string(l, keyboard_button_name(idx));

    return 1;

}

static int _keyboard_button_state(lua_State *l) {
    u32 idx = (u32) (LUA.to_float(l, 1));

    LUA.push_bool(l, keyboard_button_state(idx));

    return 1;

}

static int _keyboard_button_pressed(lua_State *l) {
    u32 idx = (u32) (LUA.to_float(l, 1));

    LUA.push_bool(l, keyboard_button_pressed(idx));

    return 1;

}

static int _keyboard_button_released(lua_State *l) {
    u32 idx = (u32) (LUA.to_float(l, 1));

    LUA.push_bool(l, keyboard_button_released(idx));

    return 1;

}


void _register_lua_keyboard_api() {
    LUA.add_module_function(API_NAME, "button_index", _keyboard_button_index);
    LUA.add_module_function(API_NAME, "button_name", _keyboard_button_name);
    LUA.add_module_function(API_NAME, "button_state", _keyboard_button_state);
    LUA.add_module_function(API_NAME, "button_pressed", _keyboard_button_pressed);
    LUA.add_module_function(API_NAME, "button_released", _keyboard_button_released);
}