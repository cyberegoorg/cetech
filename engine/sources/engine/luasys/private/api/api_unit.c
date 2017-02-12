
#include <celib/string/stringid.h>
#include <engine/world/unit.h>
#include <engine/plugin/plugin.h>
#include "engine/luasys/luasys.h"
#include <engine/plugin/plugin_api.h>

#define API_NAME "Unit"

static struct UnitApiv1 UnitApiv1;

static int _unit_spawn(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    const char *name = luasys_to_string(l, 2);

    stringid64_t nameid = stringid64_from_string(name);

    luasys_push_handler(l, UnitApiv1.spawn(w, nameid).h);
    return 1;
}

static int _unit_destroy(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    entity_t unit = {.h = luasys_to_handler(l, 2)};

    UnitApiv1.destroy(w, &unit, 1);
    return 1;
}


void _register_lua_unit_api(get_api_fce_t get_engine_api) {
    UnitApiv1 = *((struct UnitApiv1*)get_engine_api(UNIT_API_ID, 0));

    luasys_add_module_function(API_NAME, "spawn", _unit_spawn);
    luasys_add_module_function(API_NAME, "destroy", _unit_destroy);
}