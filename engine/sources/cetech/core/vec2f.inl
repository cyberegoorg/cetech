#ifndef CETECH_VEC2F_H
#define CETECH_VEC2F_H


//==============================================================================
// Includes
//==============================================================================

#include "types.h"
#include "fmath.inl"
#include "math_types.h"

#define VEC2F_UNIT_X (vec2f_t){1.0f, 0.0f}
#define VEC2F_UNIT_Y (vec2f_t){0.0f, 1.0f}

//==============================================================================
// Interface
//==============================================================================

CETECH_FORCE_INLINE void vec2f_move(vec2f_t *__restrict result,
                                     const vec2f_t *__restrict a) {
    result->f[0] = a->f[0];
    result->f[1] = a->f[1];
}

CETECH_FORCE_INLINE int vec2f_eq(const vec2f_t *__restrict a,
                                  const vec2f_t *__restrict b,
                                  const float epsilon) {
    return float_equals(a->f, b->f, 2, epsilon);
}

CETECH_FORCE_INLINE void vec2f_add(vec2f_t *__restrict result,
                                    const vec2f_t *__restrict a,
                                    const vec2f_t *__restrict b) {
    result->f[0] = a->f[0] + b->f[0];
    result->f[1] = a->f[1] + b->f[1];
}

CETECH_FORCE_INLINE void vec2f_sub(vec2f_t *__restrict result,
                                    const vec2f_t *__restrict a,
                                    const vec2f_t *__restrict b) {
    result->f[0] = a->f[0] - b->f[0];
    result->f[1] = a->f[1] - b->f[1];
}

CETECH_FORCE_INLINE void vec2f_mul(vec2f_t *__restrict result,
                                    const vec2f_t *__restrict a,
                                    const float s) {
    result->f[0] = a->f[0] * s;
    result->f[1] = a->f[1] * s;
}

CETECH_FORCE_INLINE void vec2f_div(vec2f_t *__restrict result,
                                    const vec2f_t *__restrict a,
                                    const float s) {
    result->f[0] = a->f[0] / s;
    result->f[1] = a->f[1] / s;
}

CETECH_FORCE_INLINE float vec2f_dot(const vec2f_t *__restrict a,
                                   const vec2f_t *__restrict b) {
    return (a->f[0] * b->f[0]) + (a->f[1] * b->f[1]);
}

CETECH_FORCE_INLINE float vec2f_length_squared(const vec2f_t *__restrict a) {
    return (a->f[0] * a->f[0]) + (a->f[1] * a->f[1]);
}

CETECH_FORCE_INLINE float vec2f_length(const vec2f_t *__restrict a) {
    return float_sqrt(vec2f_length_squared(a));
}

CETECH_FORCE_INLINE void vec2f_normalized(vec2f_t *__restrict result,
                                           const vec2f_t *__restrict a) {
    const float inv_length = 1.0f / vec2f_length(a);

    vec2f_mul(result, a, inv_length);
}

CETECH_FORCE_INLINE void vec2f_lerp(vec2f_t *__restrict result,
                                     const vec2f_t *__restrict a,
                                     const vec2f_t *__restrict b,
                                     float t) {

    result->f[0] = float_lerp(a->f[0], b->f[0], t);
    result->f[1] = float_lerp(a->f[1], b->f[1], t);
}

#endif //CETECH_VEC2F_H
