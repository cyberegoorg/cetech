
#include <engine/entcom/types.h>
#include <engine/world_system/transform.h>
#include <celib/math/vec3f.h>
#include "engine/lua_system/lua_system.h"

#define API_NAME "Vec3f"

static int _unit_x(lua_State *l) {
    luasys_push_vector3(l, VEC3F_UNIT_X);
    return 1;
}

static int _unit_y(lua_State *l) {
    luasys_push_vector3(l, VEC3F_UNIT_Y);
    return 1;
}

static int _unit_z(lua_State *l) {
    luasys_push_vector3(l, VEC3F_UNIT_Z);
    return 1;
}

void _register_lua_vec3f_api() {
    luasys_add_module_function(API_NAME, "unit_x", _unit_x);
    luasys_add_module_function(API_NAME, "unit_y", _unit_y);
    luasys_add_module_function(API_NAME, "unit_z", _unit_z);
}