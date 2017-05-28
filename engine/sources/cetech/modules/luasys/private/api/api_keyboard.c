
#include <cetech/core/memory/allocator.h>
#include <cetech/core/module.h>

#include <cetech/modules/input/input.h>
#include <cetech/core/api.h>
#include "../../luasys.h"

#define API_NAME "Keyboard"

IMPORT_API(keyboard_api_v0);

static int _keyboard_button_index(lua_State *l) {
    const char *name = luasys_to_string(l, 1);

    uint32_t idx = keyboard_api_v0.button_index(name);
    luasys_push_float(l, idx);

    return 1;
}

static int _keyboard_button_name(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_string(l, keyboard_api_v0.button_name(idx));

    return 1;

}

static int _keyboard_button_state(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_bool(l, keyboard_api_v0.button_state(0, idx));

    return 1;
}

static int _keyboard_button_pressed(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_bool(l, keyboard_api_v0.button_pressed(0, idx));

    return 1;

}

static int _keyboard_button_released(lua_State *l) {
    uint32_t idx = (uint32_t) (luasys_to_float(l, 1));

    luasys_push_bool(l, keyboard_api_v0.button_released(0, idx));

    return 1;

}


void _register_lua_keyboard_api(struct api_v0 *api) {
    GET_API(api, keyboard_api_v0);

    luasys_add_module_function(API_NAME, "button_index",
                               _keyboard_button_index);
    luasys_add_module_function(API_NAME, "button_name", _keyboard_button_name);
    luasys_add_module_function(API_NAME, "button_state",
                               _keyboard_button_state);
    luasys_add_module_function(API_NAME, "button_pressed",
                               _keyboard_button_pressed);
    luasys_add_module_function(API_NAME, "button_released",
                               _keyboard_button_released);
}