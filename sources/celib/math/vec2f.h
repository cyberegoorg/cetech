#ifndef CETECH_VEC2F_H
#define CETECH_VEC2F_H

#include <math.h>

static inline void vec2f_set(float* restrict result, const float* restrict a) {
    result[0] = a[0];
    result[1] = a[1];
}

static inline void vec2f_add(float* restrict result, const float* restrict a, const float* restrict b) {
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
}

static inline void vec2f_sub(float* restrict result, const float* restrict a, const float* restrict b) {
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
}

static inline void vec2f_mul(float* restrict result, const float* restrict a, const float s) {
    result[0] = a[0] * s;
    result[1] = a[1] * s;
}

static inline void vec2f_div(float* restrict result, const float* restrict a, const float s) {
    result[0] = a[0] / s;
    result[1] = a[1] / s;
}

static inline float vec2f_dot(const float* restrict a, const float* restrict b) {
    return (a[0] * b[0]) + (a[1] * b[1]);
}

static inline float vec2f_length_squared(const float* restrict a) {
    return (a[0] * a[0]) + (a[1] * a[1]);
}

static inline float vec2f_length(const float* restrict a) {
    return sqrtf(vec2f_length_squared(a));
}

static inline void vec2f_normalized(float* restrict result, const float* restrict a) {
    const float inv_length = 1.0f/vec2f_length(a);

    vec2f_mul(result, a, inv_length);
}

#endif //CETECH_VEC2F_H
