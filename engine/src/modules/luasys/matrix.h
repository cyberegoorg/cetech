#ifndef CETECH_MATRIX_H
#define CETECH_MATRIX_H

#include "include/luajit/luajit.h"

#include <cetech/celib/mat44f.inl>

#include <cetech/modules/luasys.h>

static int _mat44f_mul(lua_State *L) {
    mat44f_t *a = luasys_to_mat44f(L, 1);
    mat44f_t *b = luasys_to_mat44f(L, 2);

    mat44f_t res = {0};
    mat44f_mul(&res, a, b);

    luasys_push_mat44f(L, res);
    return 1;
}

static int _mat44f_unm(lua_State *L) {
    mat44f_t *a = luasys_to_mat44f(L, 1);

    mat44f_t res = {0};
    mat44f_inverse(&res, a);

    luasys_push_mat44f(L, res);
    return 1;
}

static int _mat44f_index(lua_State *L) {
    mat44f_t *a = luasys_to_mat44f(L, 1);
    const char *s = luasys_to_string(L, 2);

    switch (s[0]) {
        case 'x':
            luasys_push_vec4f(L, a->x);
            return 1;

        case 'y':
            luasys_push_vec4f(L, a->y);
            return 1;

        case 'z':
            luasys_push_vec4f(L, a->z);
            return 1;

        case 'w':
            luasys_push_vec4f(L, a->w);
            return 1;

        default:
            ct_log_a0.error("lua", "Mat44f bad index '%c'", s[0]);
            break;
    }

    return 0;
}

static int _mat44f_newindex(lua_State *L) {
    mat44f_t *a = luasys_to_mat44f(L, 1);

    const char *s = luasys_to_string(L, 2);
    vec4f_t *value = luasys_to_vec4f(L, 3);

    switch (s[0]) {
        case 'x':
            a->x = *value;
            break;

        case 'y':
            a->y = *value;
            break;

        case 'z':
            a->z = *value;
            break;

        case 'w':
            a->w = *value;
            break;

        default:
            ct_log_a0.error("lua", "Mat44f bad index '%c'", s[0]);
            break;
    }

    return 0;
}


//////


#endif //CETECH_MATRIX_H
