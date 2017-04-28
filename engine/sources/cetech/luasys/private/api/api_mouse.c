#include <stddef.h>
#include <celib/allocator.h>
#include <celib/math_types.h>
#include <cetech/application/private/module.h>
#include "../luasys.h"
#include <cetech/input/input.h>

static struct MouseApiV0 MouseApiV0 = {0};

static int _mouse_button_index(lua_State *l) {
    const char *name = luasys_to_string(l, 1);

    uint32_t idx = MouseApiV0.button_index(name);
    luasys_push_float(l, idx);

    return 1;
}

static int _mouse_button_name(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_string(l, MouseApiV0.button_name(idx));

    return 1;

}

static int _mouse_button_state(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_bool(l, MouseApiV0.button_state(0, idx));

    return 1;

}

static int _mouse_button_pressed(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_bool(l, MouseApiV0.button_pressed(0, idx));

    return 1;

}

static int _mouse_button_released(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_bool(l, MouseApiV0.button_released(0, idx));

    return 1;

}


static int _mouse_axis_index(lua_State *l) {
    const char *name = luasys_to_string(l, 1);

    uint32_t idx = MouseApiV0.axis_index(name);
    luasys_push_int(l, idx);
    return 1;

}

static int _mouse_axis_name(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_string(l, MouseApiV0.axis_name(idx));

    return 1;
}

static int _mouse_axis(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_int(l, 1));

    cel_vec2f_t pos = MouseApiV0.axis(0, idx);

    luasys_push_vec2f(l, pos);
    return 1;
}

#define API_NAME "Mouse"

void _register_lua_mouse_api(get_api_fce_t get_engine_api) {
    MouseApiV0 = *((struct MouseApiV0 *) get_engine_api(MOUSE_API_ID, 0));


    luasys_add_module_function(API_NAME, "button_index", _mouse_button_index);
    luasys_add_module_function(API_NAME, "button_name", _mouse_button_name);
    luasys_add_module_function(API_NAME, "button_state", _mouse_button_state);
    luasys_add_module_function(API_NAME, "button_pressed",
                               _mouse_button_pressed);
    luasys_add_module_function(API_NAME, "button_released",
                               _mouse_button_released);

    luasys_add_module_function(API_NAME, "axis_index", _mouse_axis_index);
    luasys_add_module_function(API_NAME, "axis_name", _mouse_axis_name);
    luasys_add_module_function(API_NAME, "axis", _mouse_axis);
}