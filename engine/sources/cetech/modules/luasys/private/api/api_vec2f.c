
#include <cetech/core/math/vec2f.inl>
#include <cetech/core/module.h>

#include <cetech/modules/luasys/luasys.h>


#define API_NAME "Vec2f"

static int _ctor(lua_State *l) {
    float x = luasys_to_float(l, 1);
    float y = luasys_to_float(l, 2);

    luasys_push_vec2f(l, (vec2f_t) {.x=x, .y=y});
    return 1;
}

static int _unit_x(lua_State *l) {
    luasys_push_vec2f(l, VEC2F_UNIT_X);
    return 1;
}

static int _unit_y(lua_State *l) {
    luasys_push_vec2f(l, VEC2F_UNIT_Y);
    return 1;
}

static int _length(lua_State *l) {
    vec2f_t *v = luasys_to_vec2f(l, 1);
    luasys_push_float(l, vec2f_length(v));
    return 1;
}

static int _length_squared(lua_State *l) {
    vec2f_t *v = luasys_to_vec2f(l, 1);
    luasys_push_float(l, vec2f_length_squared(v));
    return 1;
}

static int _normalized(lua_State *l) {
    vec2f_t *v = luasys_to_vec2f(l, 1);
    vec2f_t res = {0};

    vec2f_normalized(&res, v);

    luasys_push_vec2f(l, res);
    return 1;
}

static int _lerp(lua_State *l) {
    vec2f_t *from = luasys_to_vec2f(l, 1);
    vec2f_t *to = luasys_to_vec2f(l, 2);
    float time = luasys_to_float(l, 3);

    vec2f_t res = {0};

    vec2f_lerp(&res, from, to, time);

    luasys_push_vec2f(l, res);
    return 1;
}


static int _dot(lua_State *l) {
    vec2f_t *a = luasys_to_vec2f(l, 1);
    vec2f_t *b = luasys_to_vec2f(l, 2);

    luasys_push_float(l, vec2f_dot(a, b));
    return 1;
}

void _register_lua_vec2f_api( struct api_v0* api) {
    luasys_add_module_function(API_NAME, "make", _ctor);

    luasys_add_module_function(API_NAME, "unit_x", _unit_x);
    luasys_add_module_function(API_NAME, "unit_y", _unit_y);

    luasys_add_module_function(API_NAME, "length", _length);
    luasys_add_module_function(API_NAME, "length_squared", _length_squared);

    luasys_add_module_function(API_NAME, "normalized", _normalized);

    luasys_add_module_function(API_NAME, "dot", _dot);

    luasys_add_module_function(API_NAME, "lerp", _lerp);
}