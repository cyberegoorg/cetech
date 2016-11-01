#ifndef CETECH_QUATERNION_H
#define CETECH_QUATERNION_H

#include "include/luajit/luajit.h"

#include "celib/math/quatf.h"

#include "engine/luasys/luasys.h"


static int _quat_add(lua_State *L) {
    cel_quatf_t *a = luasys_to_quat(L, 1);
    cel_quatf_t *b = luasys_to_quat(L, 2);

    cel_quatf_t res = {0};
    cel_quatf_add(&res, a, b);

    luasys_push_quat(L, res);
    return 1;
}

static int _quat_sub(lua_State *L) {
    cel_quatf_t *a = luasys_to_quat(L, 1);
    cel_quatf_t *b = luasys_to_quat(L, 2);

    cel_quatf_t res = {0};
    cel_quatf_sub(&res, a, b);

    luasys_push_quat(L, res);
    return 1;
}


//TODO: mul_s
static int _quat_mul(lua_State *L) {
    cel_quatf_t *a = luasys_to_quat(L, 1);
    cel_quatf_t *b = luasys_to_quat(L, 2);

    cel_quatf_t res = {0};
    cel_quatf_mul(&res, a, b);

    luasys_push_quat(L, res);
    return 1;
}

static int _quat_div(lua_State *L) {
    cel_quatf_t *a = luasys_to_quat(L, 1);
    float b = luasys_to_f32(L, 2);

    cel_quatf_t res = {0};
    cel_quatf_div_s(&res, a, b);

    luasys_push_vec4f(L, res);
    return 1;
}

static int _quat_unm(lua_State *L) {
    cel_quatf_t *a = luasys_to_quat(L, 1);

    cel_quatf_t res = {0};
    cel_quatf_invert(&res, a);

    luasys_push_vec4f(L, res);
    return 1;
}

static int _quat_index(lua_State *L) {
    cel_quatf_t *a = luasys_to_quat(L, 1);
    const char *s = luasys_to_string(L, 2);

    switch (s[0]) {
        case 'x':
            luasys_push_float(L, a->x);
            return 1;
        case 'y':
            luasys_push_float(L, a->y);
            return 1;
        case 'z':
            luasys_push_float(L, a->z);
            return 1;
        case 'w':
            luasys_push_float(L, a->w);
            return 1;
        default:
            log_error("lua", "Quat bad index '%c'", s[0]);
            break;
    }

    return 0;
}

static int _quat_newindex(lua_State *L) {
    cel_quatf_t *a = luasys_to_quat(L, 1);

    const char *s = luasys_to_string(L, 2);
    const float value = luasys_to_f32(L, 3);

    switch (s[0]) {
        case 'x':
            a->x = value;
            break;
        case 'y':
            a->y = value;
            break;
        case 'z':
            a->z = value;
            break;
        case 'w':
            a->w = value;
            break;
        default:
            log_error("lua", "Quat bad index '%c'", s[0]);
            break;
    }

    return 0;
}


//////


#endif //CETECH_QUATERNION_H
