#ifndef CETECH_VEC3F_H
#define CETECH_VEC3F_H

#include "types.h"

#define vec3f_make(xx, yy, zz) (vec3f_t){ .x = xx, .y = yy, .z = zz }

static inline vec3f_t vec3f_add(vec3f_t a, vec3f_t b) {
    return vec3f_make(
            a.x + b.x,
            a.y + b.y,
            a.z + b.z
    );
}

static inline vec3f_t vec3f_sub(vec3f_t a, vec3f_t b) {
    return vec3f_make(
            a.x - b.x,
            a.y - b.y,
            a.z - b.z
    );
}

static inline vec3f_t vec3f_mul(vec3f_t a, float s) {
    return vec3f_make(
            a.x * s,
            a.y * s,
            a.z * s
    );
}

static inline vec3f_t vec3f_div(vec3f_t a, float s) {
    const float inv_s = 1.0f/s;
    return vec3f_make(
            a.x * inv_s,
            a.y * inv_s,
            a.z * inv_s
    );
}

static inline vec3f_t vec3f_dot(vec3f_t a, vec3f_t b) {
    return vec3f_make(
            a.x * b.x,
            a.y * b.y,
            a.z * b.z
    );
}

static inline float vec3f_length_squared(vec3f_t a) {
    return (a.x * a.x) + (a.y * a.y) + (a.z * a.z);
}

static inline float vec3f_length(vec3f_t a) {
    return sqrtf((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
}

static inline vec3f_t vec3f_normalized(vec3f_t a) {
    const float inv_length = 1.0f/vec3f_length(a);
    return vec3f_make(
            a.x * inv_length,
            a.y * inv_length,
            a.z * inv_length
    );
}

#endif //CETECH_VEC3F_H
