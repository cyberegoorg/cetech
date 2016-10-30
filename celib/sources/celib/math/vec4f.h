#ifndef CELIB_VEC4F_H
#define CELIB_VEC4F_H


//==============================================================================
// Includes
//==============================================================================

#include "../types.h"
#include "fmath.h"
#include "types.h"

#define VEC4F_UNIT_X (vec4f_t){1.0f, 0.0f, 0.0f, 0.0f}
#define VEC4F_UNIT_Y (vec4f_t){0.0f, 1.0f, 0.0f, 0.0f}
#define VEC4F_UNIT_Z (vec4f_t){0.0f, 0.0f, 1.0f, 0.0f}
#define VEC4F_UNIT_W (vec4f_t){0.0f, 0.0f, 0.0f, 1.0f}


//==============================================================================
// Functions
//==============================================================================

CE_FORCE_INLINE void vec4f_move(vec4f_t *__restrict result,
                                const vec4f_t *__restrict a) {
    result->f[0] = a->f[0];
    result->f[1] = a->f[1];
    result->f[2] = a->f[2];
    result->f[3] = a->f[3];
}

CE_FORCE_INLINE int vec4f_eq(const vec4f_t *__restrict a,
                             const vec4f_t *__restrict b,
                             const f32 epsilon) {
    return f32_equals(a->f, b->f, 4, epsilon);
}


CE_FORCE_INLINE void vec4f_add(vec4f_t *__restrict result,
                               const vec4f_t *__restrict a,
                               const vec4f_t *__restrict b) {
    result->f[0] = a->f[0] + b->f[0];
    result->f[1] = a->f[1] + b->f[1];
    result->f[2] = a->f[2] + b->f[2];
    result->f[3] = a->f[3] + b->f[3];
}

CE_FORCE_INLINE void vec4f_sub(vec4f_t *__restrict result,
                               const vec4f_t *__restrict a,
                               const vec4f_t *__restrict b) {
    result->f[0] = a->f[0] - b->f[0];
    result->f[1] = a->f[1] - b->f[1];
    result->f[2] = a->f[2] - b->f[2];
    result->f[3] = a->f[3] - b->f[3];
}

CE_FORCE_INLINE void vec4f_mul(vec4f_t *__restrict result,
                               const vec4f_t *__restrict a,
                               const f32 s) {
    result->f[0] = a->f[0] * s;
    result->f[1] = a->f[1] * s;
    result->f[2] = a->f[2] * s;
    result->f[3] = a->f[3] * s;
}

CE_FORCE_INLINE void vec4f_mul_mat44f(vec4f_t *__restrict result,
                                      const vec4f_t *__restrict v,
                                      const mat44f_t *__restrict m) {

    result->f[0] = v->f[0] * m->f[0] + v->f[1] * m->f[4] + v->f[2] * m->f[8] + v->f[3] * m->f[12];
    result->f[1] = v->f[0] * m->f[1] + v->f[1] * m->f[5] + v->f[2] * m->f[9] + v->f[3] * m->f[13];
    result->f[2] = v->f[0] * m->f[2] + v->f[1] * m->f[6] + v->f[2] * m->f[10] + v->f[3] * m->f[14];
    result->f[3] = v->f[0] * m->f[3] + v->f[1] * m->f[7] + v->f[2] * m->f[11] + v->f[3] * m->f[15];
}

CE_FORCE_INLINE void vec4f_div(vec4f_t *__restrict result,
                               const vec4f_t *__restrict a,
                               const f32 s) {

    result->f[0] = a->f[0] / s;
    result->f[1] = a->f[1] / s;
    result->f[2] = a->f[2] / s;
    result->f[3] = a->f[3] / s;
}

CE_FORCE_INLINE f32 vec4f_dot(const vec4f_t *__restrict a,
                              const vec4f_t *__restrict b) {
    return (a->f[0] * b->f[0]) + (a->f[1] * b->f[1]) + (a->f[2] * b->f[2]) + (a->f[3] * b->f[3]);
}

CE_FORCE_INLINE f32 vec4f_length_squared(const vec4f_t *__restrict a) {
    return (a->f[0] * a->f[0]) + (a->f[1] * a->f[1]) + (a->f[2] * a->f[2]) + (a->f[3] * a->f[3]);
}

CE_FORCE_INLINE f32 vec4f_length(const vec4f_t *__restrict a) {
    return f32_sqrt(vec4f_length_squared(a));
}

CE_FORCE_INLINE void vec4f_normalized(vec4f_t *__restrict result,
                                      const vec4f_t *__restrict a) {
    const f32 inv_length = 1.0f / vec4f_length(a);

    vec4f_mul(result, a, inv_length);
}

CE_FORCE_INLINE void vec4f_lerp(vec4f_t *__restrict result,
                                const vec4f_t *__restrict a,
                                const vec4f_t *__restrict b,
                                f32 t) {

    result->f[0] = f32_lerp(a->f[0], b->f[0], t);
    result->f[1] = f32_lerp(a->f[1], b->f[1], t);
    result->f[2] = f32_lerp(a->f[2], b->f[2], t);
    result->f[3] = f32_lerp(a->f[3], b->f[3], t);
}

#endif //CELIB_VEC4F_H
