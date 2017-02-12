#include <celib/math/types.h>
#include <engine/plugin/plugin.h>
#include "engine/luasys/luasys.h"
#include "engine/input/mouse.h"

static struct MouseApiV1 MouseApiV1 = {0};

static int _mouse_button_index(lua_State *l) {
    const char *name = luasys_to_string(l, 1);

    u32 idx = MouseApiV1.button_index(name);
    luasys_push_float(l, idx);

    return 1;
}

static int _mouse_button_name(lua_State *l) {
    u32 idx = (u32) (luasys_to_f32(l, 1));

    luasys_push_string(l, MouseApiV1.button_name(idx));

    return 1;

}

static int _mouse_button_state(lua_State *l) {
    u32 idx = (u32) (luasys_to_f32(l, 1));

    luasys_push_bool(l, MouseApiV1.button_state(0, idx));

    return 1;

}

static int _mouse_button_pressed(lua_State *l) {
    u32 idx = (u32) (luasys_to_f32(l, 1));

    luasys_push_bool(l, MouseApiV1.button_pressed(0,idx));

    return 1;

}

static int _mouse_button_released(lua_State *l) {
    u32 idx = (u32) (luasys_to_f32(l, 1));

    luasys_push_bool(l, MouseApiV1.button_released(0,idx));

    return 1;

}


static int _mouse_axis_index(lua_State *l) {
    const char *name = luasys_to_string(l, 1);

    u32 idx = MouseApiV1.axis_index(name);
    luasys_push_int(l, idx);
    return 1;

}

static int _mouse_axis_name(lua_State *l) {
    u32 idx = (u32) (luasys_to_f32(l, 1));

    luasys_push_string(l, MouseApiV1.axis_name(idx));

    return 1;
}

static int _mouse_axis(lua_State *l) {
    u32 idx = (u32) (luasys_to_int(l, 1));

    cel_vec2f_t pos = MouseApiV1.axis(0, idx);

    luasys_push_vec2f(l, pos);
    return 1;
}

#define API_NAME "Mouse"
void _register_lua_mouse_api() {
    MouseApiV1 = *((struct MouseApiV1*)plugin_get_engine_api(MOUSE_API_ID, 0));


    luasys_add_module_function(API_NAME, "button_index", _mouse_button_index);
    luasys_add_module_function(API_NAME, "button_name", _mouse_button_name);
    luasys_add_module_function(API_NAME, "button_state", _mouse_button_state);
    luasys_add_module_function(API_NAME, "button_pressed", _mouse_button_pressed);
    luasys_add_module_function(API_NAME, "button_released", _mouse_button_released);

    luasys_add_module_function(API_NAME, "axis_index", _mouse_axis_index);
    luasys_add_module_function(API_NAME, "axis_name", _mouse_axis_name);
    luasys_add_module_function(API_NAME, "axis", _mouse_axis);
}