#ifndef CETECH_VEC3F_H
#define CETECH_VEC3F_H


//==============================================================================
// Includes
//==============================================================================

#include "../types.h"
#include "fmath.h"



//==============================================================================
// Interface
//==============================================================================

CE_FORCE_INLINE void vec3f_move(f32 *__restrict result,
                                const f32 *__restrict a) {
    result[0] = a[0];
    result[1] = a[1];
    result[2] = a[2];
}

CE_FORCE_INLINE int vec3f_eq(const f32 *__restrict a,
                              const f32 *__restrict b,
                              const f32 epsilon) {
    return f32_equals(a, b, 3, epsilon);
}


CE_FORCE_INLINE void vec3f_add(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 *__restrict b) {
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
}

CE_FORCE_INLINE void vec3f_sub(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 *__restrict b) {
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
}

CE_FORCE_INLINE void vec3f_mul(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 s) {
    result[0] = a[0] * s;
    result[1] = a[1] * s;
    result[2] = a[2] * s;
}

CE_FORCE_INLINE void vec3f_div(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 s) {
    result[0] = a[0] / s;
    result[1] = a[1] / s;
    result[2] = a[2] / s;
}

CE_FORCE_INLINE f32 vec3f_dot(const f32 *__restrict a,
                                const f32 *__restrict b) {
    return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

CE_FORCE_INLINE void vec3f_cross(f32 *__restrict result,
                                 const f32 *__restrict a,
                                 const f32 *__restrict b) {
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2],
            result[2] = a[0] * b[1] - a[1] * b[0];
}

CE_FORCE_INLINE f32 vec3f_length_squared(const f32 *__restrict a) {
    return (a[0] * a[0]) + (a[1] * a[1]) + (a[2] * a[2]);
}

CE_FORCE_INLINE f32 vec3f_length(const f32 *__restrict a) {
    return f32_sqrt(vec3f_length_squared(a));
}

CE_FORCE_INLINE void vec3f_normalized(f32 *__restrict result,
                                      const f32 *__restrict a) {
    const f32 inv_length = 1.0f / vec3f_length(a);

    vec3f_mul(result, a, inv_length);
}

CE_FORCE_INLINE void vec3f_lerp(f32 *__restrict result,
                                const f32 *__restrict a,
                                const f32 *__restrict b,
                                f32 t) {
    result[0] = f32_lerp(a[0], b[0], t);
    result[1] = f32_lerp(a[1], b[1], t);
    result[2] = f32_lerp(a[2], b[2], t);
}

#endif //CETECH_VEC3F_H
