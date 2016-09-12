#include <engine/application/application.h>
#include <engine/entcom/types.h>
#include <celib/stringid/stringid.h>
#include <engine/world_system/unit_system.h>
#include "engine/lua_system/lua_system.h"

#define API_NAME "Unit"

static int _unit_spawn(lua_State *l) {
    world_t w = {.h = luasys_to_handler(l, 1)};
    const char *name = luasys_to_string(l, 2);

    stringid64_t nameid = stringid64_from_string(name);

    luasys_push_handler(l, unit_spawn(w, nameid).h);
    return 1;
}


void _register_lua_unit_api() {
    luasys_add_module_function(API_NAME, "spawn", _unit_spawn);
}