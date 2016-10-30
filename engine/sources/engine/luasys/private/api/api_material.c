
#include <engine/world/transform.h>
#include <engine/renderer/mesh_renderer.h>
#include "engine/luasys/luasys.h"

#define API_NAME "Material"

static int _set_texture(lua_State *l) {
    material_t m = {.h = luasys_to_handler(l, 1)};
    const char *slot_name = luasys_to_string(l, 2);
    const char *texture_name = luasys_to_string(l, 3);

    material_set_texture(m, slot_name, stringid64_from_string(texture_name));
    return 0;
}


static int _set_vec4f(lua_State *l) {
    material_t m = {.h = luasys_to_handler(l, 1)};
    const char *slot_name = luasys_to_string(l, 2);
    vec4f_t *v = luasys_to_vec4f(l, 3);

    material_set_vec4f(m, slot_name, *v);
    return 0;
}

void _register_lua_material_api() {
    luasys_add_module_function(API_NAME, "set_texture", _set_texture);
    luasys_add_module_function(API_NAME, "set_vec4f", _set_vec4f);
}