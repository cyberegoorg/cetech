#include <celib/math/types.h>

#include "engine/luasys/luasys.h"
#include "engine/input/input.h"

#define API_NAME "Gamepad"

static int _gamepad_button_index(lua_State *l) {
    const char *name = luasys_to_string(l, 1);

    u32 idx = gamepad_button_index(name);
    luasys_push_float(l, idx);

    return 1;
}

static int _gamepad_button_name(lua_State *l) {
    u32 idx = (u32) (luasys_to_f32(l, 1));

    luasys_push_string(l, gamepad_button_name(idx));

    return 1;

}

static int _gamepad_button_state(lua_State *l) {
    gamepad_t gamepad = {.idx = (luasys_to_int(l, 1))};
    u32 idx = (u32) (luasys_to_f32(l, 2));

    luasys_push_bool(l, gamepad_button_state(gamepad, idx));

    return 1;

}

static int _gamepad_button_pressed(lua_State *l) {
    gamepad_t gamepad = {.idx = (luasys_to_int(l, 1))};
    u32 idx = (u32) (luasys_to_f32(l, 2));

    luasys_push_bool(l, gamepad_button_pressed(gamepad, idx));

    return 1;

}

static int _gamepad_button_released(lua_State *l) {
    gamepad_t gamepad = {.idx = (luasys_to_int(l, 1))};
    u32 idx = (u32) (luasys_to_f32(l, 2));

    luasys_push_bool(l, gamepad_button_released(gamepad, idx));

    return 1;

}


static int _gamepad_axis_index(lua_State *l) {
    const char *name = luasys_to_string(l, 1);

    u32 idx = gamepad_axis_index(name);
    luasys_push_int(l, idx);
    return 1;

}

static int _gamepad_axis_name(lua_State *l) {
    u32 idx = (u32) (luasys_to_f32(l, 1));

    luasys_push_string(l, gamepad_axis_name(idx));

    return 1;
}

static int _gamepad_axis(lua_State *l) {
    gamepad_t gamepad = {.idx = (luasys_to_int(l, 1))};
    u32 idx = (u32) (luasys_to_int(l, 2));

    cel_vec2f_t pos = gamepad_axis(gamepad, idx);

    luasys_push_vec2f(l, pos);
    return 1;
}

static int _gamepad_is_active(lua_State *l) {
    gamepad_t gamepad = {.idx = (luasys_to_int(l, 1))};


    luasys_push_bool(l, gamepad_is_active(gamepad));
    return 1;
}

static int _gamepad_play_rumble(lua_State *l) {
    gamepad_t gamepad = {.idx = (luasys_to_int(l, 1))};
    f32 strength = luasys_to_f32(l, 2);
    u32 length = luasys_to_int(l, 3);

    gamepad_play_rumble(gamepad, strength, length);
    return 0;
}


void _register_lua_gamepad_api() {
    luasys_add_module_function(API_NAME, "is_active", _gamepad_is_active);

    luasys_add_module_function(API_NAME, "button_index", _gamepad_button_index);
    luasys_add_module_function(API_NAME, "button_name", _gamepad_button_name);
    luasys_add_module_function(API_NAME, "button_state", _gamepad_button_state);
    luasys_add_module_function(API_NAME, "button_pressed", _gamepad_button_pressed);
    luasys_add_module_function(API_NAME, "button_released", _gamepad_button_released);

    luasys_add_module_function(API_NAME, "axis_index", _gamepad_axis_index);
    luasys_add_module_function(API_NAME, "axis_name", _gamepad_axis_name);
    luasys_add_module_function(API_NAME, "axis", _gamepad_axis);

    luasys_add_module_function(API_NAME, "play_rumble", _gamepad_play_rumble);
}