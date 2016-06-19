/***********************************************************************
**** Vector 4
***********************************************************************/

#ifndef CETECH_VEC4F_H
#define CETECH_VEC4F_H


/***********************************************************************
**** Includes
***********************************************************************/

#include "fmath.h"
#include "../utils/macros.h"


/***********************************************************************
**** functions
***********************************************************************/

CE_FORCE_INLINE void vec4f_move(float* __restrict result,
                                const float* __restrict a) {
    result[0] = a[0];
    result[1] = a[1];
    result[2] = a[2];
    result[3] = a[3];
}

CE_FORCE_INLINE bool vec4f_eq(const float* __restrict a,
                              const float* __restrict b,
                              const float epsilon) {
    return f_equals(a, b, 4, epsilon);
}


CE_FORCE_INLINE void vec4f_add(float* __restrict result,
                               const float* __restrict a,
                               const float* __restrict b) {
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
    result[3] = a[3] + b[3];
}

CE_FORCE_INLINE void vec4f_sub(float* __restrict result,
                               const float* __restrict a,
                               const float* __restrict b) {
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
    result[3] = a[3] - b[3];
}

CE_FORCE_INLINE void vec4f_mul(float* __restrict result,
                               const float* __restrict a,
                               const float s) {
    result[0] = a[0] * s;
    result[1] = a[1] * s;
    result[2] = a[2] * s;
    result[3] = a[3] * s;
}

CE_FORCE_INLINE void vec4f_div(float* __restrict result,
                               const float* __restrict a,
                               const float s) {
    result[0] = a[0] / s;
    result[1] = a[1] / s;
    result[2] = a[2] / s;
    result[3] = a[3] / s;
}

CE_FORCE_INLINE float vec4f_dot(const float* __restrict a,
                                const float* __restrict b) {
    return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]) + (a[3] * b[3]);
}

CE_FORCE_INLINE float vec4f_length_squared(const float* __restrict a) {
    return (a[0] * a[0]) + (a[1] * a[1]) + (a[2] * a[2]) + (a[3] * a[3]);
}

CE_FORCE_INLINE float vec4f_length(const float* __restrict a) {
    return f_sqrt(vec4f_length_squared(a));
}

CE_FORCE_INLINE void vec4f_normalized(float* __restrict result,
                                      const float* __restrict a) {
    const float inv_length = 1.0f/vec4f_length(a);

    vec4f_mul(result, a, inv_length);
}

CE_FORCE_INLINE void vec4f_lerp(float* __restrict result,
                                const float* __restrict a,
                                const float* __restrict b,
                                float t) {
    result[0] = f_lerp(a[0], b[0], t);
    result[1] = f_lerp(a[1], b[1], t);
    result[2] = f_lerp(a[2], b[2], t);
    result[3] = f_lerp(a[3], b[3], t);
}

#endif //CETECH_VEC4F_H
