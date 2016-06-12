#ifndef CETECH_VEC4F_H
#define CETECH_VEC4F_H

#include "types.h"

#define vec4f_make(xx, yy, zz, ww) (vec4f_t){ .x = xx, .y = yy, .z = zz, .w = ww }

static inline vec4f_t vec4f_add(vec4f_t a, vec4f_t b) {
    return vec4f_make(
            a.x + b.x,
            a.y + b.y,
            a.z + b.z,
            a.w + b.w
    );
}

static inline vec4f_t vec4f_sub(vec4f_t a, vec4f_t b) {
    return vec4f_make(
            a.x - b.x,
            a.y - b.y,
            a.z - b.z,
            a.w - b.w
    );
}

static inline vec4f_t vec4f_mul(vec4f_t a, float s) {
    return vec4f_make(
            a.x * s,
            a.y * s,
            a.z * s,
            a.w * s
    );
}

static inline vec4f_t vec4f_div(vec4f_t a, float s) {
    const float inv_s = 1.0f/s;
    return vec4f_make(
            a.x * inv_s,
            a.y * inv_s,
            a.z * inv_s,
            a.w * inv_s
    );
}

static inline vec4f_t vec4f_dot(vec4f_t a, vec4f_t b) {
    return vec4f_make(
            a.x * b.x,
            a.y * b.y,
            a.z * b.z,
            a.w * b.w
    );
}

static inline float vec4f_length_squared(vec4f_t a) {
    return (a.x * a.x) + (a.y * a.y) + (a.z * a.z) + (a.w * a.w);
}

static inline float vec4f_length(vec4f_t a) {
    return sqrtf((a.x * a.x) + (a.y * a.y) + (a.z * a.z) + (a.w * a.w));
}

static inline vec4f_t vec4f_normalized(vec4f_t a) {
    const float inv_length = 1.0f/vec4f_length(a);
    return vec4f_make(
            a.x * inv_length,
            a.y * inv_length,
            a.z * inv_length,
            a.w * inv_length
    );
}

#endif //CETECH_VEC4F_H
