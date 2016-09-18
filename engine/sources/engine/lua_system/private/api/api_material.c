
#include <engine/entcom/types.h>
#include <engine/world_system/transform.h>
#include <celib/math/mat44f.h>
#include <engine/renderer/mesh_renderer.h>
#include <engine/renderer/material.h>
#include <celib/stringid/stringid.h>
#include "engine/lua_system/lua_system.h"

#define API_NAME "Material"

static int _find_slot(lua_State *l) {
    material_t m = {.h = luasys_to_handler(l, 1)};
    const char *slot_name = luasys_to_string(l, 2);

    luasys_push_int(l, material_find_slot(m, slot_name));
    return 1;
}

static int _set_texture(lua_State *l) {
    material_t m = {.h = luasys_to_handler(l, 1)};
    u32 slot = luasys_to_int(l, 2);
    const char *texute_name = luasys_to_string(l, 3);

    material_set_texture(m, slot, stringid64_from_string(texute_name));
    return 0;
}


void material_set_texture(material_t material,
                          u32 slot,
                          stringid64_t texture);

void _register_lua_material_api() {
    luasys_add_module_function(API_NAME, "find_slot", _find_slot);
    luasys_add_module_function(API_NAME, "set_texture", _set_texture);
}