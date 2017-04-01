
#include <engine/transform/transform.h>
#include <celib/math/vec3f.h>
#include "../luasys.h"
#include <engine/module/module.h>

#define API_NAME "Vec3f"

static int _ctor(lua_State *l) {
    f32 x = luasys_to_f32(l, 1);
    f32 y = luasys_to_f32(l, 2);
    f32 z = luasys_to_f32(l, 3);

    luasys_push_vec3f(l, (cel_vec3f_t) {.x=x, .y=y, .z=z});
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
    cel_vec3f_t *v = luasys_to_vec3f(l, 1);
    luasys_push_float(l, cel_vec3f_length(v));
    return 1;
}

static int _length_squared(lua_State *l) {
    cel_vec3f_t *v = luasys_to_vec3f(l, 1);
    luasys_push_float(l, cel_vec3f_length_squared(v));
    return 1;
}

static int _normalized(lua_State *l) {
    cel_vec3f_t *v = luasys_to_vec3f(l, 1);
    cel_vec3f_t res = {0};

    cel_vec3f_normalized(&res, v);

    luasys_push_vec3f(l, res);
    return 1;
}

static int _lerp(lua_State *l) {
    cel_vec3f_t *from = luasys_to_vec3f(l, 1);
    cel_vec3f_t *to = luasys_to_vec3f(l, 2);
    f32 time = luasys_to_f32(l, 3);

    cel_vec3f_t res = {0};

    cel_vec3f_lerp(&res, from, to, time);

    luasys_push_vec3f(l, res);
    return 1;
}

static int _cross(lua_State *l) {
    cel_vec3f_t *a = luasys_to_vec3f(l, 1);
    cel_vec3f_t *b = luasys_to_vec3f(l, 2);

    cel_vec3f_t res = {0};

    cel_vec3f_cross(&res, a, b);

    luasys_push_vec3f(l, res);
    return 1;
}

static int _dot(lua_State *l) {
    cel_vec3f_t *a = luasys_to_vec3f(l, 1);
    cel_vec3f_t *b = luasys_to_vec3f(l, 2);

    luasys_push_float(l, cel_vec3f_dot(a, b));
    return 1;
}

void _register_lua_vec3f_api(get_api_fce_t get_engine_api) {
    luasys_add_module_function(API_NAME, "make", _ctor);

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