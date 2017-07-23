/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef CELIB_FPUMATH_H
#define CELIB_FPUMATH_H

//==============================================================================
// Includes
//==============================================================================
#include <math.h>

#include "macros.h"
#include "memory.h"

namespace celib {
    const float PI = 3.1415926535897932384626433832795f;
    const float PI2 = 6.2831853071795864769252867665590f;
    const float INV_PI = 1.0f / PI;
    const float PI_HALF = 1.5707963267948966192313216916398f;
    const float SQRT2 = 1.4142135623730950488016887242097f;
    const float INV_LOG_NAT_2 = 1.4426950408889634073599246810019f;
    const float DEG_TO_RAD = PI / 180.0f;
    const float RAD_TO_DEG = 180.0f / PI;

#if CEL_COMPILER_MSVC
    const float HUGEE = float(HUGE_VAL);
#else
    const float HUGEE = HUGE_VALF;
#endif // CEL_COMPILER_MSVC

    ///
    struct Handness {
        enum Enum {
            Left,
            Right,
        };
    };

    ///
    struct NearFar {
        enum Enum {
            Default,
            Reverse,
        };
    };


    inline float fabsolute(float a);

    inline float fsin(float a);

    inline float fasin(float a);

    inline float fcos(float a);

    inline float ftan(float a);

    inline float facos(float a);

    inline float fatan2(float y,
                        float x);

    inline float fpow(float a,
                      float b);

    inline float flog(float a);

    inline float fsqrt(float a);

    inline float ffloor(float f);

    inline float fceil(float f);

    inline float fmod(float a,
                      float b);

    inline float fround(float f);

    inline float fmin(float a,
                      float b);

    inline float fmax(float a,
                      float b);

    inline float fmin3(float a,
                       float b,
                       float c);

    inline float fmax3(float a,
                       float b,
                       float c);

    inline float fclamp(float a,
                        float min,
                        float max);

    inline float fsaturate(float a);

    inline float flerp(float a,
                       float b,
                       float t);

    inline float fsign(float a);

    inline float fsq(float a);

    inline float fexp2(float a);

    inline float flog2(float a);

    inline float frsqrt(float a);

    inline float ffract(float a);

    inline bool fequal(float a,
                       float b,
                       float epsilon);

    inline bool fequal(const float *a,
                       const float *b,
                       uint32_t num,
                       float epsilon);

    inline float fwrap(float a,
                       float wrap);

    inline float fstep(float edge,
                       float a);

    inline float fpulse(float a,
                        float start,
                        float end);

    inline float fsmoothstep(float a);

    inline float fbias(float time,
                       float bias);

    inline float fgain(float time,
                       float gain);

    inline bool is_nan(float f);

    inline bool is_nan(double f);

    inline bool is_finite(float f);

    inline bool is_finite(double f);

    inline bool is_infinite(float f);

    inline bool is_infinite(double f);

    inline float to_rad(float deg);

    inline float to_deg(float rad);

    inline uint32_t float_to_bits(float a);

    inline float bits_to_float(uint32_t a);

    inline uint64_t double_to_bits(double a);

    inline double bits_to_double(uint64_t a);

    inline float angle_diff(float a,
                            float b);

    inline float angle_lerp(float a,
                            float b,
                            float t);

    inline void vec3_move(float *result,
                          const float *a);

    inline void vec3_abs(float *result,
                         const float *a);

    inline void vec3_neg(float *result,
                         const float *a);

    inline void vec3_add(float *result,
                         const float *a,
                         const float *b);

    inline void vec3_add(float *result,
                         const float *a,
                         float b);

    inline void vec3_sub(float *result,
                         const float *a,
                         const float *b);

    inline void vec3_sub(float *result,
                         const float *a,
                         float b);

    inline void vec3_mul(float *result,
                         const float *a,
                         const float *b);

    inline void vec3_mul(float *result,
                         const float *a,
                         float b);

    inline float vec3_dot(const float *a,
                          const float *b);

    inline void vec3_cross(float *result,
                           const float *a,
                           const float *b);

    inline float vec3_length(const float *a);

    inline void vec3_lerp(float *result,
                          const float *a,
                          const float *b,
                          float t);

    inline void vec3_lerp(float *result,
                          const float *a,
                          const float *b,
                          const float *c);

    inline float vec3_norm(float *result,
                           const float *a);

    inline void vec3_min(float *result,
                         const float *a,
                         const float *b);

    inline void vec3_max(float *result,
                         const float *a,
                         const float *b);

    inline void vec3_rcp(float *result,
                         const float *a);

    inline void vec3_tangent_frame(const float *n,
                                   float *t,
                                   float *b);

    inline void vec3_tangent_frame(const float *n,
                                   float *t,
                                   float *b,
                                   float angle);

    inline void vec3_from_lat_long(float *vec,
                                   float u,
                                   float v);

    inline void vec3_to_lat_long(float *u,
                                 float *v,
                                 const float *vec);

    inline void vec4_move(float *result,
                          const float *a);

    inline void quat_identity(float *result);

    inline void quat_move(float *result,
                          const float *a);

    inline void quat_mul_xyz(float *result,
                             const float *qa,
                             const float *qb);

    inline void quat_mul(float *result,
                         const float *qa,
                         const float *qb);

    inline void quat_invert(float *result,
                            const float *quat);

    inline float quat_dot(const float *a,
                          const float *b);

    inline void quat_norm(float *result,
                          const float *quat);

    inline void quat_to_euler(float *result,
                              const float *quat);

    inline void quat_rotate_axis(float *result,
                                 const float *axis,
                                 float angle);

    inline void quat_rotate_x(float *result,
                              float ax);

    inline void quat_rotate_y(float *result,
                              float ay);

    inline void quat_rotate_z(float *result,
                              float az);

    inline void vec3_mul_quat(float *result,
                              const float *vec,
                              const float *quat);

    inline void mat4_identity(float *result);

    inline void mat4_translate(float *result,
                              float tx,
                              float ty,
                              float tz);

    inline void mat4_scale(float *result,
                          float sx,
                          float sy,
                          float sz);

    inline void mat4_scale(float *result,
                          float scale);

    inline void mat4_from_normal(float *result,
                                const float *normal,
                                float scale,
                                const float *pos);

    inline void mat4_from_normal(float *result,
                                const float *normal,
                                float scale,
                                const float *pos,
                                float angle);

    inline void mat3_inverse(float *result,
                             const float *a);

    inline void mat4_inverse(float *result,
                            const float *a);

    inline void mat4_quat(float *result,
                         const float *quat);

    inline void mat4_quat_translation(float *result,
                                     const float *quat,
                                     const float *translation);

    inline void mat4_quat_translation_hmd(float *result,
                                         const float *quat,
                                         const float *translation);


    inline void mat4_look_at_lh(float *result,
                               const float *eye,
                               const float *at,
                               const float *up);

    inline void mat4_look_at_rh(float *result,
                               const float *eye,
                               const float *at,
                               const float *up);

    inline void mat4_look_at(float *result,
                            const float *eye,
                            const float *at,
                            const float *up);

    template<Handness::Enum HandnessT>
    inline void mat4_proj_xywh(float *result,
                              float x,
                              float y,
                              float width,
                              float height,
                              float near,
                              float far,
                              bool oglNdc);


    inline void mat4_proj(float *result,
                         float ut,
                         float dt,
                         float lt,
                         float rt,
                         float near,
                         float far,
                         bool oglNdc);


    inline void mat4_proj(float *result,
                         const float *fov,
                         float near,
                         float far,
                         bool oglNdc);


    inline void mat4_proj(float *result,
                         float fovy,
                         float aspect,
                         float near,
                         float far,
                         bool oglNdc);


    inline void mat4_proj_lh(float *result,
                            float ut,
                            float dt,
                            float lt,
                            float rt,
                            float near,
                            float far,
                            bool oglNdc);


    inline void mat4_proj_lh(float *result,
                            const float *fov,
                            float near,
                            float far,
                            bool oglNdc);

    inline void mat4_proj_lh(float *result,
                            float fovy,
                            float aspect,
                            float near,
                            float far,
                            bool oglNdc);


    inline void mat4_proj_rh(float *result,
                            float ut,
                            float dt,
                            float lt,
                            float rt,
                            float near,
                            float far,
                            bool oglNdc);

    inline void mat4_proj_rh(float *result,
                            const float *fov,
                            float near,
                            float far,
                            bool oglNdc);

    inline void mat4_proj_rh(float *result,
                            float fovy,
                            float aspect,
                            float near,
                            float far,
                            bool oglNdc);

    template<NearFar::Enum NearFarT, Handness::Enum HandnessT>
    inline void mat4_proj_inf_xywh(float *result,
                                  float x,
                                  float y,
                                  float width,
                                  float height,
                                  float near,
                                  bool oglNdc);


    inline void mat4_proj_inf(float *result,
                             const float *fov,
                             float near,
                             bool oglNdc);


    inline void mat4_proj_inf(float *result,
                             float ut,
                             float dt,
                             float lt,
                             float rt,
                             float near,
                             bool oglNdc);


    inline void mat4_proj_inf(float *result,
                             float fovy,
                             float aspect,
                             float near,
                             bool oglNdc);


    inline void mat4_proj_inf_lh(float *result,
                                float ut,
                                float dt,
                                float lt,
                                float rt,
                                float near,
                                bool oglNdc);


    inline void mat4_proj_inf_lh(float *result,
                                const float *fov,
                                float near,
                                bool oglNdc);


    inline void mat4_proj_inf_lh(float *result,
                                float fovy,
                                float aspect,
                                float near,
                                bool oglNdc);


    inline void mat4_proj_inf_rh(float *result,
                                float ut,
                                float dt,
                                float lt,
                                float rt,
                                float near,
                                bool oglNdc);


    inline void mat4_proj_inf_rh(float *result,
                                const float *fov,
                                float near,
                                bool oglNdc);


    inline void mat4_proj_inf_rh(float *result,
                                float fovy,
                                float aspect,
                                float near,
                                bool oglNdc);


    inline void mat4_proj_rev_inf_lh(float *result,
                                    float ut,
                                    float dt,
                                    float lt,
                                    float rt,
                                    float near,
                                    bool oglNdc);


    inline void mat4_proj_rev_inf_lh(float *result,
                                    const float *fov,
                                    float near,
                                    bool oglNdc);


    inline void mat4_proj_rev_inf_lh(float *result,
                                    float fovy,
                                    float aspect,
                                    float near,
                                    bool oglNdc);


    inline void mat4_proj_rev_inf_rh(float *result,
                                    float ut,
                                    float dt,
                                    float lt,
                                    float rt,
                                    float near,
                                    bool oglNdc);

    inline void mat4_proj_rev_inf_rh(float *result,
                                    const float *fov,
                                    float near,
                                    bool oglNdc);

    inline void mat4_proj_rev_inf_rh(float *result,
                                    float fovy,
                                    float aspect,
                                    float near,
                                    bool oglNdc);

    inline void mat4_ortho(float *result,
                          float left,
                          float right,
                          float bottom,
                          float top,
                          float near,
                          float far,
                          float offset,
                          bool oglNdc);

    inline void mat4_ortho_lh(float *result,
                             float left,
                             float right,
                             float bottom,
                             float top,
                             float near,
                             float far,
                             float offset,
                             bool oglNdc);

    inline void mat4_ortho_rh(float *result,
                             float left,
                             float right,
                             float bottom,
                             float top,
                             float near,
                             float far,
                             float offset,
                             bool oglNdc);


    inline void mat4_rotate_x(float *result,
                             float ax);

    inline void mat4_rotate_y(float *result,
                             float ay);

    inline void mat4_rotate_z(float *result,
                             float az);

    inline void mat4_rotate_xy(float *result,
                              float ax,
                              float ay);

    inline void mat4_rotate_xyz(float *result,
                               float ax,
                               float ay,
                               float az);

    inline void mat4_rotate_zyx(float *result,
                               float ax,
                               float ay,
                               float az);

    inline void mat4_srt(float *result,
                        float sx,
                        float sy,
                        float sz,
                        float ax,
                        float ay,
                        float az,
                        float tx,
                        float ty,
                        float tz);

    inline void vec3_mul_mtx(float *result,
                             const float *vec,
                             const float *mat);

    inline void vec3_mul_mat4_h(float *result,
                               const float *vec,
                               const float *mat);

    inline void vec4_mul_mtx(float *result,
                             const float *vec,
                             const float *mat);

    inline void mat4_mul(float *result,
                        const float *a,
                        const float *b);

    inline void mat4_transpose(float *result,
                              const float *a);

    /// Convert LH to RH projection matrix and vice versa.
    inline void mat4_proj_flip_handedness(float *dst,
                                         const float *src);

    /// Convert LH to RH view matrix and vice versa.
    inline void mat4_view_flip_handedness(float *dst,
                                         const float *src);

    inline void calc_normal(float *result,
                            float *va,
                            float *vb,
                            float *vc);

    inline void calc_plane(float *result,
                           float *va,
                           float *vb,
                           float *vc);

    inline void calc_linear_fit_2d(float *result,
                                   const void *points,
                                   uint32_t stride,
                                   uint32_t numPoints);

    inline void calc_linear_fit_3d(float *result,
                                   const void *points,
                                   uint32_t stride,
                                   uint32_t numPoints);

    inline void rgb_to_hsv(float *hsv,
                           const float *rgb);

    inline void hsv_to_rgb(float *rgb,
                           const float *hsv);
}

#include "celib/private/fpumath.inl"

#endif //CELIB_FPUMATH_H
