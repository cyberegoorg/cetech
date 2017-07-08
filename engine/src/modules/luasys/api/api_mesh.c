
#include <cetech/celib/allocator.h>

#include <cetech/modules/entity.h>
#include <cetech/modules/luasys.h>
#include <cetech/kernel/hash.h>
#include <cetech/kernel/api_system.h>

#include "cetech/modules/renderer.h"
#include "../luasys_private.h"

#define API_NAME "Mesh"

CETECH_DECL_API(ct_mesh_renderer_a0);
CETECH_DECL_API(ct_hash_a0);

struct ct_mesh_renderer _get_mesh(lua_State *l, int idx) {
    uint64_t t = luasys_to_u64(l, idx);
    return *((struct ct_mesh_renderer*) &t);
}

void _push_mesh(lua_State *l, struct ct_mesh_renderer mesh) {
    uint64_t t = *((uint64_t*)&mesh);
    luasys_push_uint64_t(l, t);
}

static int _mesh_get(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_entity ent = {.h = luasys_to_u32(l, 2)};

    _push_mesh(l, ct_mesh_renderer_a0.get(w, ent));
    return 1;
}


static int _mesh_has(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_entity ent = {.h = luasys_to_u32(l, 2)};

    luasys_push_bool(l, ct_mesh_renderer_a0.has(w, ent));
    return 1;
}


static int _mesh_get_material(lua_State *l) {
    struct ct_mesh_renderer m =_get_mesh(l, 1);

    luasys_push_handler(l, ct_mesh_renderer_a0.get_material(m).idx);
    return 1;
}

static int _mesh_set_material(lua_State *l) {
    struct ct_mesh_renderer m = _get_mesh(l, 1);

    uint64_t material = ct_hash_a0.id64_from_str(luasys_to_string(l, 3));

    ct_mesh_renderer_a0.set_material(m, material);

    return 0;
}

void _register_lua_mesh_api(struct ct_api_a0 *api) {
    if (api->exist("ct_mesh_renderer_a0")) {
        CETECH_GET_API(api, ct_mesh_renderer_a0);
        CETECH_GET_API(api, ct_hash_a0);

        luasys_add_module_function(API_NAME, "get", _mesh_get);
        luasys_add_module_function(API_NAME, "has", _mesh_has);

        luasys_add_module_function(API_NAME, "get_material",
                                   _mesh_get_material);
        luasys_add_module_function(API_NAME, "set_material",
                                   _mesh_set_material);
    }
}