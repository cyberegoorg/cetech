

#include <cetech/core/allocator.h>
#include <cetech/modules/world/world.h>
#include <cetech/modules/resource/resource.h>
#include <cetech/modules/entity/entity.h>
#include <cetech/kernel/module.h>

#include <cetech/modules/luasys/luasys.h>
#include <cetech/kernel/hash.h>
#include <cetech/kernel/api.h>

#include "../../../level/level.h"

#define API_NAME "Level"

IMPORT_API(level_api_v0);

static int _load_level(lua_State *l) {
    world_t world = {.h = luasys_to_handler(l, 1)};
    stringid64_t name = stringid64_from_string(luasys_to_string(l, 2));


    level_t level = level_api_v0.load_level(world, name);

    luasys_push_int(l, level.idx);
    return 1;
}

static int _destroy_level(lua_State *l) {
    world_t world = {.h = luasys_to_handler(l, 1)};
    level_t level = {.idx = luasys_to_int(l, 2)};

    level_api_v0.destroy(world, level);
    return 0;
}

static int _level_entity_by_id(lua_State *l) {
    level_t level = {.idx = luasys_to_int(l, 1)};
    stringid64_t name = stringid64_from_string(luasys_to_string(l, 2));

    entity_t ent = level_api_v0.entity_by_id(level, name);

    luasys_push_uint32_t(l, ent.h);
    return 1;
}

static int _level_entity(lua_State *l) {
    level_t level = {.idx = luasys_to_int(l, 1)};

    entity_t ent = level_api_v0.entity(level);

    luasys_push_uint32_t(l, ent.h);
    return 1;
}

void _register_lua_level_api( struct api_v0* api) {
    GET_API(api, level_api_v0);

    luasys_add_module_function(API_NAME, "load_level", _load_level);
    luasys_add_module_function(API_NAME, "destroy", _destroy_level);

    luasys_add_module_function(API_NAME, "entity_by_id", _level_entity_by_id);
    luasys_add_module_function(API_NAME, "entity", _level_entity);
}