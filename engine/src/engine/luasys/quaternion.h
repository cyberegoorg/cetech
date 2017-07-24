#ifndef CETECH_QUATERNION_H
#define CETECH_QUATERNION_H

#include "include/luajit/luajit.h"

#include <cetech/engine/luasys.h>


//TODO: mul_s
static int _quat_mul(lua_State *L) {
    float a[4];
    float b[4];

    luasys_to_quat(L, 1, a);
    luasys_to_quat(L, 2, b);

    float res[4];

    celib::quat_mul(res, a, b);

    luasys_push_quat(L, res);
    return 1;
}

static int _quat_unm(lua_State *L) {
    float a[4];

    luasys_to_quat(L, 1, a);

    float res[4];

    celib::quat_invert(res, a);

    luasys_push_quat(L, res);
    return 1;
}

static int _quat_index(lua_State *L) {
    float a[4];
    luasys_to_quat(L, 1, a);

    const char *s = luasys_to_string(L, 2);

    switch (s[0]) {
        case 'x':
            luasys_push_float(L, a[0]);
            return 1;
        case 'y':
            luasys_push_float(L, a[1]);
            return 1;
        case 'z':
            luasys_push_float(L, a[2]);
            return 1;
        case 'w':
            luasys_push_float(L, a[3]);
            return 1;
        default:
            ct_log_a0.error("lua", "Quat bad index '%c'", s[0]);
            break;
    }

    return 0;
}

static int _quat_newindex(lua_State *L) {
    float a[4];
    luasys_to_quat(L, 1, a);

    const char *s = luasys_to_string(L, 2);
    const float value = luasys_to_float(L, 3);

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
        case 'w':
            a[3] = value;
            break;
        default:
            ct_log_a0.error("lua", "Quat bad index '%c'", s[0]);
            break;
    }

    return 0;
}


//////


#endif //CETECH_QUATERNION_H
