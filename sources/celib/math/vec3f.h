/***********************************************************************
**** Vector 3
***********************************************************************/

#ifndef CETECH_VEC3F_H
#define CETECH_VEC3F_H


/***********************************************************************
**** Includes
***********************************************************************/

#include "fmath.h"
#include "../utils/macros.h"


/***********************************************************************
**** Functions
***********************************************************************/

static CE_FORCE_INLINE void vec3f_move(float* __restrict result, const float* __restrict a) {
    result[0] = a[0];
    result[1] = a[1];
    result[2] = a[2];
}

static CE_FORCE_INLINE bool vec3f_eq(const float* __restrict a, const float* __restrict b, const float epsilon) {
    return f_equals(a, b, 3, epsilon);
}


static CE_FORCE_INLINE void vec3f_add(float* __restrict result, const float* __restrict a, const float* __restrict b) {
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
}

static CE_FORCE_INLINE void vec3f_sub(float* __restrict result, const float* __restrict a, const float* __restrict b) {
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
}

static CE_FORCE_INLINE void vec3f_mul(float* __restrict result, const float* __restrict a, const float s) {
    result[0] = a[0] * s;
    result[1] = a[1] * s;
    result[2] = a[2] * s;
}

static CE_FORCE_INLINE void vec3f_div(float* __restrict result, const float* __restrict a, const float s) {
    result[0] = a[0] / s;
    result[1] = a[1] / s;
    result[2] = a[2] / s;
}

static CE_FORCE_INLINE float vec3f_dot(const float* __restrict a, const float* __restrict b) {
    return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

static CE_FORCE_INLINE void vec3f_cross(float* __restrict result, const float* __restrict a, const float* __restrict b) {
    result[0] = a[1]*b[2] - a[2]*b[1];
    result[1] = a[2]*b[0] - a[0]*b[2],
    result[2] = a[0]*b[1] - a[1]*b[0];
}

static CE_FORCE_INLINE float vec3f_length_squared(const float* __restrict a) {
    return (a[0] * a[0]) + (a[1] * a[1]) + (a[2] * a[2]);
}

static CE_FORCE_INLINE float vec3f_length(const float* __restrict a) {
    return f_sqrt(vec3f_length_squared(a));
}

static CE_FORCE_INLINE void vec3f_normalized(float* __restrict result, const float* __restrict a) {
    const float inv_length = 1.0f/vec3f_length(a);

    vec3f_mul(result, a, inv_length);
}

static CE_FORCE_INLINE void vec3f_lerp(float* __restrict result, const float* __restrict a, const float* __restrict b, float t) {
    result[0] = f_lerp(a[0], b[0], t);
    result[1] = f_lerp(a[1], b[1], t);
    result[2] = f_lerp(a[2], b[2], t);
}

#endif //CETECH_VEC3F_H
