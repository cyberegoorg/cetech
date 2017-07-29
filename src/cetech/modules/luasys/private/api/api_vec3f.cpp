

#include <cetech/modules/luasys/luasys.h>
#include <cetech/modules/luasys/private/luasys_private.h>
#include "cetech/core/api/api_system.h"
#include "celib/fpumath.h"

#define API_NAME "Vec3f"

#define VEC3F_UNIT_X ((float[3]){1.0f, 0.0f, 0.f})
#define VEC3F_UNIT_Y ((float[3]){0.0f, 1.0f, 0.f})
#define VEC3F_UNIT_Z ((float[3]){0.0f, 0.0f, 1.f})

static int _ctor(lua_State *l) {
    float x = luasys_to_float(l, 1);
    float y = luasys_to_float(l, 2);
    float z = luasys_to_float(l, 3);

    luasys_push_vec3f(l, (float[3]) {x, y, z});
    return 1;
}

static int _is(lua_State *l) {
    luasys_push_bool(l, _is_vec3f(l, 1));
    return 1;
}

static int _unit_x(lua_State *l) {
    luasys_push_vec3f(l, VEC3F_UNIT_X);
    return 1;
}

static int _unit_y(lua_State *l) {
    luasys_push_vec3f(l, VEC3F_UNIT_Y);
    return 1;
}

static int _unit_z(lua_State *l) {
    luasys_push_vec3f(l, VEC3F_UNIT_Z);
    return 1;
}

static int _length(lua_State *l) {
    float v[3];

    luasys_to_vec3f(l, 1, v);
    luasys_push_float(l, celib::vec3_length(v));

    return 1;
}

static int _length_squared(lua_State *l) {
    float v[3];

    luasys_to_vec3f(l, 1, v);
    luasys_push_float(l, celib::vec3_dot(v, v));

    return 1;
}

static int _normalized(lua_State *l) {
    float v[3];
    float v_norm[3];

    luasys_to_vec3f(l, 1, v);

    celib::vec3_norm(v_norm, v);

    luasys_push_vec3f(l, v_norm);

    return 1;
}

static int _lerp(lua_State *l) {
    float from[3];
    float to[3];

    luasys_to_vec3f(l, 1, from);
    luasys_to_vec3f(l, 2, to);

    float time = luasys_to_float(l, 3);

    float res[3];

    celib::vec3_lerp(res, from, to, time);

    luasys_push_vec3f(l, res);
    return 1;
}

static int _cross(lua_State *l) {
    float a[3];
    float b[3];

    luasys_to_vec3f(l, 1, a);
    luasys_to_vec3f(l, 2, b);

    float res[3];

    celib::vec3_cross(res, a, b);

    luasys_push_vec3f(l, res);
    return 1;
}

static int _dot(lua_State *l) {
    float a[3];
    float b[3];

    luasys_to_vec3f(l, 1, a);
    luasys_to_vec3f(l, 2, b);

    luasys_push_float(l, celib::vec3_dot(a, b));
    return 1;
}

void _register_lua_vec3f_api(struct ct_api_a0 *api) {
    CEL_UNUSED(api);

    luasys_add_module_function(API_NAME, "make", _ctor);
    luasys_add_module_function(API_NAME, "is", _is);

    luasys_add_module_function(API_NAME, "unit_x", _unit_x);
    luasys_add_module_function(API_NAME, "unit_y", _unit_y);
    luasys_add_module_function(API_NAME, "unit_z", _unit_z);

    luasys_add_module_function(API_NAME, "length", _length);
    luasys_add_module_function(API_NAME, "length_squared", _length_squared);

    luasys_add_module_function(API_NAME, "normalized", _normalized);

    luasys_add_module_function(API_NAME, "cross", _cross);
    luasys_add_module_function(API_NAME, "dot", _dot);

    luasys_add_module_function(API_NAME, "lerp", _lerp);
}