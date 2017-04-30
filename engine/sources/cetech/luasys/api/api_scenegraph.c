
#include <cetech/scenegraph.h>
#include <cetech/application/module.h>
#include "cetech/luasys/luasys.h"

#define API_NAME "SceneGraph"

IMPORT_API(scenegprah_api_v0);

static int _scenegraph_node_by_name(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t ent = {.h = luasys_to_handler(l, 2)};
    stringid64_t name = stringid64_from_string(luasys_to_string(l, 3));

    luasys_push_int(l, scenegprah_api_v0.node_by_name(w, ent, name).idx);
    return 1;
}

static int _scenegraph_has(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t ent = {.h = luasys_to_handler(l, 2)};

    luasys_push_bool(l, scenegprah_api_v0.has(w, ent));
    return 1;
}


static int _scenegraph_get_position(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t t = {.idx = luasys_to_int(l, 2)};

    luasys_push_vec3f(l, scenegprah_api_v0.get_position(w, t));
    return 1;
}

static int _scenegraph_get_rotation(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t t = {.idx = luasys_to_int(l, 2)};

    luasys_push_quat(l, scenegprah_api_v0.get_rotation(w, t));
    return 1;
}

static int _scenegraph_get_scale(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t t = {.idx = luasys_to_int(l, 2)};

    luasys_push_vec3f(l, scenegprah_api_v0.get_scale(w, t));
    return 1;
}

static int _scenegraph_set_position(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t t = {.idx = luasys_to_int(l, 2)};
    vec3f_t *pos = luasys_to_vec3f(l, 3);

    scenegprah_api_v0.set_position(w, t, *pos);
    return 0;
}

static int _scenegraph_set_scale(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t t = {.idx = luasys_to_int(l, 2)};
    vec3f_t *pos = luasys_to_vec3f(l, 3);

    scenegprah_api_v0.set_scale(w, t, *pos);
    return 0;
}

static int _scenegraph_set_rotation(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t t = {.idx = luasys_to_int(l, 2)};
    quatf_t *rot = luasys_to_quat(l, 3);

    scenegprah_api_v0.set_rotation(w, t, *rot);
    return 0;
}

static int _scenegraph_get_world_matrix(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t t = {.idx = luasys_to_int(l, 2)};

    mat44f_t *wm = scenegprah_api_v0.get_world_matrix(w, t);

    luasys_push_mat44f(l, *wm);
    return 1;
}

static int _scenegraph_link(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t root = {.idx = luasys_to_int(l, 2)};
    scene_node_t child = {.idx = luasys_to_int(l, 3)};

    scenegprah_api_v0.link(w, root, child);
    return 0;
}

void _register_lua_scenegraph_api(get_api_fce_t get_engine_api) {
    scenegprah_api_v0 = *((struct scenegprah_api_v0 *) get_engine_api(
            SCENEGRAPH_API_ID));

    luasys_add_module_function(API_NAME, "has", _scenegraph_has);
    luasys_add_module_function(API_NAME, "node_by_name",
                               _scenegraph_node_by_name);

    luasys_add_module_function(API_NAME, "get_position",
                               _scenegraph_get_position);
    luasys_add_module_function(API_NAME, "get_rotation",
                               _scenegraph_get_rotation);
    luasys_add_module_function(API_NAME, "get_scale", _scenegraph_get_scale);
    luasys_add_module_function(API_NAME, "get_world_matrix",
                               _scenegraph_get_world_matrix);

    luasys_add_module_function(API_NAME, "set_position",
                               _scenegraph_set_position);
    luasys_add_module_function(API_NAME, "set_rotation",
                               _scenegraph_set_rotation);
    luasys_add_module_function(API_NAME, "set_scale", _scenegraph_set_scale);
}