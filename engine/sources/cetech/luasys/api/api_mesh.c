
#include <cetech/allocator.h>
#include <cetech/world.h>
#include <cetech/config.h>
#include <cetech/application.h>
#include <cetech/vio.h>
#include <cetech/resource.h>
#include <cetech/entity.h>
#include <cetech/resource.h>
#include <cetech/vio.h>
#include <cetech/resource.h>
#include <cetech/renderer.h>
#include <cetech/module.h>
#include "cetech/luasys.h"
#include "../luasys.h"

#define API_NAME "Mesh"

IMPORT_API(mesh_renderer_api_v0);

static int _mesh_get(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t ent = {.h = luasys_to_uin32_t(l, 2)};

    luasys_push_int(l, mesh_renderer_api_v0.get(w, ent).idx);
    return 1;
}


static int _mesh_has(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t ent = {.h = luasys_to_uin32_t(l, 2)};

    luasys_push_bool(l, mesh_renderer_api_v0.has(w, ent));
    return 1;
}


static int _mesh_get_material(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    mesh_renderer_t m = {.idx = luasys_to_int(l, 2)};

    luasys_push_handler(l, mesh_renderer_api_v0.get_material(w, m).idx);
    return 1;
}

static int _mesh_set_material(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    mesh_renderer_t m = {.idx = luasys_to_int(l, 2)};
    stringid64_t material = stringid64_from_string(luasys_to_string(l, 3));

    mesh_renderer_api_v0.set_material(w, m, material);

    return 0;
}

void _register_lua_mesh_api(get_api_fce_t get_engine_api) {
    mesh_renderer_api_v0 = *(struct mesh_renderer_api_v0 *) get_engine_api(MESH_API_ID);


    luasys_add_module_function(API_NAME, "get", _mesh_get);
    luasys_add_module_function(API_NAME, "has", _mesh_has);

    luasys_add_module_function(API_NAME, "get_material", _mesh_get_material);
    luasys_add_module_function(API_NAME, "set_material", _mesh_set_material);
}