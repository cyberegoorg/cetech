#ifndef CETECH_QUATERNION_H
#define CETECH_QUATERNION_H

#include "include/luajit/luajit.h"

#include "celib/math/quatf.h"

#include "../lua_system.h"


static int quat_add(lua_State *L) {
    quatf_t *a = luasys_to_quat(L, 1);
    quatf_t *b = luasys_to_quat(L, 2);

    quatf_t res = {0};
    quatf_add(&res, a, b);

    luasys_push_quat(L, res);
    return 1;
}

static int quat_sub(lua_State *L) {
    quatf_t *a = luasys_to_quat(L, 1);
    quatf_t *b = luasys_to_quat(L, 2);

    quatf_t res = {0};
    quatf_sub(&res, a, b);

    luasys_push_quat(L, res);
    return 1;
}


//TODO: mul_s
static int quat_mul(lua_State *L) {
    quatf_t *a = luasys_to_quat(L, 1);
    quatf_t *b = luasys_to_quat(L, 2);

    quatf_t res = {0};
    quatf_mul(&res, a, b);

    luasys_push_quat(L, res);
    return 1;
}

static int quat_div(lua_State *L) {
    quatf_t *a = luasys_to_quat(L, 1);
    float b = luasys_to_float(L, 2);

    quatf_t res = {0};
    quatf_div_s(&res, a, b);

    luasys_push_vector4(L, res);
    return 1;
}

static int quat_unm(lua_State *L) {
    quatf_t *a = luasys_to_quat(L, 1);

    quatf_t res = {0};
    quatf_invert(&res, a);

    luasys_push_vector4(L, res);
    return 1;
}

static int quat_index(lua_State *L) {
    quatf_t *a = luasys_to_quat(L, 1);
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

static int quat_newindex(lua_State *L) {
    quatf_t *a = luasys_to_quat(L, 1);

    const char *s = luasys_to_string(L, 2);
    const float value = luasys_to_float(L, 3);

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

void create_quat_mt(lua_State *l) {
    luaL_newmetatable(l, "quat_mt");

    lua_pushstring(l, "__add");
    lua_pushcfunction(l, quat_add);
    lua_settable(l, 1);

    lua_pushstring(l, "__sub");
    lua_pushcfunction(l, quat_sub);
    lua_settable(l, 1);

    lua_pushstring(l, "__mul");
    lua_pushcfunction(l, quat_mul);
    lua_settable(l, 1);

    lua_pushstring(l, "__div");
    lua_pushcfunction(l, quat_div);
    lua_settable(l, 1);

    lua_pushstring(l, "__unm");
    lua_pushcfunction(l, quat_unm);
    lua_settable(l, 1);

    lua_pushstring(l, "__index");
    lua_pushcfunction(l, quat_index);
    lua_settable(l, 1);

    lua_pushstring(l, "__newindex");
    lua_pushcfunction(l, quat_newindex);
    lua_settable(l, 1);

    lua_pop(l, 1);
}
//////


#endif //CETECH_QUATERNION_H
