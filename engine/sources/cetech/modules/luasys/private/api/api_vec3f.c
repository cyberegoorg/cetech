
#include "../../../transform/transform.h"
#include <cetech/core/math/vec3f.inl>
#include <cetech/core/module.h>

#include <cetech/modules/luasys/luasys.h>

#define API_NAME "Vec3f"

static int _ctor(lua_State *l) {
    float x = luasys_to_float(l, 1);
    float y = luasys_to_float(l, 2);
    float z = luasys_to_float(l, 3);

    luasys_push_vec3f(l, (vec3f_t) {.x=x, .y=y, .z=z});
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
    vec3f_t *v = luasys_to_vec3f(l, 1);
    luasys_push_float(l, vec3f_length(v));
    return 1;
}

static int _length_squared(lua_State *l) {
    vec3f_t *v = luasys_to_vec3f(l, 1);
    luasys_push_float(l, vec3f_length_squared(v));
    return 1;
}

static int _normalized(lua_State *l) {
    vec3f_t *v = luasys_to_vec3f(l, 1);
    vec3f_t res = {0};

    vec3f_normalized(&res, v);

    luasys_push_vec3f(l, res);
    return 1;
}

static int _lerp(lua_State *l) {
    vec3f_t *from = luasys_to_vec3f(l, 1);
    vec3f_t *to = luasys_to_vec3f(l, 2);
    float time = luasys_to_float(l, 3);

    vec3f_t res = {0};

    vec3f_lerp(&res, from, to, time);

    luasys_push_vec3f(l, res);
    return 1;
}

static int _cross(lua_State *l) {
    vec3f_t *a = luasys_to_vec3f(l, 1);
    vec3f_t *b = luasys_to_vec3f(l, 2);

    vec3f_t res = {0};

    vec3f_cross(&res, a, b);

    luasys_push_vec3f(l, res);
    return 1;
}

static int _dot(lua_State *l) {
    vec3f_t *a = luasys_to_vec3f(l, 1);
    vec3f_t *b = luasys_to_vec3f(l, 2);

    luasys_push_float(l, vec3f_dot(a, b));
    return 1;
}

void _register_lua_vec3f_api(struct api_v0 *api) {
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