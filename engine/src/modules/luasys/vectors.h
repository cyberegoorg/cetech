#ifndef CETECH_VECTORS_H
#define CETECH_VECTORS_H

#include "include/luajit/luajit.h"


#include <cetech/modules/luasys.h>
#include <cetech/celib/fpumath.h>

static int _vec3f_add(lua_State *l) {
    float a[3];
    float b[3];

    luasys_to_vec3f(l, 1, a);
    luasys_to_vec3f(l, 2, b);


    float res[3];
    celib::vec3_add(res, a, b);

    luasys_push_vec3f(l, res);
    return 1;
}

static int _vec3f_sub(lua_State *l) {
    float a[3];
    float b[3];

    luasys_to_vec3f(l, 1, a);
    luasys_to_vec3f(l, 2, b);


    float res[3];
    celib::vec3_sub(res, a, b);

    luasys_push_vec3f(l, res);
    return 1;
}

static int _vec3f_mul(lua_State *l) {
    float a[3];
    float b = luasys_to_float(l, 2);

    luasys_to_vec3f(l, 1, a);

    float res[3];
    celib::vec3_mul(res, a, b);

    luasys_push_vec3f(l, res);
    return 1;
}

static int _vec3f_div(lua_State *l) {
    float a[3];
    float b = luasys_to_float(l, 2);

    luasys_to_vec3f(l, 1, a);

    float res[3];
    celib::vec3_mul(res, a, 1.0f / b);

    luasys_push_vec3f(l, res);
    return 1;
}

static int _vec3f_unm(lua_State *l) {
    float a[3];

    luasys_to_vec3f(l, 1, a);

    float res[3];
    celib::vec3_mul(res, a, -1.0f);

    luasys_push_vec3f(l, res);
    return 1;
}

static int _vec3f_index(lua_State *l) {
    float a[3];
    const char *s = luasys_to_string(l, 2);

    luasys_to_vec3f(l, 1, a);

    switch (s[0]) {
        case 'x':
            luasys_push_float(l, a[0]);
            return 1;
        case 'y':
            luasys_push_float(l, a[1]);
            return 1;
        case 'z':
            luasys_push_float(l, a[2]);
            return 1;
        default:
            ct_log_a0.error("lua", "Vector3 bad index '%c'", s[0]);
            break;
    }

    return 0;
}

static int _vec3f_newindex(lua_State *l) {
    float a[3];
    luasys_to_vec3f(l, 1, a);

    const char *s = luasys_to_string(l, 2);
    const float value = luasys_to_float(l, 3);

    switch (s[0]) {
        case 'x':
            a[0] = value;
            break;
        case 'y':
            a[1] = value;
            break;
        case 'z':
            a[2] = value;
            break;
        default:
            ct_log_a0.error("lua", "Vector3 bad index '%c'", s[0]);
            break;
    }

    return 0;
}


#endif //CETECH_VECTORS_H
