#ifndef CETECH_VEC2F_H
#define CETECH_VEC2F_H

#include <math.h>

#include "types.h"

#define vec2f_make(xx, yy) (vec2f_t){ .x = xx, .y = yy }

static inline vec2f_t vec2f_add(vec2f_t a, vec2f_t b) {
    return vec2f_make(
            a.x + b.x,
            a.y + b.y
    );
}

static inline vec2f_t vec2f_sub(vec2f_t a, vec2f_t b) {
    return vec2f_make(
            a.x - b.x,
            a.y - b.y
    );
}

static inline vec2f_t vec2f_mul(vec2f_t a, float s) {
    return vec2f_make(
            a.x * s,
            a.y * s
    );
}

static inline vec2f_t vec2f_div(vec2f_t a, float s) {
    const float inv_s = 1.0f/s;
    return vec2f_make(
            a.x * inv_s,
            a.y * inv_s
    );
}

static inline vec2f_t vec2f_dot(vec2f_t a, vec2f_t b) {
    return vec2f_make(
            a.x * b.x,
            a.y * b.y
    );
}

static inline float vec2f_length_squared(vec2f_t a) {
    return (a.x * a.x) + (a.y * a.y);
}

static inline float vec2f_length(vec2f_t a) {
    return sqrtf((a.x * a.x) + (a.y * a.y));
}

static inline vec2f_t vec2f_normalized(vec2f_t a) {
    const float inv_length = 1.0f/vec2f_length(a);
    return vec2f_make(a.x * inv_length, a.y * inv_length);
}

#endif //CETECH_VEC2F_H
