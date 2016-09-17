#include <celib/math/types.h>
#include "engine/lua_system/lua_system.h"
#include "engine/input/input.h"

#define API_NAME "Mouse"

static int _mouse_button_index(lua_State *l) {
    const char *name = luasys_to_string(l, 1);

    u32 idx = mouse_button_index(name);
    luasys_push_float(l, idx);

    return 1;
}

static int _mouse_button_name(lua_State *l) {
    u32 idx = (u32) (luasys_to_f32(l, 1));

    luasys_push_string(l, mouse_button_name(idx));

    return 1;

}

static int _mouse_button_state(lua_State *l) {
    u32 idx = (u32) (luasys_to_f32(l, 1));

    luasys_push_bool(l, mouse_button_state(idx));

    return 1;

}

static int _mouse_button_pressed(lua_State *l) {
    u32 idx = (u32) (luasys_to_f32(l, 1));

    luasys_push_bool(l, mouse_button_pressed(idx));

    return 1;

}

static int _mouse_button_released(lua_State *l) {
    u32 idx = (u32) (luasys_to_f32(l, 1));

    luasys_push_bool(l, mouse_button_released(idx));

    return 1;

}


static int _mouse_axis_index(lua_State *l) {
    const char *name = luasys_to_string(l, 1);

    u32 idx = mouse_axis_index(name);
    luasys_push_int(l, idx);
    return 1;

}

static int _mouse_axis_name(lua_State *l) {
    u32 idx = (u32) (luasys_to_f32(l, 1));

    luasys_push_string(l, mouse_axis_name(idx));

    return 1;
}

static int _mouse_axis(lua_State *l) {
    u32 idx = (u32) (luasys_to_int(l, 1));

    vec2f_t pos = mouse_axis(idx);

    luasys_push_vec2f(l, pos);
    return 1;
}

void _register_lua_mouse_api() {
    luasys_add_module_function(API_NAME, "button_index", _mouse_button_index);
    luasys_add_module_function(API_NAME, "button_name", _mouse_button_name);
    luasys_add_module_function(API_NAME, "button_state", _mouse_button_state);
    luasys_add_module_function(API_NAME, "button_pressed", _mouse_button_pressed);
    luasys_add_module_function(API_NAME, "button_released", _mouse_button_released);

    luasys_add_module_function(API_NAME, "axis_index", _mouse_axis_index);
    luasys_add_module_function(API_NAME, "axis_name", _mouse_axis_name);
    luasys_add_module_function(API_NAME, "axis", _mouse_axis);
}