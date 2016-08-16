#ifndef CETECH_VEC4F_H
#define CETECH_VEC4F_H


//==============================================================================
// Includes
//==============================================================================

#include "../types.h"
#include "fmath.h"


//==============================================================================
// Functions
//==============================================================================

CE_FORCE_INLINE void vec4f_move(f32 *__restrict result,
                                const f32 *__restrict a) {
    result[0] = a[0];
    result[1] = a[1];
    result[2] = a[2];
    result[3] = a[3];
}

CE_FORCE_INLINE int vec4f_eq(const f32 *__restrict a,
                              const f32 *__restrict b,
                              const f32 epsilon) {
    return f32_equals(a, b, 4, epsilon);
}


CE_FORCE_INLINE void vec4f_add(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 *__restrict b) {
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
    result[3] = a[3] + b[3];
}

CE_FORCE_INLINE void vec4f_sub(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 *__restrict b) {
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
    result[3] = a[3] - b[3];
}

CE_FORCE_INLINE void vec4f_mul(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 s) {
    result[0] = a[0] * s;
    result[1] = a[1] * s;
    result[2] = a[2] * s;
    result[3] = a[3] * s;
}

CE_FORCE_INLINE void vec4f_mul_mat44f(f32 *__restrict result,
                                      const f32 *__restrict v,
                                      const f32 *__restrict m) {
    result[0] = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + v[3] * m[12];
    result[1] = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + v[3] * m[13];
    result[2] = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + v[3] * m[14];
    result[3] = v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + v[3] * m[15];
}

CE_FORCE_INLINE void vec4f_div(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 s) {
    result[0] = a[0] / s;
    result[1] = a[1] / s;
    result[2] = a[2] / s;
    result[3] = a[3] / s;
}

CE_FORCE_INLINE f32 vec4f_dot(const f32 *__restrict a,
                                const f32 *__restrict b) {
    return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]) + (a[3] * b[3]);
}

CE_FORCE_INLINE f32 vec4f_length_squared(const f32 *__restrict a) {
    return (a[0] * a[0]) + (a[1] * a[1]) + (a[2] * a[2]) + (a[3] * a[3]);
}

CE_FORCE_INLINE f32 vec4f_length(const f32 *__restrict a) {
    return f32_sqrt(vec4f_length_squared(a));
}

CE_FORCE_INLINE void vec4f_normalized(f32 *__restrict result,
                                      const f32 *__restrict a) {
    const f32 inv_length = 1.0f / vec4f_length(a);

    vec4f_mul(result, a, inv_length);
}

CE_FORCE_INLINE void vec4f_lerp(f32 *__restrict result,
                                const f32 *__restrict a,
                                const f32 *__restrict b,
                                f32 t) {
    result[0] = f32_lerp(a[0], b[0], t);
    result[1] = f32_lerp(a[1], b[1], t);
    result[2] = f32_lerp(a[2], b[2], t);
    result[3] = f32_lerp(a[3], b[3], t);
}

#endif //CETECH_VEC4F_H
