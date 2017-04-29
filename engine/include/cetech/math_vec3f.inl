#ifndef CELIB_VEC3F_H
#define CELIB_VEC3F_H


//==============================================================================
// Includes
//==============================================================================

#include "cetech/types.h"
#include "math_fmath.inl"
#include "math_types.h"

#define VEC3F_UNIT_X (cel_vec3f_t){1.0f, 0.0f, 0.0f}
#define VEC3F_UNIT_Y (cel_vec3f_t){0.0f, 1.0f, 0.0f}
#define VEC3F_UNIT_Z (cel_vec3f_t){0.0f, 0.0f, 1.0f}

//==============================================================================
// Interface
//==============================================================================

CEL_FORCE_INLINE void cel_vec3f_move(cel_vec3f_t *__restrict result,
                                     const cel_vec3f_t *__restrict a) {
    result->f[0] = a->f[0];
    result->f[1] = a->f[1];
    result->f[2] = a->f[2];
}

CEL_FORCE_INLINE int cel_vec3f_eq(const cel_vec3f_t *__restrict a,
                                  const cel_vec3f_t *__restrict b,
                                  const float epsilon) {
    return cel_float_equals(a->f, b->f, 3, epsilon);
}


CEL_FORCE_INLINE void cel_vec3f_add(cel_vec3f_t *__restrict result,
                                    const cel_vec3f_t *__restrict a,
                                    const cel_vec3f_t *__restrict b) {
    result->f[0] = a->f[0] + b->f[0];
    result->f[1] = a->f[1] + b->f[1];
    result->f[2] = a->f[2] + b->f[2];
}

CEL_FORCE_INLINE void cel_vec3f_sub(cel_vec3f_t *__restrict result,
                                    const cel_vec3f_t *__restrict a,
                                    const cel_vec3f_t *__restrict b) {
    result->f[0] = a->f[0] - b->f[0];
    result->f[1] = a->f[1] - b->f[1];
    result->f[2] = a->f[2] - b->f[2];
}

CEL_FORCE_INLINE void cel_vec3f_mul(cel_vec3f_t *__restrict result,
                                    const cel_vec3f_t *__restrict a,
                                    const float s) {
    result->f[0] = a->f[0] * s;
    result->f[1] = a->f[1] * s;
    result->f[2] = a->f[2] * s;
}

CEL_FORCE_INLINE void cel_vec3f_div(cel_vec3f_t *__restrict result,
                                    const cel_vec3f_t *__restrict a,
                                    const float s) {
    result->f[0] = a->f[0] / s;
    result->f[1] = a->f[1] / s;
    result->f[2] = a->f[2] / s;
}

CEL_FORCE_INLINE float cel_vec3f_dot(const cel_vec3f_t *__restrict a,
                                   const cel_vec3f_t *__restrict b) {
    return (a->f[0] * b->f[0]) + (a->f[1] * b->f[1]) + (a->f[2] * b->f[2]);
}

CEL_FORCE_INLINE void cel_vec3f_cross(cel_vec3f_t *__restrict result,
                                      const cel_vec3f_t *__restrict a,
                                      const cel_vec3f_t *__restrict b) {
    result->f[0] = a->f[1] * b->f[2] - a->f[2] * b->f[1];
    result->f[1] = a->f[2] * b->f[0] - a->f[0] * b->f[2];
    result->f[2] = a->f[0] * b->f[1] - a->f[1] * b->f[0];
}

CEL_FORCE_INLINE float cel_vec3f_length_squared(const cel_vec3f_t *__restrict a) {
    return (a->f[0] * a->f[0]) + (a->f[1] * a->f[1]) + (a->f[2] * a->f[2]);
}

CEL_FORCE_INLINE float cel_vec3f_length(const cel_vec3f_t *__restrict a) {
    return cel_float_sqrt(cel_vec3f_length_squared(a));
}

CEL_FORCE_INLINE void cel_vec3f_normalized(cel_vec3f_t *__restrict result,
                                           const cel_vec3f_t *__restrict a) {
    const float inv_length = 1.0f / cel_vec3f_length(a);

    cel_vec3f_mul(result, a, inv_length);
}

CEL_FORCE_INLINE void cel_vec3f_lerp(cel_vec3f_t *__restrict result,
                                     const cel_vec3f_t *__restrict a,
                                     const cel_vec3f_t *__restrict b,
                                     float t) {
    result->f[0] = cel_float_lerp(a->f[0], b->f[0], t);
    result->f[1] = cel_float_lerp(a->f[1], b->f[1], t);
    result->f[2] = cel_float_lerp(a->f[2], b->f[2], t);
}

#endif //CELIB_VEC3F_H
