#ifndef CELIB_QUATF_H
#define CELIB_QUATF_H


//==============================================================================
// Includes
//==============================================================================

#include "../types.h"

#include "fmath.h"
#include "types.h"
#include "vec3f.h"
#include "vec4f.h"


//==============================================================================
// Defines
//==============================================================================

#define QUATF_ZERO (cel_quatf_t){0}
#define QUATF_IDENTITY (cel_quatf_t){0.0f, 0.0f, 0.0f, 1.0f}

//==============================================================================
// Interface
//==============================================================================

CEL_FORCE_INLINE void cel_quatf_move(cel_quatf_t *__restrict result,
                                     const cel_quatf_t *__restrict a) {
    result->f[0] = a->f[0];
    result->f[1] = a->f[1];
    result->f[2] = a->f[2];
    result->f[3] = a->f[3];
}

CEL_FORCE_INLINE int cel_quatf_eq(const cel_quatf_t *__restrict a,
                                  const cel_quatf_t *__restrict b,
                                  const f32 epsilon) {
    return cel_f32_equals(a->f, b->f, 4, epsilon);
}


CEL_FORCE_INLINE void cel_quatf_from_axis_angle(cel_quatf_t *__restrict result,
                                                const cel_vec3f_t *__restrict axis,
                                                const f32 angle) {
    cel_vec3f_t norm_axis;
    cel_vec3f_normalized(&norm_axis, axis);

    const f32 angle_half = angle * 0.5f * CEL_F32_TORAD;
    const f32 sin = cel_f32_sin(angle_half);

    result->f[0] = sin * norm_axis.x;
    result->f[1] = sin * norm_axis.y;
    result->f[2] = sin * norm_axis.z;

    result->f[3] = cel_f32_cos(angle_half);
}

CEL_FORCE_INLINE void cel_quatf_from_euler(cel_quatf_t *__restrict result,
                                           f32 heading,
                                           f32 attitude,
                                           f32 bank) {
    const f32 sx = cel_f32_sin(heading * 0.5f);
    const f32 sy = cel_f32_sin(attitude * 0.5f);
    const f32 sz = cel_f32_sin(bank * 0.5f);
    const f32 cx = cel_f32_cos(heading * 0.5f);
    const f32 cy = cel_f32_cos(attitude * 0.5f);
    const f32 cz = cel_f32_cos(bank * 0.5f);

    result->f[0] = sx * cy * cz - cx * sy * sz;
    result->f[1] = cx * sy * cz + sx * cy * sz;
    result->f[2] = cx * cy * sz - sx * sy * cz;
    result->f[3] = cx * cy * cz + sx * sy * sz;
}

CEL_FORCE_INLINE void cel_quatf_to_mat44f(cel_mat44f_t *__restrict result,
                                          const cel_quatf_t *__restrict a) {
    result->f[0] = 1.0f - 2.0f * a->f[1] * a->f[1] - 2.0f * a->f[2] * a->f[2];
    result->f[1] = 2.0f * a->f[0] * a->f[1] + 2.0f * a->f[3] * a->f[2];
    result->f[2] = 2.0f * a->f[0] * a->f[2] - 2.0f * a->f[3] * a->f[1];
    result->f[3] = 0.0f;

    result->f[4] = 2.0f * a->f[0] * a->f[1] - 2.0f * a->f[3] * a->f[2];
    result->f[5] = 1.0f - 2.0f * a->f[0] * a->f[0] - 2.0f * a->f[2] * a->f[2];
    result->f[6] = 2.0f * a->f[1] * a->f[2] + 2.0f * a->f[3] * a->f[0];
    result->f[7] = 0.0f;

    result->f[8] = 2.0f * a->f[0] * a->f[2] + 2.0f * a->f[3] * a->f[1];
    result->f[9] = 2.0f * a->f[1] * a->f[2] - 2.0f * a->f[3] * a->f[0];
    result->f[10] = 1.0f - 2.0f * a->f[0] * a->f[0] - 2.0f * a->f[1] * a->f[1];
    result->f[11] = 0.0f;

    result->f[12] = 0.0f;
    result->f[13] = 0.0f;
    result->f[14] = 0.0f;
    result->f[15] = 1.0f;
}


CEL_FORCE_INLINE void cel_quatf_to_eurel_angle(cel_vec3f_t *__restrict result,
                                               const cel_quatf_t *__restrict a) {
    result->f[0] = cel_f32_atan2(2.0f * (a->f[0] * a->f[3] - a->f[1] * a->f[2]),
                                 1.0f - 2.0f * (cel_f32_sq(a->f[0]) + cel_f32_sq(a->f[2])));

    result->f[1] = cel_f32_atan2(2.0f * (a->f[1] * a->f[3] + a->f[0] * a->f[2]),
                                 1.0f - 2.0f * (cel_f32_sq(a->f[1]) + cel_f32_sq(a->f[2])));

    result->f[2] = cel_f32_asin(2.0f * (a->f[0] * a->f[1] + a->f[2] * a->f[3]));
}

CEL_FORCE_INLINE int cel_quatf_is_identity(const cel_quatf_t *__restrict a,
                                           const f32 epsilon) {
    static cel_quatf_t _identity = QUATF_IDENTITY;
    return cel_quatf_eq(a, &_identity, epsilon);
}

CEL_FORCE_INLINE void cel_quatf_add(cel_quatf_t *__restrict result,
                                    const cel_quatf_t *__restrict a,
                                    const cel_quatf_t *__restrict b) {
    result->f[0] = a->f[0] + b->f[0];
    result->f[1] = a->f[1] + b->f[1];
    result->f[2] = a->f[2] + b->f[2];
    result->f[3] = a->f[3] + b->f[3];
}

CEL_FORCE_INLINE void cel_quatf_sub(cel_quatf_t *__restrict result,
                                    const cel_quatf_t *__restrict a,
                                    const cel_quatf_t *__restrict b) {
    result->f[0] = a->f[0] - b->f[0];
    result->f[1] = a->f[1] - b->f[1];
    result->f[2] = a->f[2] - b->f[2];
    result->f[3] = a->f[3] - b->f[3];
}

CEL_FORCE_INLINE void cel_quatf_neg(cel_quatf_t *__restrict result,
                                    const cel_quatf_t *__restrict a) {
    result->f[0] = -a->f[0];
    result->f[1] = -a->f[1];
    result->f[2] = -a->f[2];
    result->f[3] = -a->f[3];
}


CEL_FORCE_INLINE void cel_quatf_mul_s(cel_quatf_t *__restrict result,
                                      const cel_quatf_t *__restrict a,
                                      const f32 s) {
    result->f[0] = a->f[0] * s;
    result->f[1] = a->f[1] * s;
    result->f[2] = a->f[2] * s;
    result->f[3] = a->f[3] * s;
}

CEL_FORCE_INLINE void cel_quatf_div_s(cel_quatf_t *__restrict result,
                                      const cel_quatf_t *__restrict a,
                                      const f32 s) {
    result->f[0] = a->f[0] / s;
    result->f[1] = a->f[1] / s;
    result->f[2] = a->f[2] / s;
    result->f[3] = a->f[3] / s;
}

CEL_FORCE_INLINE void cel_quatf_mul(cel_quatf_t *__restrict result,
                                    const cel_quatf_t *__restrict a,
                                    const cel_quatf_t *__restrict b) {

    result->f[0] = a->f[3] * b->f[0] + a->f[0] * b->f[3] + a->f[1] * b->f[2] - a->f[2] * b->f[1];
    result->f[1] = a->f[3] * b->f[1] + a->f[1] * b->f[3] + a->f[2] * b->f[0] - a->f[0] * b->f[2];
    result->f[2] = a->f[3] * b->f[2] + a->f[2] * b->f[3] + a->f[0] * b->f[1] - a->f[1] * b->f[0];
    result->f[3] = a->f[3] * b->f[3] - a->f[0] * b->f[0] - a->f[1] * b->f[1] - a->f[2] * b->f[2];
}

CEL_FORCE_INLINE void cel_quatf_mul_vec3f(cel_vec3f_t *__restrict result,
                                          const cel_quatf_t *__restrict a,
                                          const cel_vec3f_t *__restrict b) {
    cel_vec3f_t tmp_vec1, tmp_vec2, tmp_vec3;

    cel_vec3f_cross(&tmp_vec1, (const cel_vec3f_t *) a, b);
    cel_vec3f_mul(&tmp_vec3, &tmp_vec1, 2.0f * a->f[3]);

    cel_vec3f_mul(&tmp_vec1, b, cel_f32_sq(a->f[3]) - cel_vec3f_dot((cel_vec3f_t *) a, (cel_vec3f_t *) a));
    cel_vec3f_add(&tmp_vec2, &tmp_vec1, &tmp_vec3);

    cel_vec3f_mul(&tmp_vec3, (const cel_vec3f_t *) a, 2.0f * cel_vec3f_dot((cel_vec3f_t *) a, b));
    cel_vec3f_add(result, &tmp_vec2, &tmp_vec3);
}

CEL_FORCE_INLINE f32 cel_quatf_length_squared(const cel_quatf_t *__restrict a) {
    return (a->f[0] * a->f[0]) + (a->f[1] * a->f[1]) + (a->f[2] * a->f[2]) + (a->f[3] * a->f[3]);
}

CEL_FORCE_INLINE f32 cel_quatf_length(const cel_quatf_t *__restrict a) {
    return cel_f32_fast_sqrt(cel_quatf_length_squared(a));
}

CEL_FORCE_INLINE f32 cel_quatf_inv_length(const cel_quatf_t *__restrict a) {
    return cel_f32_fast_inv_sqrt(cel_quatf_length_squared(a));
}

CEL_FORCE_INLINE void cel_quatf_normalized(cel_quatf_t *__restrict result,
                                           const cel_quatf_t *__restrict a) {
    cel_quatf_mul_s(result, a, cel_quatf_inv_length(a));
}

CEL_FORCE_INLINE void cel_quatf_invert(cel_quatf_t *__restrict result,
                                       const cel_quatf_t *__restrict a) {
    result->f[0] = -a->f[0];
    result->f[1] = -a->f[1];
    result->f[2] = -a->f[2];
    result->f[3] = a->f[3];
}

#endif //CELIB_QUATF_H
