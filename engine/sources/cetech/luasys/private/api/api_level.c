#include <stddef.h>

#include <cetech/memory/allocator.h>
#include <cetech/level/level.h>
#include <cetech/application/private/module.h>

#include "../luasys.h"

#define API_NAME "Level"

static struct LevelApiV0 LevelApiV0;

static int _load_level(lua_State *l) {
    world_t world = {.h = luasys_to_handler(l, 1)};
    stringid64_t name = stringid64_from_string(luasys_to_string(l, 2));


    level_t level = LevelApiV0.load_level(world, name);

    luasys_push_int(l, level.idx);
    return 1;
}

static int _destroy_level(lua_State *l) {
    world_t world = {.h = luasys_to_handler(l, 1)};
    level_t level = {.idx = luasys_to_int(l, 2)};

    LevelApiV0.destroy(world, level);
    return 0;
}

static int _level_entity_by_id(lua_State *l) {
    level_t level = {.idx = luasys_to_int(l, 1)};
    stringid64_t name = stringid64_from_string(luasys_to_string(l, 2));

    entity_t ent = LevelApiV0.entity_by_id(level, name);

    luasys_push_int(l, ent.idx);
    return 1;
}

static int _level_entity(lua_State *l) {
    level_t level = {.idx = luasys_to_int(l, 1)};

    entity_t ent = LevelApiV0.entity(level);

    luasys_push_int(l, ent.idx);
    return 1;
}

void _register_lua_level_api(get_api_fce_t get_engine_api) {
    LevelApiV0 = *((struct LevelApiV0 *) get_engine_api(LEVEL_API_ID, 0));

    luasys_add_module_function(API_NAME, "load_level", _load_level);
    luasys_add_module_function(API_NAME, "destroy", _destroy_level);

    luasys_add_module_function(API_NAME, "entity_by_id", _level_entity_by_id);
    luasys_add_module_function(API_NAME, "entity", _level_entity);
}