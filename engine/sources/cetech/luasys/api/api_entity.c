#include <stddef.h>

#include <cetech/allocator.h>
#include <cetech/stringid.h>
#include <cetech/world.h>
#include <cetech/yaml.h>
#include <cetech/vio.h>
#include <cetech/config.h>
#include <cetech/application.h>
#include <cetech/resource.h>
#include <cetech/entity.h>
#include <cetech/module.h>
#include <cetech/handler.h>
#include "cetech/luasys.h"
#include "../luasys.h"

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


void _register_lua_entity_api(get_api_fce_t get_engine_api) {
    entity_api_v0 = *((struct entity_api_v0 *) get_engine_api(ENTITY_API_ID));

    luasys_add_module_function(API_NAME, "spawn", _entity_spawn);
    luasys_add_module_function(API_NAME, "destroy", _entity_destroy);
}