#ifndef CETECH_VECTORS_H
#define CETECH_VECTORS_H

#include "include/luajit/luajit.h"

#include "celib/math/vec2f.h"
#include "celib/math/vec3f.h"
#include "celib/math/vec4f.h"

#include "../lua_system.h"

/////// VECTOR2

static int vector2_add(lua_State *L) {
    vec2f_t *a = luasys_to_vector2(L, 1);
    vec2f_t *b = luasys_to_vector2(L, 2);

    vec2f_t res = {0};
    vec2f_add(&res, a, b);

    luasys_push_vector2(L, res);
    return 1;
}

static int vector2_sub(lua_State *L) {
    vec2f_t *a = luasys_to_vector2(L, 1);
    vec2f_t *b = luasys_to_vector2(L, 2);

    vec2f_t res = {0};
    vec2f_sub(&res, a, b);

    luasys_push_vector2(L, res);
    return 1;
}

static int vector2_mul(lua_State *L) {
    vec2f_t *a = luasys_to_vector2(L, 1);
    float b = luasys_to_float(L, 2);

    vec2f_t res = {0};
    vec2f_mul(&res, a, b);

    luasys_push_vector2(L, res);
    return 1;
}

static int vector2_div(lua_State *L) {
    vec2f_t *a = luasys_to_vector2(L, 1);
    float b = luasys_to_float(L, 2);

    vec2f_t res = {0};
    vec2f_div(&res, a, b);

    luasys_push_vector2(L, res);
    return 1;
}

static int vector2_unm(lua_State *L) {
    vec2f_t *a = luasys_to_vector2(L, 1);

    vec2f_t res = {0};
    vec2f_mul(&res, a, -1.0f);

    luasys_push_vector2(L, res);
    return 1;
}

static int vector2_index(lua_State *L) {
    vec2f_t *a = luasys_to_vector2(L, 1);
    const char *s = luasys_to_string(L, 2);

    switch (s[0]) {
        case 'x':
            luasys_push_float(L, a->x);
            return 1;
        case 'y':
            luasys_push_float(L, a->y);
            return 1;
        default:
            log_error("lua", "Vector2 bad index '%c'", s[0]);
            break;
    }

    return 0;
}

static int vector2_newindex(lua_State *L) {
    vec2f_t *a = luasys_to_vector2(L, 1);

    const char *s = luasys_to_string(L, 2);
    const float value = luasys_to_float(L, 3);

    switch (s[0]) {
        case 'x':
            a->x = value;
            break;
        case 'y':
            a->y = value;
            break;
        default:
            log_error("lua", "Vector2 bad index '%c'", s[0]);
            break;
    }

    return 0;
}

void create_vector2_mt(lua_State *l) {
    luaL_newmetatable(l, "vector2_mt");

    lua_pushstring(l, "__add");
    lua_pushcfunction(l, vector2_add);
    lua_settable(l, 1);

    lua_pushstring(l, "__sub");
    lua_pushcfunction(l, vector2_sub);
    lua_settable(l, 1);

    lua_pushstring(l, "__mul");
    lua_pushcfunction(l, vector2_mul);
    lua_settable(l, 1);

    lua_pushstring(l, "__div");
    lua_pushcfunction(l, vector2_div);
    lua_settable(l, 1);

    lua_pushstring(l, "__unm");
    lua_pushcfunction(l, vector2_unm);
    lua_settable(l, 1);

    lua_pushstring(l, "__index");
    lua_pushcfunction(l, vector2_index);
    lua_settable(l, 1);

    lua_pushstring(l, "__newindex");
    lua_pushcfunction(l, vector2_newindex);
    lua_settable(l, 1);

    lua_pop(l, 1);
}

//////

/////// VECTOR3


static int vector3_add(lua_State *L) {
    vec3f_t *a = luasys_to_vector3(L, 1);
    vec3f_t *b = luasys_to_vector3(L, 2);

    vec3f_t res = {0};
    vec3f_add(&res, a, b);

    luasys_push_vector3(L, res);
    return 1;
}

static int vector3_sub(lua_State *L) {
    vec3f_t *a = luasys_to_vector3(L, 1);
    vec3f_t *b = luasys_to_vector3(L, 2);

    vec3f_t res = {0};
    vec3f_sub(&res, a, b);

    luasys_push_vector3(L, res);
    return 1;
}

static int vector3_mul(lua_State *L) {
    vec3f_t *a = luasys_to_vector3(L, 1);
    float b = luasys_to_float(L, 2);

    vec3f_t res = {0};
    vec3f_mul(&res, a, b);

    luasys_push_vector3(L, res);
    return 1;
}

static int vector3_div(lua_State *L) {
    vec3f_t *a = luasys_to_vector3(L, 1);
    float b = luasys_to_float(L, 2);

    vec3f_t res = {0};
    vec3f_div(&res, a, b);

    luasys_push_vector3(L, res);
    return 1;
}

static int vector3_unm(lua_State *L) {
    vec3f_t *a = luasys_to_vector3(L, 1);

    vec3f_t res = {0};
    vec3f_mul(&res, a, -1.0f);

    luasys_push_vector3(L, res);
    return 1;
}

static int vector3_index(lua_State *L) {
    vec3f_t *a = luasys_to_vector3(L, 1);
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
        default:
            log_error("lua", "Vector3 bad index '%c'", s[0]);
            break;
    }

    return 0;
}

static int vector3_newindex(lua_State *L) {
    vec3f_t *a = luasys_to_vector3(L, 1);

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
        default:
            log_error("lua", "Vector3 bad index '%c'", s[0]);
            break;
    }

    return 0;
}

void create_vector3_mt(lua_State *l) {
    luaL_newmetatable(l, "vector3_mt");

    lua_pushstring(l, "__add");
    lua_pushcfunction(l, vector3_add);
    lua_settable(l, 1);

    lua_pushstring(l, "__sub");
    lua_pushcfunction(l, vector3_sub);
    lua_settable(l, 1);

    lua_pushstring(l, "__mul");
    lua_pushcfunction(l, vector3_mul);
    lua_settable(l, 1);

    lua_pushstring(l, "__div");
    lua_pushcfunction(l, vector3_div);
    lua_settable(l, 1);

    lua_pushstring(l, "__unm");
    lua_pushcfunction(l, vector3_unm);
    lua_settable(l, 1);

    lua_pushstring(l, "__index");
    lua_pushcfunction(l, vector3_index);
    lua_settable(l, 1);

    lua_pushstring(l, "__newindex");
    lua_pushcfunction(l, vector3_newindex);
    lua_settable(l, 1);

    lua_pop(l, 1);
}

//////

/////// VECTOR4


static int vector4_add(lua_State *L) {
    vec4f_t *a = luasys_to_vector4(L, 1);
    vec4f_t *b = luasys_to_vector4(L, 2);

    vec4f_t res = {0};
    vec4f_add(&res, a, b);

    luasys_push_vector4(L, res);
    return 1;
}

static int vector4_sub(lua_State *L) {
    vec4f_t *a = luasys_to_vector4(L, 1);
    vec4f_t *b = luasys_to_vector4(L, 2);

    vec4f_t res = {0};
    vec4f_sub(&res, a, b);

    luasys_push_vector4(L, res);
    return 1;
}

static int vector4_mul(lua_State *L) {
    vec4f_t *a = luasys_to_vector4(L, 1);
    float b = luasys_to_float(L, 2);

    vec4f_t res = {0};
    vec4f_mul(&res, a, b);

    luasys_push_vector4(L, res);
    return 1;
}

static int vector4_div(lua_State *L) {
    vec4f_t *a = luasys_to_vector4(L, 1);
    float b = luasys_to_float(L, 2);

    vec4f_t res = {0};
    vec4f_div(&res, a, b);

    luasys_push_vector4(L, res);
    return 1;
}

static int vector4_unm(lua_State *L) {
    vec4f_t *a = luasys_to_vector4(L, 1);

    vec4f_t res = {0};
    vec4f_mul(&res, a, -1.0f);

    luasys_push_vector4(L, res);
    return 1;
}

static int vector4_index(lua_State *L) {
    vec4f_t *a = luasys_to_vector4(L, 1);
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
            log_error("lua", "Vector4 bad index '%c'", s[0]);
            break;
    }

    return 0;
}

static int vector4_newindex(lua_State *L) {
    vec4f_t *a = luasys_to_vector4(L, 1);

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
            log_error("lua", "Vector4 bad index '%c'", s[0]);
            break;
    }

    return 0;
}

void create_vector4_mt(lua_State *l) {
    luaL_newmetatable(l, "vector4_mt");

    lua_pushstring(l, "__add");
    lua_pushcfunction(l, vector4_add);
    lua_settable(l, 1);

    lua_pushstring(l, "__sub");
    lua_pushcfunction(l, vector4_sub);
    lua_settable(l, 1);

    lua_pushstring(l, "__mul");
    lua_pushcfunction(l, vector4_mul);
    lua_settable(l, 1);

    lua_pushstring(l, "__div");
    lua_pushcfunction(l, vector4_div);
    lua_settable(l, 1);

    lua_pushstring(l, "__unm");
    lua_pushcfunction(l, vector4_unm);
    lua_settable(l, 1);

    lua_pushstring(l, "__index");
    lua_pushcfunction(l, vector4_index);
    lua_settable(l, 1);

    lua_pushstring(l, "__newindex");
    lua_pushcfunction(l, vector4_newindex);
    lua_settable(l, 1);

    lua_pop(l, 1);
}
//////

#endif //CETECH_VECTORS_H
