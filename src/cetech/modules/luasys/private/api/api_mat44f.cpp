

#include "cetech/core/api/api_system.h"
#include "celib/fpumath.h"
#include <include/luajit/lua.h>
#include <cetech/modules/luasys/private/luasys_private.h>

#define API_NAME "Mat44f"

static int _identity(lua_State *l) {
    float m[16];

    celib::mat4_identity(m);

    luasys_push_mat44f(l, m);
    return 1;
}


static int _translate(lua_State *l) {
    float m[16];
    float v[3];

    luasys_to_vec3f(l, 1, v);

    celib::mat4_translate(m, v[0], v[1], v[2]);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _scale(lua_State *l) {
    float m[16];
    float v[3];

    luasys_to_vec3f(l, 1, v);

    celib::mat4_scale(m, v[0], v[1], v[2]);

    luasys_push_mat44f(l, m);
    return 1;
}


static int _rotate_x(lua_State *l) {
    float m[16];
    float x = luasys_to_float(l, 1);

    celib::mat4_rotate_x(m, x);
    luasys_push_mat44f(l, m);

    return 1;
}

static int _rotate_y(lua_State *l) {
    float m[16];
    float x = luasys_to_float(l, 1);

    celib::mat4_rotate_y(m, x);
    luasys_push_mat44f(l, m);

    return 1;
}

static int _rotate_z(lua_State *l) {
    float m[16];
    float x = luasys_to_float(l, 1);

    celib::mat4_rotate_z(m, x);
    luasys_push_mat44f(l, m);

    return 1;
}

static int _rotate_xy(lua_State *l) {
    float m[16];

    float x = luasys_to_float(l, 1);
    float y = luasys_to_float(l, 2);

    celib::mat4_rotate_xy(m, x, y);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _rotate_xyz(lua_State *l) {
    float m[16];

    float x = luasys_to_float(l, 1);
    float y = luasys_to_float(l, 2);
    float z = luasys_to_float(l, 3);

    celib::mat4_rotate_xyz(m, x, y, z);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _rotate_zyx(lua_State *l) {
    float m[16];

    float x = luasys_to_float(l, 1);
    float y = luasys_to_float(l, 2);
    float z = luasys_to_float(l, 3);

    celib::mat4_rotate_zyx(m, x, y, z);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _transpose(lua_State *l) {
    float m[16];
    float res[16];


    luasys_to_mat44f(l, 1, m);

    celib::mat4_transpose(res, m);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _perspective_fov(lua_State *l) {
    float m[16];

    float fov = luasys_to_float(l, 1);
    float ar = luasys_to_float(l, 2);
    float near = luasys_to_float(l, 3);
    float far = luasys_to_float(l, 4);

    celib::mat4_proj(m, fov, ar, near, far, true);

    luasys_push_mat44f(l, m);
    return 1;
}

static int _ctor(lua_State *l) {
    float m[16];

    for (int i = 0; i < 16; ++i) {
        m[i] = luasys_to_float(l, i + 1);
    }

    luasys_push_mat44f(l, m);
    return 1;
}

void _register_lua_mat44f_api(struct ct_api_a0 *api) {
    CEL_UNUSED(api);

    luasys_add_module_function(API_NAME, "make", _ctor);

    luasys_add_module_function(API_NAME, "identity", _identity);

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