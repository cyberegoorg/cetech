#ifndef CETECH_MATRIX_H
#define CETECH_MATRIX_H

#include "include/luajit/luajit.h"
#include "luasys_private.h"

#include <cetech/luasys.h>
#include <celib/fpumath.h>

static int _mat44f_mul(lua_State *l) {
    float a[16];
    float b[16];

    luasys_to_mat44f(l, 1, a);
    luasys_to_mat44f(l, 2, b);

    float res[16];
    celib::mat4_mul(res, a, b);

    luasys_push_mat44f(l, res);
    return 1;
}

static int _mat44f_unm(lua_State *l) {
    float a[16];

    luasys_to_mat44f(l, 1, a);

    float res[16];
    celib::mat4_inverse(res, a);

    luasys_push_mat44f(l, res);
    return 1;
}

static int _mat44f_index(lua_State *l) {
    float a[16];
    const char *s = luasys_to_string(l, 2);

    luasys_to_mat44f(l, 1, a);

    switch (s[0]) {
        case 'x':
            luasys_push_vec3f(l, &a[0*4]);
            return 1;

        case 'y':
            luasys_push_vec3f(l, &a[1*4]);
            return 1;

        case 'z':
            luasys_push_vec3f(l, &a[2*4]);
            return 1;

        case 'w':
            luasys_push_vec3f(l, &a[3*4]);
            return 1;

        default:
            ct_log_a0.error("lua", "Mat44f bad index '%c'", s[0]);
            break;
    }

    return 0;
}
//
//static int _mat44f_newindex(lua_State *l) {
//
//}


//////


#endif //CETECH_MATRIX_H
