#ifndef CELIB_VEC2F_H
#define CELIB_VEC2F_H


//==============================================================================
// Includes
//==============================================================================

#include "../types.h"
#include "fmath.h"
#include "types.h"

#define VEC2F_UNIT_X (vec2f_t){1.0f, 0.0f}
#define VEC2F_UNIT_Y (vec2f_t){0.0f, 1.0f}

//==============================================================================
// Interface
//==============================================================================

CEL_FORCE_INLINE void vec2f_move(vec2f_t *__restrict result,
                                const vec2f_t *__restrict a) {
    result->f[0] = a->f[0];
    result->f[1] = a->f[1];
}

CEL_FORCE_INLINE int vec2f_eq(const vec2f_t *__restrict a,
                             const vec2f_t *__restrict b,
                             const f32 epsilon) {
    return f32_equals(a->f, b->f, 2, epsilon);
}

CEL_FORCE_INLINE void vec2f_add(vec2f_t *__restrict result,
                               const vec2f_t *__restrict a,
                               const vec2f_t *__restrict b) {
    result->f[0] = a->f[0] + b->f[0];
    result->f[1] = a->f[1] + b->f[1];
}

CEL_FORCE_INLINE void vec2f_sub(vec2f_t *__restrict result,
                               const vec2f_t *__restrict a,
                               const vec2f_t *__restrict b) {
    result->f[0] = a->f[0] - b->f[0];
    result->f[1] = a->f[1] - b->f[1];
}

CEL_FORCE_INLINE void vec2f_mul(vec2f_t *__restrict result,
                               const vec2f_t *__restrict a,
                               const f32 s) {
    result->f[0] = a->f[0] * s;
    result->f[1] = a->f[1] * s;
}

CEL_FORCE_INLINE void vec2f_div(vec2f_t *__restrict result,
                               const vec2f_t *__restrict a,
                               const f32 s) {
    result->f[0] = a->f[0] / s;
    result->f[1] = a->f[1] / s;
}

CEL_FORCE_INLINE f32 vec2f_dot(const vec2f_t *__restrict a,
                              const vec2f_t *__restrict b) {
    return (a->f[0] * b->f[0]) + (a->f[1] * b->f[1]);
}

CEL_FORCE_INLINE f32 vec2f_length_squared(const vec2f_t *__restrict a) {
    return (a->f[0] * a->f[0]) + (a->f[1] * a->f[1]);
}

CEL_FORCE_INLINE f32 vec2f_length(const vec2f_t *__restrict a) {
    return f32_sqrt(vec2f_length_squared(a));
}

CEL_FORCE_INLINE void vec2f_normalized(vec2f_t *__restrict result,
                                      const vec2f_t *__restrict a) {
    const f32 inv_length = 1.0f / vec2f_length(a);

    vec2f_mul(result, a, inv_length);
}

CEL_FORCE_INLINE void vec2f_lerp(vec2f_t *__restrict result,
                                const vec2f_t *__restrict a,
                                const vec2f_t *__restrict b,
                                f32 t) {

    result->f[0] = f32_lerp(a->f[0], b->f[0], t);
    result->f[1] = f32_lerp(a->f[1], b->f[1], t);
}

#endif //CELIB_VEC2F_H
