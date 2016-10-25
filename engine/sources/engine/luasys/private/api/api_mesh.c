
#include "engine/entcom/types.h"
#include <engine/world/transform.h>
#include <celib/math/mat44f.h>
#include <engine/renderer/mesh_renderer.h>
#include <engine/renderer/material.h>
#include <celib/string/stringid.h>
#include "engine/luasys/luasys.h"

#define API_NAME "Mesh"

static int _mesh_get(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t ent = {.h = luasys_to_handler(l, 2)};

    luasys_push_int(l, mesh_get(w, ent).idx);
    return 1;
}


static int _mesh_has(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t ent = {.h = luasys_to_handler(l, 2)};

    luasys_push_bool(l, mesh_has(w, ent));
    return 1;
}


static int _mesh_get_material(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    mesh_t m = {.idx = luasys_to_int(l, 2)};

    luasys_push_handler(l, mesh_get_material(w, m).h);
    return 1;
}

static int _mesh_set_material(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    mesh_t m = {.idx = luasys_to_int(l, 2)};
    stringid64_t material = stringid64_from_string(luasys_to_string(l, 3));

    mesh_set_material(w, m, material);
    return 0;
}

void _register_lua_mesh_api() {
    luasys_add_module_function(API_NAME, "get", _mesh_get);
    luasys_add_module_function(API_NAME, "has", _mesh_has);

    luasys_add_module_function(API_NAME, "get_material", _mesh_get_material);
    luasys_add_module_function(API_NAME, "set_material", _mesh_set_material);
}