
#include <cetech/celib/allocator.h>

#include <cetech/modules/entity.h>
#include <cetech/kernel/module.h>
#include <cetech/modules/luasys.h>
#include <cetech/kernel/api_system.h>
#include "../luasys_private.h"

#define API_NAME "World"

CETECH_DECL_API(ct_world_a0);

static int _world_create(lua_State *l) {
    struct ct_world world = ct_world_a0.create();
    luasys_push_handler(l, world.h);
    return 1;
}

static int _world_destroy(lua_State *l) {
    struct ct_world world = {.h = luasys_to_handler(l, 1)};
    ct_world_a0.destroy(world);
    return 0;
}

static int _world_update(lua_State *l) {
    struct ct_world world = {.h = luasys_to_handler(l, 1)};
    float dt = luasys_to_float(l, 2);
    ct_world_a0.update(world, dt);
    return 0;
}


void _register_lua_world_api(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_world_a0);

    luasys_add_module_function(API_NAME, "create", _world_create);
    luasys_add_module_function(API_NAME, "destroy", _world_destroy);
    luasys_add_module_function(API_NAME, "update", _world_update);
}