#ifndef CETECH_VEC3F_H
#define CETECH_VEC3F_H


//==============================================================================
// Includes
//==============================================================================

#include "celib/types.h"
#include "fmath.h"
#include "types.h"

#define VEC3F_UNIT_X (vec3f_t){1.0f, 0.0f, 0.0f}
#define VEC3F_UNIT_Y (vec3f_t){0.0f, 1.0f, 0.0f}
#define VEC3F_UNIT_Z (vec3f_t){0.0f, 0.0f, 1.0f}

//==============================================================================
// Interface
//==============================================================================

CE_FORCE_INLINE void vec3f_move(vec3f_t *__restrict result,
                                const vec3f_t *__restrict a) {
    result->f[0] = a->f[0];
    result->f[1] = a->f[1];
    result->f[2] = a->f[2];
}

CE_FORCE_INLINE int vec3f_eq(const vec3f_t *__restrict a,
                             const vec3f_t *__restrict b,
                             const f32 epsilon) {
    return f32_equals(a->f, b->f, 3, epsilon);
}


CE_FORCE_INLINE void vec3f_add(vec3f_t *__restrict result,
                               const vec3f_t *__restrict a,
                               const vec3f_t *__restrict b) {
    result->f[0] = a->f[0] + b->f[0];
    result->f[1] = a->f[1] + b->f[1];
    result->f[2] = a->f[2] + b->f[2];
}

CE_FORCE_INLINE void vec3f_sub(vec3f_t *__restrict result,
                               const vec3f_t *__restrict a,
                               const vec3f_t *__restrict b) {
    result->f[0] = a->f[0] - b->f[0];
    result->f[1] = a->f[1] - b->f[1];
    result->f[2] = a->f[2] - b->f[2];
}

CE_FORCE_INLINE void vec3f_mul(vec3f_t *__restrict result,
                               const vec3f_t *__restrict a,
                               const f32 s) {
    result->f[0] = a->f[0] * s;
    result->f[1] = a->f[1] * s;
    result->f[2] = a->f[2] * s;
}

CE_FORCE_INLINE void vec3f_div(vec3f_t *__restrict result,
                               const vec3f_t *__restrict a,
                               const f32 s) {
    result->f[0] = a->f[0] / s;
    result->f[1] = a->f[1] / s;
    result->f[2] = a->f[2] / s;
}

CE_FORCE_INLINE f32 vec3f_dot(const vec3f_t *__restrict a,
                              const vec3f_t *__restrict b) {
    return (a->f[0] * b->f[0]) + (a->f[1] * b->f[1]) + (a->f[2] * b->f[2]);
}

CE_FORCE_INLINE void vec3f_cross(vec3f_t *__restrict result,
                                 const vec3f_t *__restrict a,
                                 const vec3f_t *__restrict b) {
    result->f[0] = a->f[1] * b->f[2] - a->f[2] * b->f[1];
    result->f[1] = a->f[2] * b->f[0] - a->f[0] * b->f[2],
            result->f[2] = a->f[0] * b->f[1] - a->f[1] * b->f[0];
}

CE_FORCE_INLINE f32 vec3f_length_squared(const vec3f_t *__restrict a) {
    return (a->f[0] * a->f[0]) + (a->f[1] * a->f[1]) + (a->f[2] * a->f[2]);
}

CE_FORCE_INLINE f32 vec3f_length(const vec3f_t *__restrict a) {
    return f32_sqrt(vec3f_length_squared(a));
}

CE_FORCE_INLINE void vec3f_normalized(vec3f_t *__restrict result,
                                      const vec3f_t *__restrict a) {
    const f32 inv_length = 1.0f / vec3f_length(a);

    vec3f_mul(result, a, inv_length);
}

CE_FORCE_INLINE void vec3f_lerp(vec3f_t *__restrict result,
                                const vec3f_t *__restrict a,
                                const vec3f_t *__restrict b,
                                f32 t) {
    result->f[0] = f32_lerp(a->f[0], b->f[0], t);
    result->f[1] = f32_lerp(a->f[1], b->f[1], t);
    result->f[2] = f32_lerp(a->f[2], b->f[2], t);
}

#endif //CETECH_VEC3F_H
