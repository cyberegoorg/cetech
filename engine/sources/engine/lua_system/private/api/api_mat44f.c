
#include <celib/math/mat44f.h>

#include <engine/entcom/types.h>
#include "engine/lua_system/lua_system.h"

#define API_NAME "Mat44f"

static int _identity(lua_State *l) {
    luasys_push_mat44f(l, MAT44F_INIT_IDENTITY);
    return 1;
}

static int _ctor(lua_State *l) {
    vec4f_t *x = luasys_to_vec4f(l, 1);
    vec4f_t *y = luasys_to_vec4f(l, 2);
    vec4f_t *z = luasys_to_vec4f(l, 3);
    vec4f_t *w = luasys_to_vec4f(l, 4);

    luasys_push_mat44f(l, (mat44f_t) {.x=*x, .y=*y, .z=*z, .w=*w});
    return 1;
}

static int _translate(lua_State *l) {
    mat44f_t m = {0};
    vec3f_t *v = luasys_to_vec3f(l, 1);

    mat44f_translate(&m, v->x, v->y, v->z);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _scale(lua_State *l) {
    mat44f_t m = {0};
    vec3f_t *v = luasys_to_vec3f(l, 1);

    mat44f_scale(&m, v->x, v->y, v->z);

    luasys_push_mat44f(l, m);
    return 1;
}


static int _rotate_x(lua_State *l) {
    mat44f_t m = {0};
    f32 x = luasys_to_f32(l, 1);

    mat44f_rotate_x(&m, x);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _rotate_y(lua_State *l) {
    mat44f_t m = {0};
    f32 y = luasys_to_f32(l, 1);

    mat44f_rotate_y(&m, y);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _rotate_z(lua_State *l) {
    mat44f_t m = {0};
    f32 z = luasys_to_f32(l, 1);

    mat44f_rotate_z(&m, z);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _rotate_xy(lua_State *l) {
    mat44f_t m = {0};
    f32 x = luasys_to_f32(l, 1);
    f32 y = luasys_to_f32(l, 2);

    mat44f_rotate_xy(&m, x, y);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _rotate_xyz(lua_State *l) {
    mat44f_t m = {0};
    f32 x = luasys_to_f32(l, 1);
    f32 y = luasys_to_f32(l, 2);
    f32 z = luasys_to_f32(l, 3);

    mat44f_rotate_xyz(&m, x, y, z);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _rotate_zyx(lua_State *l) {
    mat44f_t m = {0};
    f32 x = luasys_to_f32(l, 1);
    f32 y = luasys_to_f32(l, 2);
    f32 z = luasys_to_f32(l, 3);

    mat44f_rotate_zyx(&m, x, y, z);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _transpose(lua_State *l) {
    mat44f_t m = {0};
    mat44f_t *a = luasys_to_mat44f(l, 1);

    mat44f_transpose(&m, a);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _perspective_fov(lua_State *l) {
    mat44f_t m = {0};
    mat44f_t *a = luasys_to_mat44f(l, 1);

    f32 fov = luasys_to_f32(l, 1);
    f32 ar = luasys_to_f32(l, 2);
    f32 near = luasys_to_f32(l, 3);
    f32 far = luasys_to_f32(l, 4);

    mat44f_set_perspective_fov(&m, fov, ar, near, far);

    luasys_push_mat44f(l, m);
    return 1;
}

void _register_lua_mat44f_api() {
    luasys_add_module_function(API_NAME, "identity", _identity);

    luasys_add_module_function(API_NAME, "make", _ctor);

    luasys_add_module_function(API_NAME, "perspective_fov", _perspective_fov);

    luasys_add_module_function(API_NAME, "translate", _translate);
    luasys_add_module_function(API_NAME, "scale", _scale);

    luasys_add_module_function(API_NAME, "rotate_x", _rotate_x);
    luasys_add_module_function(API_NAME, "rotate_y", _rotate_y);
    luasys_add_module_function(API_NAME, "rotate_z", _rotate_z);
    luasys_add_module_function(API_NAME, "rotate_xy", _rotate_xy);
    luasys_add_module_function(API_NAME, "rotate_xyz", _rotate_xyz);
    luasys_add_module_function(API_NAME, "rotate_zyx", _rotate_zyx);

    luasys_add_module_function(API_NAME, "transpose", _transpose);
}