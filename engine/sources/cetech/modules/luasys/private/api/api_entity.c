

#include <cetech/core/allocator.h>
#include <cetech/core/hash.h>
#include <cetech/kernel/world.h>
#include <cetech/kernel/resource.h>
#include <cetech/kernel/entity.h>
#include <cetech/core/module.h>
#include <cetech/modules/luasys/luasys.h>
#include <cetech/core/api.h>

#define API_NAME "Entity"

IMPORT_API(entity_api_v0);

static int _entity_spawn(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    const char *name = luasys_to_string(l, 2);

    stringid64_t nameid = stringid64_from_string(name);

    luasys_push_handler(l, entity_api_v0.spawn(w, nameid).h);
    return 1;
}

static int _entity_destroy(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t entity = {.h = luasys_to_handler(l, 2)};

    entity_api_v0.destroy(w, &entity, 1);
    return 1;
}


void _register_lua_entity_api( struct api_v0* api) {
    USE_API(api, entity_api_v0);

    luasys_add_module_function(API_NAME, "spawn", _entity_spawn);
    luasys_add_module_function(API_NAME, "destroy", _entity_destroy);
}