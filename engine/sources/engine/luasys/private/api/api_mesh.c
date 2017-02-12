
#include "engine/entcom/entcom.h"
#include <engine/renderer/mesh_renderer.h>
#include <engine/plugin/plugin.h>
#include "engine/luasys/luasys.h"
#include <engine/plugin/plugin_api.h>

#define API_NAME "Mesh"

struct MeshApiV1 MeshApiV1;

static int _mesh_get(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t ent = {.h = luasys_to_handler(l, 2)};

    luasys_push_int(l, MeshApiV1.get(w, ent).idx);
    return 1;
}


static int _mesh_has(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t ent = {.h = luasys_to_handler(l, 2)};

    luasys_push_bool(l, MeshApiV1.has(w, ent));
    return 1;
}


static int _mesh_get_material(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    mesh_t m = {.idx = luasys_to_int(l, 2)};

    luasys_push_handler(l, MeshApiV1.get_material(w, m).h);
    return 1;
}

static int _mesh_set_material(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    mesh_t m = {.idx = luasys_to_int(l, 2)};
    stringid64_t material = stringid64_from_string(luasys_to_string(l, 3));

    MeshApiV1.set_material(w, m, material);

    return 0;
}

void _register_lua_mesh_api(get_api_fce_t get_engine_api) {
    MeshApiV1 = *(struct MeshApiV1 *) get_engine_api(MESH_API_ID, 0);


    luasys_add_module_function(API_NAME, "get", _mesh_get);
    luasys_add_module_function(API_NAME, "has", _mesh_has);

    luasys_add_module_function(API_NAME, "get_material", _mesh_get_material);
    luasys_add_module_function(API_NAME, "set_material", _mesh_set_material);
}