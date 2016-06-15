#ifndef CETECH_VEC2F_H
#define CETECH_VEC2F_H

#include <math.h>

static inline void vec2f_set(float* __restrict result, const float* __restrict a) {
    result[0] = a[0];
    result[1] = a[1];
}

static inline void vec2f_add(float* __restrict result, const float* __restrict a, const float* __restrict b) {
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
}

static inline void vec2f_sub(float* __restrict result, const float* __restrict a, const float* __restrict b) {
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
}

static inline void vec2f_mul(float* __restrict result, const float* __restrict a, const float s) {
    result[0] = a[0] * s;
    result[1] = a[1] * s;
}

static inline void vec2f_div(float* __restrict result, const float* __restrict a, const float s) {
    result[0] = a[0] / s;
    result[1] = a[1] / s;
}

static inline float vec2f_dot(const float* __restrict a, const float* __restrict b) {
    return (a[0] * b[0]) + (a[1] * b[1]);
}

static inline float vec2f_length_squared(const float* __restrict a) {
    return (a[0] * a[0]) + (a[1] * a[1]);
}

static inline float vec2f_length(const float* __restrict a) {
    return sqrtf(vec2f_length_squared(a));
}

static inline void vec2f_normalized(float* __restrict result, const float* __restrict a) {
    const float inv_length = 1.0f/vec2f_length(a);

    vec2f_mul(result, a, inv_length);
}

#endif //CETECH_VEC2F_H
