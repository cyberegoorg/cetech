
#include <cetech/core/allocator.h>

#include <cetech/modules/world/world.h>
#include <cetech/kernel/module.h>
#include <cetech/modules/luasys/luasys.h>
#include <cetech/kernel/api.h>

#define API_NAME "World"

IMPORT_API(world_api_v0);

static int _world_create(lua_State *l) {
    world_t world = world_api_v0.create();
    luasys_push_handler(l, world.h);
    return 1;
}

static int _world_destroy(lua_State *l) {
    world_t world = {.h = luasys_to_handler(l, 1)};
    world_api_v0.destroy(world);
    return 0;
}

static int _world_update(lua_State *l) {
    world_t world = {.h = luasys_to_handler(l, 1)};
    float dt = luasys_to_float(l, 2);
    world_api_v0.update(world, dt);
    return 0;
}


void _register_lua_world_api( struct api_v0* api) {
    GET_API(api, world_api_v0);

    luasys_add_module_function(API_NAME, "create", _world_create);
    luasys_add_module_function(API_NAME, "destroy", _world_destroy);
    luasys_add_module_function(API_NAME, "update", _world_update);
}