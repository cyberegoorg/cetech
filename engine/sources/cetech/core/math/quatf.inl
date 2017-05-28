#ifndef CETECH_QUATF_H
#define CETECH_QUATF_H


//==============================================================================
// Includes
//==============================================================================

#include "cetech/core/types.h"

#include "fmath.inl"
#include "math_types.h"
#include "vec3f.inl"
#include "vec4f.inl"


//==============================================================================
// Defines
//==============================================================================

#define QUATF_ZERO (quatf_t){0}
#define QUATF_IDENTITY (quatf_t){0.0f, 0.0f, 0.0f, 1.0f}

//==============================================================================
// Interface
//==============================================================================

CETECH_FORCE_INLINE void quatf_move(quatf_t *__restrict result,
                                    const quatf_t *__restrict a) {
    result->f[0] = a->f[0];
    result->f[1] = a->f[1];
    result->f[2] = a->f[2];
    result->f[3] = a->f[3];
}

CETECH_FORCE_INLINE int quatf_eq(const quatf_t *__restrict a,
                                 const quatf_t *__restrict b,
                                 const float epsilon) {
    return float_equals(a->f, b->f, 4, epsilon);
}


CETECH_FORCE_INLINE void quatf_from_axis_angle(quatf_t *__restrict result,
                                               const vec3f_t *__restrict axis,
                                               const float angle) {
    vec3f_t norm_axis;
    vec3f_normalized(&norm_axis, axis);

    const float angle_half = angle * 0.5f * CETECH_float_TORAD;
    const float sin = float_sin(angle_half);

    result->f[0] = sin * norm_axis.x;
    result->f[1] = sin * norm_axis.y;
    result->f[2] = sin * norm_axis.z;

    result->f[3] = float_cos(angle_half);
}

CETECH_FORCE_INLINE void quatf_from_euler(quatf_t *__restrict result,
                                          float heading,
                                          float attitude,
                                          float bank) {
    const float sx = float_sin(heading * 0.5f);
    const float sy = float_sin(attitude * 0.5f);
    const float sz = float_sin(bank * 0.5f);
    const float cx = float_cos(heading * 0.5f);
    const float cy = float_cos(attitude * 0.5f);
    const float cz = float_cos(bank * 0.5f);

    result->f[0] = sx * cy * cz - cx * sy * sz;
    result->f[1] = cx * sy * cz + sx * cy * sz;
    result->f[2] = cx * cy * sz - sx * sy * cz;
    result->f[3] = cx * cy * cz + sx * sy * sz;
}

CETECH_FORCE_INLINE void quatf_to_mat44f(mat44f_t *__restrict result,
                                         const quatf_t *__restrict a) {
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


CETECH_FORCE_INLINE void quatf_to_eurel_angle(vec3f_t *__restrict result,
                                              const quatf_t *__restrict a) {
    result->f[0] = float_atan2(2.0f * (a->f[0] * a->f[3] - a->f[1] * a->f[2]),
                               1.0f -
                               2.0f * (float_sq(a->f[0]) + float_sq(a->f[2])));

    result->f[1] = float_atan2(2.0f * (a->f[1] * a->f[3] + a->f[0] * a->f[2]),
                               1.0f -
                               2.0f * (float_sq(a->f[1]) + float_sq(a->f[2])));

    result->f[2] = float_asin(2.0f * (a->f[0] * a->f[1] + a->f[2] * a->f[3]));
}

CETECH_FORCE_INLINE int quatf_is_identity(const quatf_t *__restrict a,
                                          const float epsilon) {
    static quatf_t _identity = QUATF_IDENTITY;
    return quatf_eq(a, &_identity, epsilon);
}

CETECH_FORCE_INLINE void quatf_add(quatf_t *__restrict result,
                                   const quatf_t *__restrict a,
                                   const quatf_t *__restrict b) {
    result->f[0] = a->f[0] + b->f[0];
    result->f[1] = a->f[1] + b->f[1];
    result->f[2] = a->f[2] + b->f[2];
    result->f[3] = a->f[3] + b->f[3];
}

CETECH_FORCE_INLINE void quatf_sub(quatf_t *__restrict result,
                                   const quatf_t *__restrict a,
                                   const quatf_t *__restrict b) {
    result->f[0] = a->f[0] - b->f[0];
    result->f[1] = a->f[1] - b->f[1];
    result->f[2] = a->f[2] - b->f[2];
    result->f[3] = a->f[3] - b->f[3];
}

CETECH_FORCE_INLINE void quatf_neg(quatf_t *__restrict result,
                                   const quatf_t *__restrict a) {
    result->f[0] = -a->f[0];
    result->f[1] = -a->f[1];
    result->f[2] = -a->f[2];
    result->f[3] = -a->f[3];
}


CETECH_FORCE_INLINE void quatf_mul_s(quatf_t *__restrict result,
                                     const quatf_t *__restrict a,
                                     const float s) {
    result->f[0] = a->f[0] * s;
    result->f[1] = a->f[1] * s;
    result->f[2] = a->f[2] * s;
    result->f[3] = a->f[3] * s;
}

CETECH_FORCE_INLINE void quatf_div_s(quatf_t *__restrict result,
                                     const quatf_t *__restrict a,
                                     const float s) {
    result->f[0] = a->f[0] / s;
    result->f[1] = a->f[1] / s;
    result->f[2] = a->f[2] / s;
    result->f[3] = a->f[3] / s;
}

CETECH_FORCE_INLINE void quatf_mul(quatf_t *__restrict result,
                                   const quatf_t *__restrict a,
                                   const quatf_t *__restrict b) {

    result->f[0] = a->f[3] * b->f[0] + a->f[0] * b->f[3] + a->f[1] * b->f[2] -
                   a->f[2] * b->f[1];
    result->f[1] = a->f[3] * b->f[1] + a->f[1] * b->f[3] + a->f[2] * b->f[0] -
                   a->f[0] * b->f[2];
    result->f[2] = a->f[3] * b->f[2] + a->f[2] * b->f[3] + a->f[0] * b->f[1] -
                   a->f[1] * b->f[0];
    result->f[3] = a->f[3] * b->f[3] - a->f[0] * b->f[0] - a->f[1] * b->f[1] -
                   a->f[2] * b->f[2];
}

CETECH_FORCE_INLINE void quatf_mul_vec3f(vec3f_t *__restrict result,
                                         const quatf_t *__restrict a,
                                         const vec3f_t *__restrict b) {
    vec3f_t tmp_vec1, tmp_vec2, tmp_vec3;

    vec3f_cross(&tmp_vec1, (const vec3f_t *) a, b);
    vec3f_mul(&tmp_vec3, &tmp_vec1, 2.0f * a->f[3]);

    vec3f_mul(&tmp_vec1, b,
              float_sq(a->f[3]) - vec3f_dot((vec3f_t *) a, (vec3f_t *) a));
    vec3f_add(&tmp_vec2, &tmp_vec1, &tmp_vec3);

    vec3f_mul(&tmp_vec3, (const vec3f_t *) a,
              2.0f * vec3f_dot((vec3f_t *) a, b));
    vec3f_add(result, &tmp_vec2, &tmp_vec3);
}

CETECH_FORCE_INLINE float quatf_length_squared(const quatf_t *__restrict a) {
    return (a->f[0] * a->f[0]) + (a->f[1] * a->f[1]) + (a->f[2] * a->f[2]) +
           (a->f[3] * a->f[3]);
}

CETECH_FORCE_INLINE float quatf_length(const quatf_t *__restrict a) {
    return float_fast_sqrt(quatf_length_squared(a));
}

CETECH_FORCE_INLINE float quatf_inv_length(const quatf_t *__restrict a) {
    return float_fast_inv_sqrt(quatf_length_squared(a));
}

CETECH_FORCE_INLINE void quatf_normalized(quatf_t *__restrict result,
                                          const quatf_t *__restrict a) {
    quatf_mul_s(result, a, quatf_inv_length(a));
}

CETECH_FORCE_INLINE void quatf_invert(quatf_t *__restrict result,
                                      const quatf_t *__restrict a) {
    result->f[0] = -a->f[0];
    result->f[1] = -a->f[1];
    result->f[2] = -a->f[2];
    result->f[3] = a->f[3];
}

#endif //CETECH_QUATF_H
