#ifndef CELIB_VEC2F_H
#define CELIB_VEC2F_H


//==============================================================================
// Includes
//==============================================================================

#include "types.h"
#include "math_fmath.inl"
#include "math_types.h"

#define VEC2F_UNIT_X (cel_vec2f_t){1.0f, 0.0f}
#define VEC2F_UNIT_Y (cel_vec2f_t){0.0f, 1.0f}

//==============================================================================
// Interface
//==============================================================================

CEL_FORCE_INLINE void cel_vec2f_move(cel_vec2f_t *__restrict result,
                                     const cel_vec2f_t *__restrict a) {
    result->f[0] = a->f[0];
    result->f[1] = a->f[1];
}

CEL_FORCE_INLINE int cel_vec2f_eq(const cel_vec2f_t *__restrict a,
                                  const cel_vec2f_t *__restrict b,
                                  const float epsilon) {
    return cel_float_equals(a->f, b->f, 2, epsilon);
}

CEL_FORCE_INLINE void cel_vec2f_add(cel_vec2f_t *__restrict result,
                                    const cel_vec2f_t *__restrict a,
                                    const cel_vec2f_t *__restrict b) {
    result->f[0] = a->f[0] + b->f[0];
    result->f[1] = a->f[1] + b->f[1];
}

CEL_FORCE_INLINE void cel_vec2f_sub(cel_vec2f_t *__restrict result,
                                    const cel_vec2f_t *__restrict a,
                                    const cel_vec2f_t *__restrict b) {
    result->f[0] = a->f[0] - b->f[0];
    result->f[1] = a->f[1] - b->f[1];
}

CEL_FORCE_INLINE void cel_vec2f_mul(cel_vec2f_t *__restrict result,
                                    const cel_vec2f_t *__restrict a,
                                    const float s) {
    result->f[0] = a->f[0] * s;
    result->f[1] = a->f[1] * s;
}

CEL_FORCE_INLINE void cel_vec2f_div(cel_vec2f_t *__restrict result,
                                    const cel_vec2f_t *__restrict a,
                                    const float s) {
    result->f[0] = a->f[0] / s;
    result->f[1] = a->f[1] / s;
}

CEL_FORCE_INLINE float cel_vec2f_dot(const cel_vec2f_t *__restrict a,
                                   const cel_vec2f_t *__restrict b) {
    return (a->f[0] * b->f[0]) + (a->f[1] * b->f[1]);
}

CEL_FORCE_INLINE float cel_vec2f_length_squared(const cel_vec2f_t *__restrict a) {
    return (a->f[0] * a->f[0]) + (a->f[1] * a->f[1]);
}

CEL_FORCE_INLINE float cel_vec2f_length(const cel_vec2f_t *__restrict a) {
    return cel_float_sqrt(cel_vec2f_length_squared(a));
}

CEL_FORCE_INLINE void cel_vec2f_normalized(cel_vec2f_t *__restrict result,
                                           const cel_vec2f_t *__restrict a) {
    const float inv_length = 1.0f / cel_vec2f_length(a);

    cel_vec2f_mul(result, a, inv_length);
}

CEL_FORCE_INLINE void cel_vec2f_lerp(cel_vec2f_t *__restrict result,
                                     const cel_vec2f_t *__restrict a,
                                     const cel_vec2f_t *__restrict b,
                                     float t) {

    result->f[0] = cel_float_lerp(a->f[0], b->f[0], t);
    result->f[1] = cel_float_lerp(a->f[1], b->f[1], t);
}

#endif //CELIB_VEC2F_H
