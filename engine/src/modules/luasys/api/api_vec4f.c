
#include "cetech/modules/transform.h"
#include "../luasys_private.h"
#include <cetech/celib/vec4f.inl>
#include <cetech/kernel/api_system.h>


#define API_NAME "Vec4f"


static int _ctor(lua_State *l) {
    float x = luasys_to_float(l, 1);
    float y = luasys_to_float(l, 2);
    float z = luasys_to_float(l, 3);
    float w = luasys_to_float(l, 4);

    luasys_push_vec4f(l, (vec4f_t) {.x=x, .y=y, .z=z, .w=w});
    return 1;
}

static int _unit_x(lua_State *l) {
    luasys_push_vec4f(l, VEC4F_UNIT_X);
    return 1;
}

static int _unit_y(lua_State *l) {
    luasys_push_vec4f(l, VEC4F_UNIT_Y);
    return 1;
}

static int _unit_z(lua_State *l) {
    luasys_push_vec4f(l, VEC4F_UNIT_Z);
    return 1;
}

static int _unit_w(lua_State *l) {
    luasys_push_vec4f(l, VEC4F_UNIT_W);
    return 1;
}

static int _length(lua_State *l) {
    vec4f_t *v = luasys_to_vec4f(l, 1);
    luasys_push_float(l, vec4f_length(v));
    return 1;
}

static int _length_squared(lua_State *l) {
    vec4f_t *v = luasys_to_vec4f(l, 1);
    luasys_push_float(l, vec4f_length_squared(v));
    return 1;
}

static int _normalized(lua_State *l) {
    vec4f_t *v = luasys_to_vec4f(l, 1);
    vec4f_t res = {0};

    vec4f_normalized(&res, v);

    luasys_push_vec4f(l, res);
    return 1;
}

static int _lerp(lua_State *l) {
    vec4f_t *from = luasys_to_vec4f(l, 1);
    vec4f_t *to = luasys_to_vec4f(l, 2);
    float time = luasys_to_float(l, 3);

    vec4f_t res = {0};

    vec4f_lerp(&res, from, to, time);

    luasys_push_vec4f(l, res);
    return 1;
}


static int _dot(lua_State *l) {
    vec4f_t *a = luasys_to_vec4f(l, 1);
    vec4f_t *b = luasys_to_vec4f(l, 2);

    luasys_push_float(l, vec4f_dot(a, b));
    return 1;
}

void _register_lua_vec4f_api(struct ct_api_a0 *api) {
    luasys_add_module_function(API_NAME, "make", _ctor);

    luasys_add_module_function(API_NAME, "unit_x", _unit_x);
    luasys_add_module_function(API_NAME, "unit_y", _unit_y);
    luasys_add_module_function(API_NAME, "unit_z", _unit_z);
    luasys_add_module_function(API_NAME, "unit_w", _unit_w);

    luasys_add_module_function(API_NAME, "length", _length);
    luasys_add_module_function(API_NAME, "length_squared", _length_squared);

    luasys_add_module_function(API_NAME, "normalized", _normalized);

    luasys_add_module_function(API_NAME, "dot", _dot);

    luasys_add_module_function(API_NAME, "lerp", _lerp);
}