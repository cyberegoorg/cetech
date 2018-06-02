#ifndef CETECH_FMATH_H
#define CETECH_FMATH_H



#include <stdint.h>
#include <stdbool.h>

#define CT_PI             (3.1415926535897932384626433832795f)
#define CT_PI2            (6.2831853071795864769252867665590f)
#define CT_INV_PI         (1.0f / CT_PI)
#define CT_PI_HALF        (1.5707963267948966192313216916398f)
#define CT_SQRT2          (1.4142135623730950488016887242097f)
#define CT_INV_LOG_NAT_2  (1.4426950408889634073599246810019f)
#define CT_DEG_TO_RAD     (CT_PI / 180.0f)
#define CT_RAD_TO_DEG     (180.0f / CT_PI)

#if CT_COMPILER_MSVC
#define HUGEE = float(HUGE_VAL);
#else
#define HUGEE = HUGE_VALF;
#endif // CT_COMPILER_MSVC


float ct_fabsolute(float a);

float ct_fsin(float a);

float ct_fasin(float a);

float ct_fcos(float a);

float ct_ftan(float a);

float ct_facos(float a);

float ct_fatan2(float y,
                float x);

float ct_fpow(float a,
              float b);

float ct_flog(float a);

float ct_fsqrt(float a);

float ct_ffloor(float f);

float ct_fceil(float f);

float ct_fmod(float a,
              float b);

float ct_fround(float f);

float ct_fmin(float a,
              float b);

float ct_fmax(float a,
              float b);

float ct_fmin3(float a,
               float b,
               float c);

float ct_fmax3(float a,
               float b,
               float c);

float ct_fclamp(float a,
                float min,
                float max);

float ct_fsaturate(float a);

float ct_flerp(float a,
               float b,
               float t);

float ct_fsign(float a);

float ct_fsq(float a);

float ct_fexp2(float a);

float ct_flog2(float a);

float ct_frsqrt(float a);

float ct_ffract(float a);

bool ct_fequal(float a,
               float b,
               float epsilon);

bool ct_fequal_n(const float *a,
                 const float *b,
                 uint32_t num,
                 float epsilon);

float ct_fwrap(float a,
               float wrap);

float ct_fstep(float edge,
               float a);

float ct_fpulse(float a,
                float start,
                float end);

float ct_fsmoothstep(float a);

float ct_fbias(float time,
               float bias);

float ct_fgain(float time,
               float gain);

bool ct_is_nan(float f);

bool ct_is_nand(double f);

bool ct_is_finite(float f);

bool ct_is_finited(double f);

bool ct_is_infinite(float f);

bool ct_is_infinited(double f);

float ct_to_rad(float deg);

float ct_to_deg(float rad);

uint32_t ct_float_to_bits(float a);

float ct_bits_to_float(uint32_t a);

uint64_t ct_double_to_bits(double a);

double ct_bits_to_double(uint64_t a);

float ct_angle_diff(float a,
                    float b);

float ct_angle_lerp(float a,
                    float b,
                    float t);

void ct_vec3_move(float *result,
                  const float *a);

void ct_vec3_abs(float *result,
                 const float *a);

void ct_vec3_neg(float *result,
                 const float *a);

void ct_vec3_add(float *result,
                 const float *a,
                 const float *b);

void ct_vec3_add_s(float *result,
                   const float *a,
                   float b);

void ct_vec3_sub(float *result,
                 const float *a,
                 const float *b);

void ct_vec3_sub_s(float *result,
                   const float *a,
                   float b);

void ct_vec3_mul(float *result,
                 const float *a,
                 const float *b);

void ct_vec3_mul_s(float *result,
                   const float *a,
                   float b);

float ct_vec3_dot(const float *a,
                  const float *b);

void ct_vec3_cross(float *result,
                   const float *a,
                   const float *b);

float ct_vec3_length(const float *a);

void ct_vec3_lerp_s(float *result,
                    const float *a,
                    const float *b,
                    float t);

void ct_vec3_lerp(float *result,
                  const float *a,
                  const float *b,
                  const float *c);

float ct_vec3_norm(float *result,
                   const float *a);

void ct_vec3_min(float *result,
                 const float *a,
                 const float *b);

void ct_vec3_max(float *result,
                 const float *a,
                 const float *b);

void ct_vec3_rcp(float *result,
                 const float *a);

void ct_vec3_tangent_frame(const float *n,
                           float *t,
                           float *b);

void ct_vec3_tangent_frame_a(const float *n,
                             float *t,
                             float *b,
                             float angle);

void ct_vec3_from_lat_long(float *vec,
                           float u,
                           float v);

void ct_vec3_to_lat_long(float *u,
                         float *v,
                         const float *vec);

void ct_vec4_move(float *result,
                  const float *a);

void ct_quat_identity(float *result);

void ct_quat_move(float *result,
                  const float *a);

void ct_quat_mul_xyz(float *result,
                     const float *qa,
                     const float *qb);

void ct_quat_mul(float *result,
                 const float *qa,
                 const float *qb);

void ct_quat_invert(float *result,
                    const float *quat);

float ct_quat_dot(const float *a,
                  const float *b);

void ct_quat_norm(float *result,
                  const float *quat);

void ct_quat_to_euler(float *result,
                      const float *quat);

void ct_quat_from_euler(float *result,
                        float heading,
                        float attitude,
                        float bank);

void ct_quat_rotate_axis(float *result,
                         const float *axis,
                         float angle);

void ct_quat_rotate_x(float *result,
                      float ax);

void ct_quat_rotate_y(float *result,
                      float ay);

void ct_quat_rotate_z(float *result,
                      float az);

void ct_vec3_mul_quat(float *result,
                      const float *vec,
                      const float *quat);

void ct_mat4_identity(float *result);

void ct_mat4_translate(float *result,
                       float tx,
                       float ty,
                       float tz);

void ct_mat4_scale(float *result,
                   float sx,
                   float sy,
                   float sz);

void ct_mat4_scale_s(float *result,
                     float scale);

void ct_mat4_from_normal(float *result,
                         const float *normal,
                         float scale,
                         const float *pos);

void ct_mat4_from_normal_a(float *result,
                           const float *normal,
                           float scale,
                           const float *pos,
                           float angle);

void ct_mat4_move(float *_result,
                  const float *_a);

void ct_mat3_inverse(float *result,
                     const float *a);

void ct_mat4_inverse(float *result,
                     const float *a);

void ct_mat4_quat(float *result,
                  const float *quat);

void ct_mat4_quat_translation(float *result,
                              const float *quat,
                              const float *translation);

void ct_mat4_quat_translation_hmd(float *result,
                                  const float *quat,
                                  const float *translation);

void ct_mat4_rotate_x(float *result,
                      float ax);

void ct_mat4_rotate_y(float *result,
                      float ay);

void ct_mat4_rotate_z(float *result,
                      float az);

void ct_mat4_rotate_xy(float *result,
                       float ax,
                       float ay);

void ct_mat4_rotate_xyz(float *result,
                        float ax,
                        float ay,
                        float az);

void ct_mat4_rotate_zyx(float *result,
                        float ax,
                        float ay,
                        float az);

void ct_mat4_srt(float *result,
                 float sx,
                 float sy,
                 float sz,
                 float ax,
                 float ay,
                 float az,
                 float tx,
                 float ty,
                 float tz);

void ct_vec3_mul_mtx(float *result,
                     const float *vec,
                     const float *mat);

void ct_vec3_mul_mat4_h(float *result,
                        const float *vec,
                        const float *mat);

void ct_vec4_mul_mtx(float *result,
                     const float *vec,
                     const float *mat);

void ct_mat4_mul(float *result,
                 const float *a,
                 const float *b);

void ct_mat4_transpose(float *result,
                       const float *a);

void ct_mat4_proj_flip_handedness(float *dst,
                                  const float *src);

void ct_mat4_view_flip_handedness(float *dst,
                                  const float *src);

void ct_calc_normal(float *result,
                    float *va,
                    float *vb,
                    float *vc);

void ct_calc_plane(float *result,
                   float *va,
                   float *vb,
                   float *vc);

void ct_calc_linear_fit_2d(float *result,
                           const void *points,
                           uint32_t stride,
                           uint32_t numPoints);

void ct_calc_linear_fit_3d(float *result,
                           const void *points,
                           uint32_t stride,
                           uint32_t numPoints);

void ct_rgb_to_hsv(float *hsv,
                   const float *rgb);

void ct_hsv_to_rgb(float *rgb,
                   const float *hsv);


void ct_mat4_look_at_lh(float *result,
                        const float *eye,
                        const float *at,
                        const float *up);

void ct_mat4_look_at_rh(float *result,
                        const float *eye,
                        const float *at,
                        const float *up);

void ct_mat4_look_at(float *result,
                     const float *eye,
                     const float *at,
                     const float *up);


void ct_mat4_ortho(float *result,
                   float left,
                   float right,
                   float bottom,
                   float top,
                   float near,
                   float far,
                   float offset,
                   bool oglNdc);

void ct_mat4_ortho_lh(float *result,
                      float left,
                      float right,
                      float bottom,
                      float top,
                      float near,
                      float far,
                      float offset,
                      bool oglNdc);

void ct_mat4_ortho_rh(float *result,
                      float left,
                      float right,
                      float bottom,
                      float top,
                      float near,
                      float far,
                      float offset,
                      bool oglNdc);


void ct_mat4_proj(float *result,
                  float ut,
                  float dt,
                  float lt,
                  float rt,
                  float near,
                  float far,
                  bool oglNdc);


void ct_mat4_proj_fov(float *result,
                      const float *fov,
                      float near,
                      float far,
                      bool oglNdc);


void ct_mat4_proj_fovy(float *result,
                       float fovy,
                       float aspect,
                       float near,
                       float far,
                       bool oglNdc);


void ct_mat4_proj_lh(float *result,
                     float ut,
                     float dt,
                     float lt,
                     float rt,
                     float near,
                     float far,
                     bool oglNdc);


void ct_mat4_proj_lh_fov(float *result,
                         const float *fov,
                         float near,
                         float far,
                         bool oglNdc);

void ct_mat4_proj_lh_fovy(float *result,
                          float fovy,
                          float aspect,
                          float near,
                          float far,
                          bool oglNdc);


void ct_mat4_proj_rh(float *result,
                     float ut,
                     float dt,
                     float lt,
                     float rt,
                     float near,
                     float far,
                     bool oglNdc);

void ct_mat4_proj_rh_fov(float *result,
                         const float *fov,
                         float near,
                         float far,
                         bool oglNdc);

void ct_mat4_proj_rh_fovy(float *result,
                          float fovy,
                          float aspect,
                          float near,
                          float far,
                          bool oglNdc);


void ct_mat4_proj_inf_fov(float *result,
                          const float *fov,
                          float near,
                          bool oglNdc);


void ct_mat4_proj_inf(float *result,
                      float ut,
                      float dt,
                      float lt,
                      float rt,
                      float near,
                      bool oglNdc);


void ct_mat4_proj_inf_fovy(float *result,
                           float fovy,
                           float aspect,
                           float near,
                           bool oglNdc);


void ct_mat4_proj_inf_lh(float *result,
                         float ut,
                         float dt,
                         float lt,
                         float rt,
                         float near,
                         bool oglNdc);


void ct_mat4_proj_inf_lh_fov(float *result,
                             const float *fov,
                             float near,
                             bool oglNdc);


void ct_mat4_proj_inf_lh_fovy(float *result,
                              float fovy,
                              float aspect,
                              float near,
                              bool oglNdc);


void ct_mat4_proj_inf_rh(float *result,
                         float ut,
                         float dt,
                         float lt,
                         float rt,
                         float near,
                         bool oglNdc);


void ct_mat4_proj_inf_rh_fov(float *result,
                             const float *fov,
                             float near,
                             bool oglNdc);


void ct_mat4_proj_inf_rh_fovy(float *result,
                              float fovy,
                              float aspect,
                              float near,
                              bool oglNdc);


void ct_mat4_proj_rev_inf_lh(float *result,
                             float ut,
                             float dt,
                             float lt,
                             float rt,
                             float near,
                             bool oglNdc);


void ct_mat4_proj_rev_inf_lh_fov(float *result,
                                 const float *fov,
                                 float near,
                                 bool oglNdc);


void ct_mat4_proj_rev_inf_lh_fovy(float *result,
                                  float fovy,
                                  float aspect,
                                  float near,
                                  bool oglNdc);


void ct_mat4_proj_rev_inf_rh(float *result,
                             float ut,
                             float dt,
                             float lt,
                             float rt,
                             float near,
                             bool oglNdc);

void ct_mat4_proj_rev_inf_rh_fov(float *result,
                                 const float *fov,
                                 float near,
                                 bool oglNdc);

void ct_mat4_proj_rev_inf_rh_fovy(float *result,
                                  float fovy,
                                  float aspect,
                                  float near,
                                  bool oglNdc);


#endif //CETECH_FMATH_H
