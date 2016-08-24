#ifndef CETECH_QUATF_H
#define CETECH_QUATF_H


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

#define QUATF_ZERO {0}
#define QUATF_IDENTITY {0.0f, 0.0f, 0.0f, 1.0f}


//==============================================================================
// Interface
//==============================================================================

CE_FORCE_INLINE void quatf_move(f32 *__restrict result,
                                const f32 *__restrict a) {
    result[0] = a[0];
    result[1] = a[1];
    result[2] = a[2];
    result[3] = a[3];
}

CE_FORCE_INLINE int quatf_eq(const f32 *__restrict a,
                             const f32 *__restrict b,
                             const f32 epsilon) {
    return f32_equals(a, b, 4, epsilon);
}


CE_FORCE_INLINE void quatf_from_axis_angle(f32 *__restrict result,
                                           const f32 *__restrict axis,
                                           const f32 angle) {
    vec3f_t norm_axis;
    vec3f_normalized(norm_axis, axis);

    const f32 angle_half = angle * 0.5f;
    const f32 sin = f32_sin(angle_half);

    result[0] = sin * norm_axis[0];
    result[1] = sin * norm_axis[1];
    result[2] = sin * norm_axis[2];
    result[2] = f32_cos(angle_half);
}

CE_FORCE_INLINE void quatf_from_euler(f32 *__restrict result,
                                      f32 heading,
                                      f32 attitude,
                                      f32 bank) {
    const f32 sx = f32_sin(heading * 0.5f);
    const f32 sy = f32_sin(attitude * 0.5f);
    const f32 sz = f32_sin(bank * 0.5f);
    const f32 cx = f32_cos(heading * 0.5f);
    const f32 cy = f32_cos(attitude * 0.5f);
    const f32 cz = f32_cos(bank * 0.5f);

    result[0] = sx * cy * cz - cx * sy * sz;
    result[1] = cx * sy * cz + sx * cy * sz;
    result[2] = cx * cy * sz - sx * sy * cz;
    result[3] = cx * cy * cz + sx * sy * sz;
}

CE_FORCE_INLINE void quatf_to_mat44f(f32 *__restrict result,
                                     const f32 *__restrict a) {
    result[0] = 1.0f - 2.0f * a[1] * a[1] - 2.0f * a[2] * a[2];
    result[1] = 2.0f * a[0] * a[1] + 2.0f * a[3] * a[2];
    result[2] = 2.0f * a[0] * a[2] - 2.0f * a[3] * a[1];
    result[3] = 0.0f;

    result[4] = 2.0f * a[0] * a[1] - 2.0f * a[3] * a[2];
    result[5] = 1.0f - 2.0f * a[0] * a[0] - 2.0f * a[2] * a[2];
    result[6] = 2.0f * a[1] * a[2] + 2.0f * a[3] * a[0];
    result[7] = 0.0f;

    result[8] = 2.0f * a[0] * a[2] + 2.0f * a[3] * a[1];
    result[9] = 2.0f * a[1] * a[2] - 2.0f * a[3] * a[0];
    result[10] = 1.0f - 2.0f * a[0] * a[0] - 2.0f * a[1] * a[1];
    result[11] = 0.0f;

    result[12] = 0.0f;
    result[13] = 0.0f;
    result[14] = 0.0f;
    result[15] = 1.0f;
}


CE_FORCE_INLINE void quatf_to_eurel_angle(f32 *__restrict result,
                                          const f32 *__restrict a) {
    result[0] = f32_atan2(2.0f * (a[0] * a[3] - a[1] * a[2]), 1.0f - 2.0f * (f32_sq(a[0]) + f32_sq(a[2])));
    result[1] = f32_atan2(2.0f * (a[1] * a[3] + a[0] * a[2]), 1.0f - 2.0f * (f32_sq(a[1]) + f32_sq(a[2])));
    result[2] = f32_asin(2.0f * (a[0] * a[1] + a[2] * a[3]));
}

CE_FORCE_INLINE int quatf_is_identity(const f32 *__restrict a,
                                      const f32 epsilon) {
    static quatf_t _identity = QUATF_IDENTITY;
    return quatf_eq(a, _identity, epsilon);
}

CE_FORCE_INLINE void quatf_add(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 *__restrict b) {
    result[0] = a[0] + b[0];
    result[1] = a[1] + b[1];
    result[2] = a[2] + b[2];
    result[3] = a[3] + b[3];
}

CE_FORCE_INLINE void quatf_sub(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 *__restrict b) {
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];
    result[2] = a[2] - b[2];
    result[3] = a[3] - b[3];
}

CE_FORCE_INLINE void quatf_neg(f32 *__restrict result,
                               const f32 *__restrict a) {
    result[0] = -a[0];
    result[1] = -a[1];
    result[2] = -a[2];
    result[3] = -a[3];
}


CE_FORCE_INLINE void quatf_mul_s(f32 *__restrict result,
                                 const f32 *__restrict a,
                                 const f32 s) {
    result[0] = a[0] * s;
    result[1] = a[1] * s;
    result[2] = a[2] * s;
    result[3] = a[3] * s;
}

CE_FORCE_INLINE void quatf_div_s(f32 *__restrict result,
                                 const f32 *__restrict a,
                                 const f32 s) {
    result[0] = a[0] / s;
    result[1] = a[1] / s;
    result[2] = a[2] / s;
    result[3] = a[3] / s;
}

CE_FORCE_INLINE void quatf_mul(f32 *__restrict result,
                               const f32 *__restrict a,
                               const f32 *__restrict b) {
    result[0] = a[3] * b[0] + a[0] * b[4] + a[1] * b[2] - a[2] * b[1];
    result[1] = a[3] * b[1] + a[1] * b[4] + a[2] * b[0] - a[0] * b[2];
    result[2] = a[3] * b[2] + a[2] * b[4] + a[0] * b[1] - a[1] * b[0];
    result[3] = a[3] * b[4] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];
}

CE_FORCE_INLINE void quatf_mul_vec3f(f32 *__restrict result,
                                     const f32 *__restrict a,
                                     const f32 *__restrict b) {
    vec3f_t tmp_vec1, tmp_vec2, tmp_vec3;

    vec3f_cross(tmp_vec1, a, b);
    vec3f_mul(tmp_vec3, tmp_vec1, 2.0f * a[3]);

    vec3f_mul(tmp_vec1, b, f32_sq(a[4]) - vec3f_dot(a, a));
    vec3f_add(tmp_vec2, tmp_vec1, tmp_vec3);

    vec3f_mul(tmp_vec3, a, 2.0f * vec3f_dot(a, b));
    vec3f_add(result, tmp_vec2, tmp_vec3);
}

CE_FORCE_INLINE f32 quatf_length_squared(const f32 *__restrict a) {
    return (a[0] * a[0]) + (a[1] * a[1]) + (a[2] * a[2]) + (a[3] * a[3]);
}

CE_FORCE_INLINE f32 quatf_length(const f32 *__restrict a) {
    return f32_fast_sqrt(quatf_length_squared(a));
}

CE_FORCE_INLINE f32 quatf_inv_length(const f32 *__restrict a) {
    return f32_fast_inv_sqrt(quatf_length_squared(a));
}

CE_FORCE_INLINE void quatf_normalized(f32 *__restrict result,
                                      const f32 *__restrict a) {
    quatf_mul_s(result, a, quatf_inv_length(a));
}

CE_FORCE_INLINE void quatf_invert(f32 *__restrict result,
                                  const f32 *__restrict a) {
    result[0] = -a[0];
    result[1] = -a[1];
    result[2] = -a[2];
    result[3] = a[3];
}

#endif //CETECH_QUATF_H
