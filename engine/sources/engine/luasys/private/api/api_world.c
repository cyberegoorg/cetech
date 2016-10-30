
#include <engine/world/world.h>
#include <celib/string/stringid.h>
#include "engine/luasys/luasys.h"

#define API_NAME "World"

static int _world_create(lua_State *l) {
    world_t world = world_create();
    luasys_push_handler(l, world.h);
    return 1;
}

static int _world_destroy(lua_State *l) {
    world_t world = {.h = luasys_to_handler(l, 1)};
    world_destroy(world);
    return 0;
}

static int _world_update(lua_State *l) {
    world_t world = {.h = luasys_to_handler(l, 1)};
    float dt = luasys_to_f32(l, 2);
    world_update(world, dt);
    return 0;
}



void _register_lua_world_api() {
    luasys_add_module_function(API_NAME, "create", _world_create);
    luasys_add_module_function(API_NAME, "destroy", _world_destroy);
    luasys_add_module_function(API_NAME, "update", _world_update);
}