
#include <cetech/celib/allocator.h>
#include <cetech/celib/math_types.h>
#include <cetech/kernel/module.h>
#include <cetech/modules/luasys.h>
#include <cetech/modules/input.h>
#include <cetech/kernel/api.h>

CETECH_DECL_API(mouse_api_v0);

static int _mouse_button_index(lua_State *l) {
    const char *name = luasys_to_string(l, 1);

    uint32_t idx = mouse_api_v0.button_index(name);
    luasys_push_float(l, idx);

    return 1;
}

static int _mouse_button_name(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_string(l, mouse_api_v0.button_name(idx));

    return 1;

}

static int _mouse_button_state(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_bool(l, mouse_api_v0.button_state(0, idx));

    return 1;

}

static int _mouse_button_pressed(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_bool(l, mouse_api_v0.button_pressed(0, idx));

    return 1;

}

static int _mouse_button_released(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_bool(l, mouse_api_v0.button_released(0, idx));

    return 1;

}


static int _mouse_axis_index(lua_State *l) {
    const char *name = luasys_to_string(l, 1);

    uint32_t idx = mouse_api_v0.axis_index(name);
    luasys_push_int(l, idx);
    return 1;

}

static int _mouse_axis_name(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_string(l, mouse_api_v0.axis_name(idx));

    return 1;
}

static int _mouse_axis(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_int(l, 1));

    vec2f_t pos = mouse_api_v0.axis(0, idx);

    luasys_push_vec2f(l, pos);
    return 1;
}

#define API_NAME "Mouse"

void _register_lua_mouse_api(struct api_v0 *api) {
    CETECH_GET_API(api, mouse_api_v0);


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