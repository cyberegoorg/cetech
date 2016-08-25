#ifndef CETECH_VEC2F_H
#define CETECH_VEC2F_H


//==============================================================================
// Includes
//==============================================================================

#include "celib/types.h"
#include "fmath.h"


//==============================================================================
// Interface
//==============================================================================

CE_FORCE_INLINE void vec2f_move(f32 *__restrict result,
                                const f32 *__restrict a) {
    result[0] = a[0];
    result[1] = a[1];
}

CE_FORCE_INLINE int vec2f_eq(const f32 *__restrict a,
                             const f32 *__restrict b,
                             const f32 epsilon) {
    return f32_equals(a, b, 2, epsilon);
}

CE_FORCE_INLINE void vec2f_add(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 *__restrict b) {
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
}

CE_FORCE_INLINE void vec2f_sub(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 *__restrict b) {
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
}

CE_FORCE_INLINE void vec2f_mul(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 s) {
    result[0] = a[0] * s;
    result[1] = a[1] * s;
}

CE_FORCE_INLINE void vec2f_div(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 s) {
    result[0] = a[0] / s;
    result[1] = a[1] / s;
}

CE_FORCE_INLINE f32 vec2f_dot(const f32 *__restrict a,
                              const f32 *__restrict b) {
    return (a[0] * b[0]) + (a[1] * b[1]);
}

CE_FORCE_INLINE f32 vec2f_length_squared(const f32 *__restrict a) {
    return (a[0] * a[0]) + (a[1] * a[1]);
}

CE_FORCE_INLINE f32 vec2f_length(const f32 *__restrict a) {
    return f32_sqrt(vec2f_length_squared(a));
}

CE_FORCE_INLINE void vec2f_normalized(f32 *__restrict result,
                                      const f32 *__restrict a) {
    const f32 inv_length = 1.0f / vec2f_length(a);

    vec2f_mul(result, a, inv_length);
}

CE_FORCE_INLINE void vec2f_lerp(f32 *__restrict result,
                                const f32 *__restrict a,
                                const f32 *__restrict b,
                                f32 t) {
    result[0] = f32_lerp(a[0], b[0], t);
    result[1] = f32_lerp(a[1], b[1], t);
}

#endif //CETECH_VEC2F_H
