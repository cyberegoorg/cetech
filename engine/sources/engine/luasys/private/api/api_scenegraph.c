
#include <engine/world/scenegraph.h>
#include <engine/module/module.h>
#include "../luasys.h"

#define API_NAME "SceneGraph"

static struct SceneGprahApiV0 SceneGprahApiV0;

static int _scenegraph_node_by_name(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t ent = {.h = luasys_to_handler(l, 2)};
    stringid64_t name = stringid64_from_string(luasys_to_string(l, 3));

    luasys_push_int(l, SceneGprahApiV0.node_by_name(w, ent, name).idx);
    return 1;
}

static int _scenegraph_has(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t ent = {.h = luasys_to_handler(l, 2)};

    luasys_push_bool(l, SceneGprahApiV0.has(w, ent));
    return 1;
}


static int _scenegraph_get_position(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t t = {.idx = luasys_to_int(l, 2)};

    luasys_push_vec3f(l, SceneGprahApiV0.get_position(w, t));
    return 1;
}

static int _scenegraph_get_rotation(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t t = {.idx = luasys_to_int(l, 2)};

    luasys_push_quat(l, SceneGprahApiV0.get_rotation(w, t));
    return 1;
}

static int _scenegraph_get_scale(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t t = {.idx = luasys_to_int(l, 2)};

    luasys_push_vec3f(l, SceneGprahApiV0.get_scale(w, t));
    return 1;
}

static int _scenegraph_set_position(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t t = {.idx = luasys_to_int(l, 2)};
    cel_vec3f_t *pos = luasys_to_vec3f(l, 3);

    SceneGprahApiV0.set_position(w, t, *pos);
    return 0;
}

static int _scenegraph_set_scale(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t t = {.idx = luasys_to_int(l, 2)};
    cel_vec3f_t *pos = luasys_to_vec3f(l, 3);

    SceneGprahApiV0.set_scale(w, t, *pos);
    return 0;
}

static int _scenegraph_set_rotation(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t t = {.idx = luasys_to_int(l, 2)};
    cel_quatf_t *rot = luasys_to_quat(l, 3);

    SceneGprahApiV0.set_rotation(w, t, *rot);
    return 0;
}

static int _scenegraph_get_world_matrix(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t t = {.idx = luasys_to_int(l, 2)};

    cel_mat44f_t *wm = SceneGprahApiV0.get_world_matrix(w, t);

    luasys_push_mat44f(l, *wm);
    return 1;
}

static int _scenegraph_link(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    scene_node_t root = {.idx = luasys_to_int(l, 2)};
    scene_node_t child = {.idx = luasys_to_int(l, 3)};

    SceneGprahApiV0.link(w, root, child);
    return 0;
}

void _register_lua_scenegraph_api(get_api_fce_t get_engine_api) {
    SceneGprahApiV0 = *((struct SceneGprahApiV0 *) get_engine_api(
            SCENEGRAPH_API_ID, 0));

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