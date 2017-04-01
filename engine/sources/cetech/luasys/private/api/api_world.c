
#include <cetech/world/world.h>
#include <cetech/application/private/module.h>
#include "../luasys.h"

#define API_NAME "World"

static struct WorldApiV0 WorldApiV0;

static int _world_create(lua_State *l) {
    world_t world = WorldApiV0.create();
    luasys_push_handler(l, world.h);
    return 1;
}

static int _world_destroy(lua_State *l) {
    world_t world = {.h = luasys_to_handler(l, 1)};
    WorldApiV0.destroy(world);
    return 0;
}

static int _world_update(lua_State *l) {
    world_t world = {.h = luasys_to_handler(l, 1)};
    float dt = luasys_to_f32(l, 2);
    WorldApiV0.update(world, dt);
    return 0;
}


void _register_lua_world_api(get_api_fce_t get_engine_api) {
    WorldApiV0 = *((struct WorldApiV0 *) get_engine_api(WORLD_API_ID, 0));


    luasys_add_module_function(API_NAME, "create", _world_create);
    luasys_add_module_function(API_NAME, "destroy", _world_destroy);
    luasys_add_module_function(API_NAME, "update", _world_update);
}