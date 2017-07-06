
#include <cetech/celib/allocator.h>
#include <cetech/celib/math_types.h>

#include <cetech/modules/luasys.h>
#include <cetech/modules/input.h>
#include <cetech/kernel/api_system.h>
#include "../luasys_private.h"

CETECH_DECL_API(ct_gamepad_a0);

static int _gamepad_button_index(lua_State *l) {
    const char *name = luasys_to_string(l, 1);

    uint32_t idx = ct_gamepad_a0.button_index(name);
    luasys_push_float(l, idx);

    return 1;
}

static int _gamepad_button_name(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_string(l, ct_gamepad_a0.button_name(idx));

    return 1;

}

static int _gamepad_button_state(lua_State *l) {
    uint32_t gamepad = luasys_to_int(l, 1);
    uint32_t idx = (uint32_t) (luasys_to_float(l, 2));

    luasys_push_bool(l, ct_gamepad_a0.button_state(gamepad, idx));

    return 1;
}

static int _gamepad_button_pressed(lua_State *l) {
    uint32_t gamepad = luasys_to_int(l, 1);
    uint32_t idx = (uint32_t) (luasys_to_float(l, 2));

    luasys_push_bool(l, ct_gamepad_a0.button_pressed(gamepad, idx));

    return 1;
}

static int _gamepad_button_released(lua_State *l) {
    uint32_t gamepad = luasys_to_int(l, 1);
    uint32_t idx = (uint32_t) (luasys_to_float(l, 2));

    luasys_push_bool(l, ct_gamepad_a0.button_released(gamepad, idx));

    return 1;

}


static int _gamepad_axis_index(lua_State *l) {
    const char *name = luasys_to_string(l, 1);

    uint32_t idx = ct_gamepad_a0.axis_index(name);
    luasys_push_int(l, idx);
    return 1;

}

static int _gamepad_axis_name(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_string(l, ct_gamepad_a0.axis_name(idx));

    return 1;
}

static int _gamepad_axis(lua_State *l) {
    uint32_t gamepad = luasys_to_int(l, 1);
    uint32_t idx = (uint32_t) (luasys_to_int(l, 2));

    vec2f_t pos = ct_gamepad_a0.axis(gamepad, idx);

    luasys_push_vec2f(l, pos);
    return 1;
}

static int _gamepad_is_active(lua_State *l) {
    uint32_t gamepad = luasys_to_int(l, 1);


    luasys_push_bool(l, ct_gamepad_a0.is_active(gamepad));
    return 1;
}

static int _gamepad_play_rumble(lua_State *l) {
    uint32_t gamepad = luasys_to_int(l, 1);
    float strength = luasys_to_float(l, 2);
    uint32_t length = luasys_to_int(l, 3);

    ct_gamepad_a0.play_rumble(gamepad, strength, length);
    return 0;
}

#define API_NAME "Gamepad"

void _register_lua_gamepad_api(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_gamepad_a0);

    luasys_add_module_function(API_NAME, "is_active", _gamepad_is_active);

    luasys_add_module_function(API_NAME, "button_index", _gamepad_button_index);
    luasys_add_module_function(API_NAME, "button_name", _gamepad_button_name);
    luasys_add_module_function(API_NAME, "button_state", _gamepad_button_state);
    luasys_add_module_function(API_NAME, "button_pressed",
                               _gamepad_button_pressed);
    luasys_add_module_function(API_NAME, "button_released",
                               _gamepad_button_released);

    luasys_add_module_function(API_NAME, "axis_index", _gamepad_axis_index);
    luasys_add_module_function(API_NAME, "axis_name", _gamepad_axis_name);
    luasys_add_module_function(API_NAME, "axis", _gamepad_axis);

    luasys_add_module_function(API_NAME, "play_rumble", _gamepad_play_rumble);
}