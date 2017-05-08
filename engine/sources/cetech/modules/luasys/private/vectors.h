#ifndef CETECH_VECTORS_H
#define CETECH_VECTORS_H

#include "include/luajit/luajit.h"

#include "../../../core/vec2f.inl"
#include "../../../core/vec3f.inl"
#include "../../../core/vec4f.inl"

#include "luasys.h"

static int _vec2f_add(lua_State *L) {
    vec2f_t *a = luasys_to_vec2f(L, 1);
    vec2f_t *b = luasys_to_vec2f(L, 2);

    vec2f_t res = {0};
    vec2f_add(&res, a, b);

    luasys_push_vec2f(L, res);
    return 1;
}

static int _vec2f_sub(lua_State *L) {
    vec2f_t *a = luasys_to_vec2f(L, 1);
    vec2f_t *b = luasys_to_vec2f(L, 2);

    vec2f_t res = {0};
    vec2f_sub(&res, a, b);

    luasys_push_vec2f(L, res);
    return 1;
}

static int _vec2f_mul(lua_State *L) {
    vec2f_t *a = luasys_to_vec2f(L, 1);
    float b = luasys_to_float(L, 2);

    vec2f_t res = {0};
    vec2f_mul(&res, a, b);

    luasys_push_vec2f(L, res);
    return 1;
}

static int _vec2f_div(lua_State *L) {
    vec2f_t *a = luasys_to_vec2f(L, 1);
    float b = luasys_to_float(L, 2);

    vec2f_t res = {0};
    vec2f_div(&res, a, b);

    luasys_push_vec2f(L, res);
    return 1;
}

static int _vec2f_unm(lua_State *L) {
    vec2f_t *a = luasys_to_vec2f(L, 1);

    vec2f_t res = {0};
    vec2f_mul(&res, a, -1.0f);

    luasys_push_vec2f(L, res);
    return 1;
}

static int _vec2f_index(lua_State *L) {
    vec2f_t *a = luasys_to_vec2f(L, 1);
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

static int _vec2f_newindex(lua_State *L) {
    vec2f_t *a = luasys_to_vec2f(L, 1);

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

static int _vec3f_add(lua_State *L) {
    vec3f_t *a = luasys_to_vec3f(L, 1);
    vec3f_t *b = luasys_to_vec3f(L, 2);

    vec3f_t res = {0};
    vec3f_add(&res, a, b);

    luasys_push_vec3f(L, res);
    return 1;
}

static int _vec3f_sub(lua_State *L) {
    vec3f_t *a = luasys_to_vec3f(L, 1);
    vec3f_t *b = luasys_to_vec3f(L, 2);

    vec3f_t res = {0};
    vec3f_sub(&res, a, b);

    luasys_push_vec3f(L, res);
    return 1;
}

static int _vec3f_mul(lua_State *L) {
    vec3f_t *a = luasys_to_vec3f(L, 1);
    float b = luasys_to_float(L, 2);

    vec3f_t res = {0};
    vec3f_mul(&res, a, b);

    luasys_push_vec3f(L, res);
    return 1;
}

static int _vec3f_div(lua_State *L) {
    vec3f_t *a = luasys_to_vec3f(L, 1);
    float b = luasys_to_float(L, 2);

    vec3f_t res = {0};
    vec3f_div(&res, a, b);

    luasys_push_vec3f(L, res);
    return 1;
}

static int _vec3f_unm(lua_State *L) {
    vec3f_t *a = luasys_to_vec3f(L, 1);

    vec3f_t res = {0};
    vec3f_mul(&res, a, -1.0f);

    luasys_push_vec3f(L, res);
    return 1;
}

static int _vec3f_index(lua_State *L) {
    vec3f_t *a = luasys_to_vec3f(L, 1);
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

static int _vec3f_newindex(lua_State *L) {
    vec3f_t *a = luasys_to_vec3f(L, 1);

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

static int _vec4f_add(lua_State *L) {
    vec4f_t *a = luasys_to_vec4f(L, 1);
    vec4f_t *b = luasys_to_vec4f(L, 2);

    vec4f_t res = {0};
    vec4f_add(&res, a, b);

    luasys_push_vec4f(L, res);
    return 1;
}

static int _vec4f_sub(lua_State *L) {
    vec4f_t *a = luasys_to_vec4f(L, 1);
    vec4f_t *b = luasys_to_vec4f(L, 2);

    vec4f_t res = {0};
    vec4f_sub(&res, a, b);

    luasys_push_vec4f(L, res);
    return 1;
}

static int _vec4f_mul(lua_State *L) {
    vec4f_t *a = luasys_to_vec4f(L, 1);
    float b = luasys_to_float(L, 2);

    vec4f_t res = {0};
    vec4f_mul(&res, a, b);

    luasys_push_vec4f(L, res);
    return 1;
}

static int _vec4f_div(lua_State *L) {
    vec4f_t *a = luasys_to_vec4f(L, 1);
    float b = luasys_to_float(L, 2);

    vec4f_t res = {0};
    vec4f_div(&res, a, b);

    luasys_push_vec4f(L, res);
    return 1;
}

static int _vec4f_unm(lua_State *L) {
    vec4f_t *a = luasys_to_vec4f(L, 1);

    vec4f_t res = {0};
    vec4f_mul(&res, a, -1.0f);

    luasys_push_vec4f(L, res);
    return 1;
}

static int _vec4f_index(lua_State *L) {
    vec4f_t *a = luasys_to_vec4f(L, 1);
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

static int _vec4f_newindex(lua_State *L) {
    vec4f_t *a = luasys_to_vec4f(L, 1);

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

#endif //CETECH_VECTORS_H
