
#include <cetech/celib/allocator.h>
#include <cetech/kernel/module.h>

#include <cetech/modules/entity.h>
#include <cetech/modules/luasys.h>
#include <cetech/kernel/hash.h>
#include <cetech/kernel/api_system.h>

#include "cetech/modules/renderer.h"

#define API_NAME "Mesh"

CETECH_DECL_API(ct_mesh_renderer_api_v0);
CETECH_DECL_API(ct_hash_api_v0);

static int _mesh_get(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t ent = {.h = luasys_to_uin32_t(l, 2)};

    luasys_push_int(l, ct_mesh_renderer_api_v0.get(w, ent).idx);
    return 1;
}


static int _mesh_has(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t ent = {.h = luasys_to_uin32_t(l, 2)};

    luasys_push_bool(l, ct_mesh_renderer_api_v0.has(w, ent));
    return 1;
}


static int _mesh_get_material(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    mesh_renderer_t m = {.idx = luasys_to_int(l, 2)};

    luasys_push_handler(l, ct_mesh_renderer_api_v0.get_material(w, m).idx);
    return 1;
}

static int _mesh_set_material(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    mesh_renderer_t m = {.idx = luasys_to_int(l, 2)};
    uint64_t material = ct_hash_api_v0.id64_from_str(luasys_to_string(l, 3));

    ct_mesh_renderer_api_v0.set_material(w, m, material);

    return 0;
}

void _register_lua_mesh_api(struct ct_api_v0 *api) {
    if (api->exist("ct_mesh_renderer_api_v0")) {
        CETECH_GET_API(api, ct_mesh_renderer_api_v0);
        CETECH_GET_API(api, ct_hash_api_v0);

        luasys_add_module_function(API_NAME, "get", _mesh_get);
        luasys_add_module_function(API_NAME, "has", _mesh_has);

        luasys_add_module_function(API_NAME, "get_material",
                                   _mesh_get_material);
        luasys_add_module_function(API_NAME, "set_material",
                                   _mesh_set_material);
    }
}