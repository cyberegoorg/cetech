#include "cetech/modules/transform.h"
#include <cetech/kernel/module.h>
#include <cetech/modules/luasys.h>

#include <cetech/modules/entity.h>
#include <cetech/kernel/api_system.h>

#define API_NAME "Transform"

CETECH_DECL_API(ct_transform_api_v0);

static int _transform_get(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t ent = {.h = luasys_to_handler(l, 2)};

    luasys_push_int(l, ct_transform_api_v0.get(w, ent).idx);
    return 1;
}


static int _transform_has(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t ent = {.h = luasys_to_handler(l, 2)};

    luasys_push_bool(l, ct_transform_api_v0.has(w, ent));
    return 1;
}


static int _transform_get_position(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    ct_transform_t t = {.idx = luasys_to_int(l, 2)};

    luasys_push_vec3f(l, ct_transform_api_v0.get_position(w, t));
    return 1;
}

static int _transform_get_rotation(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    ct_transform_t t = {.idx = luasys_to_int(l, 2)};

    luasys_push_quat(l, ct_transform_api_v0.get_rotation(w, t));
    return 1;
}

static int _transform_get_scale(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    ct_transform_t t = {.idx = luasys_to_int(l, 2)};

    luasys_push_vec3f(l, ct_transform_api_v0.get_scale(w, t));
    return 1;
}

static int _transform_set_position(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    ct_transform_t t = {.idx = luasys_to_int(l, 2)};
    vec3f_t *pos = luasys_to_vec3f(l, 3);

    ct_transform_api_v0.set_position(w, t, *pos);
    return 0;
}

static int _transform_set_scale(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    ct_transform_t t = {.idx = luasys_to_int(l, 2)};
    vec3f_t *pos = luasys_to_vec3f(l, 3);

    ct_transform_api_v0.set_scale(w, t, *pos);
    return 0;
}

static int _transform_set_rotation(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    ct_transform_t t = {.idx = luasys_to_int(l, 2)};
    quatf_t *rot = luasys_to_quat(l, 3);

    ct_transform_api_v0.set_rotation(w, t, *rot);
    return 0;
}

static int _transform_get_world_matrix(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    ct_transform_t t = {.idx = luasys_to_int(l, 2)};

    mat44f_t *wm = ct_transform_api_v0.get_world_matrix(w, t);

    luasys_push_mat44f(l, *wm);
    return 1;
}


static int _transform_link(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t root = {.h = luasys_to_handler(l, 2)};
    entity_t child = {.h = luasys_to_handler(l, 3)};

    ct_transform_api_v0.link(w, root, child);
    return 0;
}

void _register_lua_transform_api(struct ct_api_v0 *api) {
    CETECH_GET_API(api, ct_transform_api_v0);

    luasys_add_module_function(API_NAME, "get", _transform_get);
    luasys_add_module_function(API_NAME, "has", _transform_has);

    luasys_add_module_function(API_NAME, "get_position",
                               _transform_get_position);
    luasys_add_module_function(API_NAME, "get_rotation",
                               _transform_get_rotation);
    luasys_add_module_function(API_NAME, "get_scale", _transform_get_scale);
    luasys_add_module_function(API_NAME, "get_world_matrix",
                               _transform_get_world_matrix);

    luasys_add_module_function(API_NAME, "set_position",
                               _transform_set_position);
    luasys_add_module_function(API_NAME, "set_rotation",
                               _transform_set_rotation);
    luasys_add_module_function(API_NAME, "set_scale", _transform_set_scale);
    luasys_add_module_function(API_NAME, "link", _transform_link);
}