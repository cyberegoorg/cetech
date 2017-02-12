
#include <engine/world/level.h>
#include <engine/plugin/plugin.h>

#include "../luasys.h"
#include <engine/plugin/plugin_api.h>

#define API_NAME "Level"

static struct LevelApiV1 LevelApiV1;

static int _load_level(lua_State *l) {
    world_t world = {.h = luasys_to_handler(l, 1)};
    stringid64_t name = stringid64_from_string(luasys_to_string(l, 2));


    level_t level = LevelApiV1.load_level(world, name);

    luasys_push_int(l, level.idx);
    return 1;
}

static int _destroy_level(lua_State *l) {
    world_t world = {.h = luasys_to_handler(l, 1)};
    level_t level = {.idx = luasys_to_int(l, 2)};

    LevelApiV1.destroy(world, level);
    return 0;
}

static int _level_unit_by_id(lua_State *l) {
    level_t level = {.idx = luasys_to_int(l, 1)};
    stringid64_t name = stringid64_from_string(luasys_to_string(l, 2));

    entity_t ent = LevelApiV1.unit_by_id(level, name);

    luasys_push_int(l, ent.idx);
    return 1;
}

static int _level_unit(lua_State *l) {
    level_t level = {.idx = luasys_to_int(l, 1)};

    entity_t ent = LevelApiV1.unit(level);

    luasys_push_int(l, ent.idx);
    return 1;
}

void _register_lua_level_api(get_api_fce_t get_engine_api) {
    LevelApiV1 = *((struct LevelApiV1 *) get_engine_api(LEVEL_API_ID, 0));

    luasys_add_module_function(API_NAME, "load_level", _load_level);
    luasys_add_module_function(API_NAME, "destroy", _destroy_level);

    luasys_add_module_function(API_NAME, "unit_by_id", _level_unit_by_id);
    luasys_add_module_function(API_NAME, "unit", _level_unit);
}