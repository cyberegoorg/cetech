/***********************************************************************
**** Vector 2
***********************************************************************/

#ifndef CETECH_VEC2F_H
#define CETECH_VEC2F_H


/***********************************************************************
**** Includes
***********************************************************************/

#include "fmath.h"
#include "../utils/macros.h"


/***********************************************************************
**** Functions
***********************************************************************/

static CE_FORCE_INLINE void vec2f_move(float* __restrict result, const float* __restrict a) {
    result[0] = a[0];
    result[1] = a[1];
}

static CE_FORCE_INLINE bool vec2f_eq(const float* __restrict a, const float* __restrict b, const float epsilon) {
    return f_equals(a, b, 2, epsilon);
}

static CE_FORCE_INLINE void vec2f_add(float* __restrict result, const float* __restrict a, const float* __restrict b) {
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
}

static CE_FORCE_INLINE void vec2f_sub(float* __restrict result, const float* __restrict a, const float* __restrict b) {
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
}

static CE_FORCE_INLINE void vec2f_mul(float* __restrict result, const float* __restrict a, const float s) {
    result[0] = a[0] * s;
    result[1] = a[1] * s;
}

static CE_FORCE_INLINE void vec2f_div(float* __restrict result, const float* __restrict a, const float s) {
    result[0] = a[0] / s;
    result[1] = a[1] / s;
}

static CE_FORCE_INLINE float vec2f_dot(const float* __restrict a, const float* __restrict b) {
    return (a[0] * b[0]) + (a[1] * b[1]);
}

static CE_FORCE_INLINE float vec2f_length_squared(const float* __restrict a) {
    return (a[0] * a[0]) + (a[1] * a[1]);
}

static CE_FORCE_INLINE float vec2f_length(const float* __restrict a) {
    return f_sqrt(vec2f_length_squared(a));
}

static CE_FORCE_INLINE void vec2f_normalized(float* __restrict result, const float* __restrict a) {
    const float inv_length = 1.0f/vec2f_length(a);

    vec2f_mul(result, a, inv_length);
}

static CE_FORCE_INLINE void vec2f_lerp(float* __restrict result, const float* __restrict a, const float* __restrict b, float t) {
    result[0] = f_lerp(a[0], b[0], t);
    result[1] = f_lerp(a[1], b[1], t);
}

#endif //CETECH_VEC2F_H
