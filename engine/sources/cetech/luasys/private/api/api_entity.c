
#include <celib/string/stringid.h>
#include <cetech/entity/entity.h>
#include <cetech/application/private/module.h>
#include "../luasys.h"

#define API_NAME "Entity"

static struct EntitySystemApiV0 EntitySystemApiV0;

static int _entity_spawn(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    const char *name = luasys_to_string(l, 2);

    stringid64_t nameid = stringid64_from_string(name);

    luasys_push_handler(l, EntitySystemApiV0.spawn(w, nameid).h);
    return 1;
}

static int _entity_destroy(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t entity = {.h = luasys_to_handler(l, 2)};

    EntitySystemApiV0.destroy(w, &entity, 1);
    return 1;
}


void _register_lua_entity_api(get_api_fce_t get_engine_api) {
    EntitySystemApiV0 = *((struct EntitySystemApiV0 *) get_engine_api(ENTITY_API_ID, 0));

    luasys_add_module_function(API_NAME, "spawn", _entity_spawn);
    luasys_add_module_function(API_NAME, "destroy", _entity_destroy);
}