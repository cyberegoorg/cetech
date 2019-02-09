//                          **FPU math**
//

/* Bassed on bx math lib. license ---VVV
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef CE_FMATH_H
#define CE_FMATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

#include <math.h>
#include <string.h>

// # Constant

// $$ \pi $$
#define CE_PI             (3.1415926535897932384626433832795f)

// $$ \pi * 2 $$
#define CE_PI2            (6.2831853071795864769252867665590f)

// $$ \frac{1}{\pi} $$
#define CE_INV_PI         (1.0f / CE_PI)

// $$ \frac{\pi}{2} $$
#define CE_PI_HALF        (1.5707963267948966192313216916398f)

// $$ \frac{\pi}{2} $$
#define CE_SQRT2          (1.4142135623730950488016887242097f)

#define CE_INV_LOG_NAT_2  (1.4426950408889634073599246810019f)

// $$ \frac{\pi}{180} $$
#define CE_DEG_TO_RAD     (CE_PI / 180.0f)

// $$ \frac{180}{\pi} $$
#define CE_RAD_TO_DEG     (180.0f / CE_PI)

#if CE_COMPILER_MSVC
#define HUGEE float(HUGE_VAL);
#else
#define HUGEE HUGE_VALF;
#endif // CE_COMPILER_MSVC


// # Float

// Convert float -> uint32_t
static inline uint32_t ce_float_to_bits(float _a) {
    union {
        float f;
        uint32_t ui;
    } u = {_a};
    return u.ui;
}

// Convert uint32_t -> float
static inline float ce_bits_to_float(uint32_t _a) {
    union {
        uint32_t ui;
        float f;
    } u = {_a};
    return u.f;
}

// Convert double -> uint32_t
static inline uint64_t ce_double_to_bits(double _a) {
    union {
        double f;
        uint64_t ui;
    } u = {_a};
    return u.ui;
}

// Convert uint32_t -> double
static inline double ce_bits_to_double(uint64_t _a) {
    union {
        uint64_t ui;
        double f;
    } u = {_a};
    return u.f;
}


// $$ \|a\|  $$
static inline float ce_fabsolute(float _a) {
    return fabsf(_a);
}

static inline float ce_fpow(float _a,
                            float _b) {
    return powf(_a, _b);
}

static inline float ce_flog(float _a) {
    return logf(_a);
}


static inline float ce_fsqrt(float _a) {
    return sqrtf(_a);
}

static inline float ce_ffloor(float _f) {
    return floorf(_f);
}

static inline float ce_fceil(float _f) {
    return ceilf(_f);
}

static inline float ce_fmod(float _a,
                            float _b) {
    return fmodf(_a, _b);
}

static inline bool ce_is_nan(float _f) {
    const uint32_t tmp = ce_float_to_bits(_f) & INT32_MAX;
    return tmp > UINT32_C(0x7f800000);
}

static inline bool ce_is_nand(double _f) {
    const uint64_t tmp = ce_double_to_bits(_f) & INT64_MAX;
    return tmp > UINT64_C(0x7ff0000000000000);
}

static inline bool ce_is_finite(float _f) {
    const uint32_t tmp = ce_float_to_bits(_f) & INT32_MAX;
    return tmp < UINT32_C(0x7f800000);
}

static inline bool ce_is_finited(double _f) {
    const uint64_t tmp = ce_double_to_bits(_f) & INT64_MAX;
    return tmp < UINT64_C(0x7ff0000000000000);
}

static inline bool ce_is_infinite(float _f) {
    const uint32_t tmp = ce_float_to_bits(_f) & INT32_MAX;
    return tmp == UINT32_C(0x7f800000);
}

static inline bool ce_is_infinited(double _f) {
    const uint64_t tmp = ce_double_to_bits(_f) & INT64_MAX;
    return tmp == UINT64_C(0x7ff0000000000000);
}

static inline float ce_fround(float _f) {
    return ce_ffloor(_f + 0.5f);
}

static inline float ce_fmin(float _a,
                            float _b) {
    return _a < _b ? _a : _b;
}

static inline float ce_fmax(float _a,
                            float _b) {
    return _a > _b ? _a : _b;
}

static inline float ce_fmin3(float _a,
                             float _b,
                             float _c) {
    return ce_fmin(_a, ce_fmin(_b, _c));
}

static inline float ce_fmax3(float _a,
                             float _b,
                             float _c) {
    return ce_fmax(_a, ce_fmax(_b, _c));
}

static inline float ce_fclamp(float _a,
                              float _min,
                              float _max) {
    return ce_fmin(ce_fmax(_a, _min), _max);
}

static inline float ce_fsaturate(float _a) {
    return ce_fclamp(_a, 0.0f, 1.0f);
}

static inline float ce_flerp(float _a,
                             float _b,
                             float _t) {
    return _a + (_b - _a) * _t;
}

static inline float ce_fsign(float _a) {
    return _a < 0.0f ? -1.0f : 1.0f;
}

static inline float ce_fsq(float _a) {
    return _a * _a;
}

static inline float ce_fexp2(float _a) {
    return ce_fpow(2.0f, _a);
}

static inline float ce_flog2(float _a) {
    return ce_flog(_a) * CE_INV_LOG_NAT_2;
}

static inline float ce_frsqrt(float _a) {
    return 1.0f / ce_fsqrt(_a);
}

static inline float ce_ffract(float _a) {
    return _a - ce_ffloor(_a);
}

static inline bool ce_fequal(float _a,
                             float _b,
                             float _epsilon) {
    // http://realtimecollisiondetection.net/blog/?p=89
    const float lhs = ce_fabsolute(_a - _b);
    const float rhs =
            _epsilon * ce_fmax3(1.0f, ce_fabsolute(_a), ce_fabsolute(_b));
    return lhs <= rhs;
}

static inline bool ce_fequal_n(const float *_a,
                               const float *_b,
                               uint32_t _num,
                               float _epsilon) {
    bool equal = ce_fequal(_a[0], _b[0], _epsilon);
    for (uint32_t ii = 1; equal && ii < _num; ++ii) {
        equal = ce_fequal(_a[ii], _b[ii], _epsilon);
    }
    return equal;
}

static inline float ce_fwrap(float _a,
                             float _wrap) {
    const float mod = ce_fmod(_a, _wrap);
    const float result = mod < 0.0f ? _wrap + mod : mod;
    return result;
}

static inline float ce_fstep(float _edge,
                             float _a) {
    return _a < _edge ? 0.0f : 1.0f;
}

static inline float ce_fpulse(float _a,
                              float _start,
                              float _end) {
    return ce_fstep(_a, _start) - ce_fstep(_a, _end);
}

static inline float ce_fsmoothstep(float _a) {
    return ce_fsq(_a) * (3.0f - 2.0f * _a);
}

static inline float ce_fbias(float _time,
                             float _bias) {
    return _time / (((1.0f / _bias - 2.0f) * (1.0f - _time)) + 1.0f);
}

static inline float ce_fgain(float _time,
                             float _gain) {
    if (_time < 0.5f) {
        return ce_fbias(_time * 2.0f, _gain) * 0.5f;
    }

    return ce_fbias(_time * 2.0f - 1.0f, 1.0f - _gain) * 0.5f + 0.5f;
}


// # Goniometry

static inline float ce_fsin(float _a) {
    return sinf(_a);
}

static inline float ce_fasin(float _a) {
    return asinf(_a);
}

static inline float ce_fcos(float _a) {
    return cosf(_a);
}

static inline float ce_ftan(float _a) {
    return tanf(_a);
}

static inline float ce_facos(float _a) {
    return acosf(_a);
}

static inline float ce_fatan2(float _y,
                              float _x) {
    return atan2f(_y, _x);
}

// # Angle

// Convert degree -> radian
static inline float ce_to_rad(float _deg) {
    return _deg * CE_DEG_TO_RAD;
}

// Convert radian -> degree
static inline float ce_to_deg(float _rad) {
    return _rad * CE_RAD_TO_DEG;
}


// Calc angle diff
static inline float ce_angle_diff(float _a,
                                  float _b) {
    const float dist = ce_fwrap(_b - _a, CE_PI * 2.0f);
    return ce_fwrap(dist * 2.0f, CE_PI * 2.0f) - dist;
}

// LERP
static inline float ce_angle_lerp(float _a,
                                  float _b,
                                  float _t) {
    return _a + ce_angle_diff(_a, _b) * _t;
}

// # Vec2
static inline ce_vec2_t ce_vec2_abs(ce_vec2_t a) {
    return (ce_vec2_t) {
            .x = ce_fabsolute(a.x),
            .y = ce_fabsolute(a.y),
    };
}

static inline ce_vec2_t ce_vec2_neg(ce_vec2_t a) {
    return (ce_vec2_t) {
            .x = -a.x,
            .y = -a.y,
    };
}

static inline ce_vec2_t ce_vec2_add(ce_vec2_t _a,
                                    ce_vec2_t _b) {
    return (ce_vec2_t) {
            .x = _a.x + _b.x,
            .y = _a.y + _b.y,
    };
}

static inline ce_vec2_t ce_vec2_add_s(ce_vec2_t _a,
                                      float _b) {
    return (ce_vec2_t) {
            .x = _a.x + _b,
            .y = _a.y + _b,
    };
}

static inline ce_vec2_t ce_vec2_sub(ce_vec2_t _a,
                                    ce_vec2_t _b) {
    return (ce_vec2_t) {
            .x = _a.x - _b.x,
            .y = _a.y - _b.y,
    };
}

static inline ce_vec2_t ce_vec2_sub_s(ce_vec2_t _a,
                                      float _b) {
    return (ce_vec2_t) {
            .x = _a.x - _b,
            .y = _a.y - _b,
    };
}

static inline ce_vec2_t ce_vec2_mul(ce_vec2_t _a,
                                    ce_vec2_t _b) {
    return (ce_vec2_t) {
            .x = _a.x * _b.x,
            .y = _a.y * _b.y,
    };
}

static inline ce_vec2_t ce_vec2_mul_s(ce_vec2_t _a,
                                      float _b) {
    return (ce_vec2_t) {
            .x = _a.x * _b,
            .y = _a.y * _b,
    };
}

static inline float ce_vec2_dot(ce_vec2_t _a,
                                ce_vec2_t _b) {
    return _a.x * _b.x + _a.y * _b.y;
}

static inline float ce_vec2_length(ce_vec2_t _a) {
    return ce_fsqrt(ce_vec2_dot(_a, _a));
}

static inline ce_vec2_t ce_vec2_lerp_s(ce_vec2_t _a,
                                       ce_vec2_t _b,
                                       float _t) {
    return (ce_vec2_t) {
            .x = ce_flerp(_a.x, _b.x, _t),
            .y = ce_flerp(_a.y, _b.y, _t),
    };


}

static inline ce_vec2_t ce_vec2_lerp(ce_vec2_t _a,
                                     ce_vec2_t _b,
                                     ce_vec2_t _c) {
    return (ce_vec2_t) {
            .x = ce_flerp(_a.x, _b.x, _c.x),
            .y = ce_flerp(_a.y, _b.y, _c.y),
    };

}

static inline ce_vec2_t ce_vec2_norm(ce_vec2_t _a) {
    const float len = ce_vec2_length(_a);
    const float invLen = 1.0f / len;
    return ce_vec2_mul_s(_a, invLen);
}

static inline ce_vec2_t ce_vec2_min(ce_vec2_t _a,
                                    ce_vec2_t _b) {
    return (ce_vec2_t) {
            .x = fmin(_a.x, _b.x),
            .y = fmin(_a.y, _b.y),
    };
}

static inline ce_vec2_t ce_vec2_max(ce_vec2_t _a,
                                    ce_vec2_t _b) {

    return (ce_vec2_t) {
            .x = fmax(_a.x, _b.x),
            .y = fmax(_a.y, _b.y),
    };
}

static inline ce_vec2_t ce_vec2_rcp(ce_vec2_t _a) {
    return (ce_vec2_t) {
            .x = 1.0f / _a.x,
            .y = 1.0f / _a.y,
    };
}

// # Vec3
static inline ce_vec3_t ce_vec3_abs(ce_vec3_t a) {
    return (ce_vec3_t) {
            .x = ce_fabsolute(a.x),
            .y = ce_fabsolute(a.y),
            .z = ce_fabsolute(a.z),
    };
}

static inline ce_vec3_t ce_vec3_neg(ce_vec3_t a) {
    return (ce_vec3_t) {
            .x = -a.x,
            .y = -a.y,
            .z = -a.z,
    };
}

static inline ce_vec3_t ce_vec3_add(ce_vec3_t _a,
                                    ce_vec3_t _b) {
    return (ce_vec3_t) {
            .x = _a.x + _b.x,
            .y = _a.y + _b.y,
            .z = _a.z + _b.z,
    };
}

static inline ce_vec3_t ce_vec3_add_s(ce_vec3_t _a,
                                      float _b) {
    return (ce_vec3_t) {
            .x = _a.x + _b,
            .y = _a.y + _b,
            .z = _a.z + _b,
    };
}

static inline ce_vec3_t ce_vec3_sub(ce_vec3_t _a,
                                    ce_vec3_t _b) {
    return (ce_vec3_t) {
            .x = _a.x - _b.x,
            .y = _a.y - _b.y,
            .z = _a.z - _b.z,
    };
}

static inline ce_vec3_t ce_vec3_sub_s(ce_vec3_t _a,
                                      float _b) {
    return (ce_vec3_t) {
            .x = _a.x - _b,
            .y = _a.y - _b,
            .z = _a.z - _b,
    };
}

static inline ce_vec3_t ce_vec3_mul(ce_vec3_t _a,
                                    ce_vec3_t _b) {
    return (ce_vec3_t) {
            .x = _a.x * _b.x,
            .y = _a.y * _b.y,
            .z = _a.z * _b.z,
    };
}

static inline ce_vec3_t ce_vec3_mul_s(ce_vec3_t _a,
                                      float _b) {
    return (ce_vec3_t) {
            .x = _a.x * _b,
            .y = _a.y * _b,
            .z = _a.z * _b,
    };
}

static inline float ce_vec3_dot(ce_vec3_t _a,
                                ce_vec3_t _b) {
    return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z;
}

static inline ce_vec3_t ce_vec3_cross(ce_vec3_t _a,
                                      ce_vec3_t _b) {
    return (ce_vec3_t) {
            .x = _a.y * _b.z - _a.z * _b.y,
            .y = _a.z * _b.x - _a.x * _b.z,
            .z = _a.x * _b.y - _a.y * _b.x,
    };

}

static inline float ce_vec3_length(ce_vec3_t _a) {
    return ce_fsqrt(ce_vec3_dot(_a, _a));
}

static inline ce_vec3_t ce_vec3_lerp_s(ce_vec3_t _a,
                                       ce_vec3_t _b,
                                       float _t) {
    return (ce_vec3_t) {
            .x = ce_flerp(_a.x, _b.x, _t),
            .y = ce_flerp(_a.y, _b.y, _t),
            .z = ce_flerp(_a.z, _b.z, _t),
    };


}

static inline ce_vec3_t ce_vec3_lerp(ce_vec3_t _a,
                                     ce_vec3_t _b,
                                     ce_vec3_t _c) {
    return (ce_vec3_t) {
            .x = ce_flerp(_a.x, _b.x, _c.x),
            .y = ce_flerp(_a.y, _b.y, _c.y),
            .z = ce_flerp(_a.z, _b.z, _c.z),
    };

}

static inline ce_vec3_t ce_vec3_norm(ce_vec3_t _a) {
    const float len = ce_vec3_length(_a);
    const float invLen = 1.0f / len;
    return ce_vec3_mul_s(_a, invLen);
}

static inline ce_vec3_t ce_vec3_min(ce_vec3_t _a,
                                    ce_vec3_t _b) {
    return (ce_vec3_t) {
            .x = fmin(_a.x, _b.x),
            .y = fmin(_a.y, _b.y),
            .z = fmin(_a.z, _b.z),
    };
}

static inline ce_vec3_t ce_vec3_max(ce_vec3_t _a,
                                    ce_vec3_t _b) {

    return (ce_vec3_t) {
            .x = fmax(_a.x, _b.x),
            .y = fmax(_a.y, _b.y),
            .z = fmax(_a.z, _b.z),
    };
}

static inline ce_vec3_t ce_vec3_rcp(ce_vec3_t _a) {
    return (ce_vec3_t) {
            .x = 1.0f / _a.x,
            .y = 1.0f / _a.y,
            .z = 1.0f / _a.z,
    };
}

static inline void ce_vec3_tangent_frame(ce_vec3_t _n,
                                         ce_vec3_t *_t,
                                         ce_vec3_t *_b) {
    const float nx = _n.x;
    const float ny = _n.y;
    const float nz = _n.z;

    if (ce_fabsolute(nx) > ce_fabsolute(nz)) {
        float invLen = 1.0f / ce_fsqrt(nx * nx + nz * nz);
        _t->x = -nz * invLen;
        _t->y = 0.0f;
        _t->z = nx * invLen;
    } else {
        float invLen = 1.0f / ce_fsqrt(ny * ny + nz * nz);
        _t->x = 0.0f;
        _t->y = nz * invLen;
        _t->z = -ny * invLen;
    }

    *_b = ce_vec3_cross(_n, *_t);
}

static inline void ce_vec3_tangent_frame_a(ce_vec3_t _n,
                                           ce_vec3_t *_t,
                                           ce_vec3_t *_b,
                                           float _angle) {
    ce_vec3_tangent_frame(_n, _t, _b);

    const float sa = ce_fsin(_angle);
    const float ca = ce_fcos(_angle);

    _t->x = -sa * _b->x + ca * _t->x;
    _t->y = -sa * _b->y + ca * _t->y;
    _t->z = -sa * _b->z + ca * _t->z;

    *_b = ce_vec3_cross(_n, *_t);
}

static inline void ce_vec3_from_lat_long(ce_vec3_t _vec,
                                         float _u,
                                         float _v) {
    const float phi = _u * CE_PI2;
    const float theta = _v * CE_PI;

    const float st = ce_fsin(theta);
    const float sp = ce_fsin(phi);
    const float ct = ce_fcos(theta);
    const float cp = ce_fcos(phi);

    _vec.x = -st * sp;
    _vec.y = ct;
    _vec.z = -st * cp;
}

static inline void ce_vec3_to_lat_long(float *_u,
                                       float *_v,
                                       ce_vec3_t _vec) {
    const float phi = ce_fatan2(_vec.x, _vec.z);
    const float theta = ce_facos(_vec.y);

    *_u = (CE_PI + phi) * CE_INV_PI * 0.5f;
    *_v = theta * CE_INV_PI;
}

// # Quaternion

static const ce_vec4_t ce_quat_identity = {.x = 0, .y=0, .z=0, .w=1.0f};

static inline ce_vec3_t ce_quat_mul_xyz(ce_vec4_t _qa,
                                        ce_vec4_t _qb) {
    const float ax = _qa.x;
    const float ay = _qa.y;
    const float az = _qa.z;
    const float aw = _qa.w;

    const float bx = _qb.x;
    const float by = _qb.y;
    const float bz = _qb.z;
    const float bw = _qb.w;

    return (ce_vec3_t) {
            .x = aw * bx + ax * bw + ay * bz - az * by,
            .y = aw * by - ax * bz + ay * bw + az * bx,
            .z = aw * bz + ax * by - ay * bx + az * bw,
    };
}

static inline ce_vec4_t ce_quat_mul(ce_vec4_t _qa,
                                    ce_vec4_t _qb) {
    const float ax = _qa.x;
    const float ay = _qa.y;
    const float az = _qa.z;
    const float aw = _qa.w;

    const float bx = _qb.x;
    const float by = _qb.y;
    const float bz = _qb.z;
    const float bw = _qb.w;

    return (ce_vec4_t) {
            .x = aw * bx + ax * bw + ay * bz - az * by,
            .y = aw * by - ax * bz + ay * bw + az * bx,
            .z = aw * bz + ax * by - ay * bx + az * bw,
            .w = aw * bw - ax * bx - ay * by - az * bz,
    };
}

static inline ce_vec4_t ce_quat_invert(ce_vec4_t _quat) {
    return (ce_vec4_t) {
            .x= -_quat.x,
            .y= -_quat.y,
            .z= -_quat.z,
            .w= _quat.w,
    };
}

static inline float ce_quat_dot(ce_vec4_t _a,
                                ce_vec4_t _b) {
    return _a.x * _b.x
           + _a.y * _b.y
           + _a.z * _b.z
           + _a.w * _b.w;
}

static inline ce_vec4_t ce_quat_norm(ce_vec4_t _quat) {
    const float norm = ce_quat_dot(_quat, _quat);
    if (0.0f < norm) {
        const float invNorm = 1.0f / ce_fsqrt(norm);

        return (ce_vec4_t) {
                .x = _quat.x * invNorm,
                .y = _quat.y * invNorm,
                .z = _quat.z * invNorm,
                .w = _quat.w * invNorm,
        };
    }

    return ce_quat_identity;
}

static inline ce_vec3_t ce_quat_to_euler(ce_vec4_t _quat) {
    const float x = _quat.x;
    const float y = _quat.y;
    const float z = _quat.z;
    const float w = _quat.w;

    const float yy = y * y;
    const float zz = z * z;
    const float xx = x * x;

    return (ce_vec3_t) {
            .x = ce_fatan2(2.0f * (x * w - y * z),
                           1.0f - 2.0f * (xx + zz)),

            .y = ce_fatan2(2.0f * (y * w + x * z),
                           1.0f - 2.0f * (yy + zz)),

            .z = ce_fasin(2.0f * (x * y + z * w)),
    };

}

static inline ce_vec4_t ce_quat_from_euler(float heading,
                                           float attitude,
                                           float bank) {

    const float sx = ce_fsin(heading * 0.5f);
    const float sy = ce_fsin(attitude * 0.5f);
    const float sz = ce_fsin(bank * 0.5f);
    const float cx = ce_fcos(heading * 0.5f);
    const float cy = ce_fcos(attitude * 0.5f);
    const float cz = ce_fcos(bank * 0.5f);

    return (ce_vec4_t) {
            .x  = sx * cy * cz - cx * sy * sz,
            .y  = cx * sy * cz + sx * cy * sz,
            .z  = cx * cy * sz - sx * sy * cz,
            .w  = cx * cy * cz + sx * sy * sz,
    };
}

static inline void ce_quat_rotate_axis(float *_result,
                                       const float *_axis,
                                       float _angle) {
    const float ha = _angle * 0.5f;
    const float ca = ce_fcos(ha);
    const float sa = ce_fsin(ha);
    _result[0] = _axis[0] * sa;
    _result[1] = _axis[1] * sa;
    _result[2] = _axis[2] * sa;
    _result[3] = ca;
}

static inline void ce_quat_rotate_x(float *_result,
                                    float _ax) {
    const float hx = _ax * 0.5f;
    const float cx = ce_fcos(hx);
    const float sx = ce_fsin(hx);
    _result[0] = sx;
    _result[1] = 0.0f;
    _result[2] = 0.0f;
    _result[3] = cx;
}

static inline void ce_quat_rotate_y(float *_result,
                                    float _ay) {
    const float hy = _ay * 0.5f;
    const float cy = ce_fcos(hy);
    const float sy = ce_fsin(hy);
    _result[0] = 0.0f;
    _result[1] = sy;
    _result[2] = 0.0f;
    _result[3] = cy;
}

static inline void ce_quat_rotate_z(float *_result,
                                    float _az) {
    const float hz = _az * 0.5f;
    const float cz = ce_fcos(hz);
    const float sz = ce_fsin(hz);
    _result[0] = 0.0f;
    _result[1] = 0.0f;
    _result[2] = sz;
    _result[3] = cz;
}

static inline ce_vec3_t ce_vec3_mul_quat(ce_vec3_t _vec,
                                         ce_vec4_t _quat) {
    ce_vec4_t inv_q = ce_quat_invert(_quat);

    ce_vec4_t qv = {
            .x = _vec.x,
            .y = _vec.y,
            .z = _vec.z,
            .w = 0.0f,
    };

    ce_vec4_t iqv = ce_quat_mul(inv_q, qv);
    return ce_quat_mul_xyz(iqv, _quat);
}

// # Vec4


// # Mat 3

static inline void ce_mat3_inverse(float *_result,
                                   const float *_a) {
    float xx = _a[0];
    float xy = _a[1];
    float xz = _a[2];
    float yx = _a[3];
    float yy = _a[4];
    float yz = _a[5];
    float zx = _a[6];
    float zy = _a[7];
    float zz = _a[8];

    float det = 0.0f;
    det += xx * (yy * zz - yz * zy);
    det -= xy * (yx * zz - yz * zx);
    det += xz * (yx * zy - yy * zx);

    float invDet = 1.0f / det;

    _result[0] = +(yy * zz - yz * zy) * invDet;
    _result[1] = -(xy * zz - xz * zy) * invDet;
    _result[2] = +(xy * yz - xz * yy) * invDet;

    _result[3] = -(yx * zz - yz * zx) * invDet;
    _result[4] = +(xx * zz - xz * zx) * invDet;
    _result[5] = -(xx * yz - xz * yx) * invDet;

    _result[6] = +(yx * zy - yy * zx) * invDet;
    _result[7] = -(xx * zy - xy * zx) * invDet;
    _result[8] = +(xx * yy - xy * yx) * invDet;
}

// # Mat 4

static inline void ce_mat4_identity(float *_result) {
    memset(_result, 0, sizeof(float) * 16);
    _result[0] = _result[5] = _result[10] = _result[15] = 1.0f;
}

static inline bool ce_mat4_is_identity(float *_result) {
    return (_result[0] == 1.0f) && (_result[1] == 0.0f) &&
           (_result[2] == 0.0f) && (_result[3] == 0.0f) &&
           (_result[4] == 0.0f) && (_result[5] == 1.0f) &&
           (_result[6] == 0.0f) && (_result[7] == 0.0f) &&
           (_result[8] == 0.0f) && (_result[9] == 0.0f) &&
           (_result[10] == 1.0f) && (_result[11] == 0.0f) &&
           (_result[12] == 0.0f) && (_result[13] == 0.0f) &&
           (_result[14] == 0.0f) && (_result[15] == 1.0f);

}


static inline void ce_mat4_translate(float *_result,
                                     float _tx,
                                     float _ty,
                                     float _tz) {
    ce_mat4_identity(_result);
    _result[12] = _tx;
    _result[13] = _ty;
    _result[14] = _tz;
}

static inline void ce_mat4_scale(float *_result,
                                 float _sx,
                                 float _sy,
                                 float _sz) {
    memset(_result, 0, sizeof(float) * 16);
    _result[0] = _sx;
    _result[5] = _sy;
    _result[10] = _sz;
    _result[15] = 1.0f;
}

static inline void ce_mat4_scale_s(float *_result,
                                   float _scale) {
    ce_mat4_scale(_result, _scale, _scale, _scale);
}

static inline void ce_mat4_from_normal(float *_result,
                                       ce_vec3_t _normal,
                                       float _scale,
                                       ce_vec3_t _pos) {
    ce_vec3_t tangent;
    ce_vec3_t bitangent;

    ce_vec3_tangent_frame(_normal, &tangent, &bitangent);

    *(ce_vec3_t *) (&_result[0]) = ce_vec3_mul_s(bitangent, _scale);
    *(ce_vec3_t *) (&_result[4]) = ce_vec3_mul_s(_normal, _scale);
    *(ce_vec3_t *) (&_result[8]) = ce_vec3_mul_s(tangent, _scale);

    _result[3] = 0.0f;
    _result[7] = 0.0f;
    _result[11] = 0.0f;
    _result[12] = _pos.x;
    _result[13] = _pos.y;
    _result[14] = _pos.z;
    _result[15] = 1.0f;
}

static inline void ce_mat4_from_normal_a(float *_result,
                                         ce_vec3_t _normal,
                                         float _scale,
                                         ce_vec3_t _pos,
                                         float _angle) {
    ce_vec3_t tangent;
    ce_vec3_t bitangent;
    ce_vec3_tangent_frame_a(_normal, &tangent, &bitangent, _angle);

    *(ce_vec3_t *) (&_result[0]) = ce_vec3_mul_s(bitangent, _scale);
    *(ce_vec3_t *) (&_result[4]) = ce_vec3_mul_s(_normal, _scale);
    *(ce_vec3_t *) (&_result[8]) = ce_vec3_mul_s(tangent, _scale);

    _result[3] = 0.0f;
    _result[7] = 0.0f;
    _result[11] = 0.0f;
    _result[12] = _pos.x;
    _result[13] = _pos.y;
    _result[14] = _pos.z;
    _result[15] = 1.0f;
}


static inline void ce_mat4_move(float *_result,
                                const float *_a) {
    memcpy(_result, _a, sizeof(float) * 16);
}

static inline void ce_mat4_inverse(float *_result,
                                   const float *_a) {
    float xx = _a[0];
    float xy = _a[1];
    float xz = _a[2];
    float xw = _a[3];
    float yx = _a[4];
    float yy = _a[5];
    float yz = _a[6];
    float yw = _a[7];
    float zx = _a[8];
    float zy = _a[9];
    float zz = _a[10];
    float zw = _a[11];
    float wx = _a[12];
    float wy = _a[13];
    float wz = _a[14];
    float ww = _a[15];

    float det = 0.0f;
    det += xx * (yy * (zz * ww - zw * wz) - yz * (zy * ww - zw * wy) +
                 yw * (zy * wz - zz * wy));
    det -= xy * (yx * (zz * ww - zw * wz) - yz * (zx * ww - zw * wx) +
                 yw * (zx * wz - zz * wx));
    det += xz * (yx * (zy * ww - zw * wy) - yy * (zx * ww - zw * wx) +
                 yw * (zx * wy - zy * wx));
    det -= xw * (yx * (zy * wz - zz * wy) - yy * (zx * wz - zz * wx) +
                 yz * (zx * wy - zy * wx));

    float invDet = 1.0f / det;

    _result[0] = +(yy * (zz * ww - wz * zw) - yz * (zy * ww - wy * zw) +
                   yw * (zy * wz - wy * zz)) * invDet;
    _result[1] = -(xy * (zz * ww - wz * zw) - xz * (zy * ww - wy * zw) +
                   xw * (zy * wz - wy * zz)) * invDet;
    _result[2] = +(xy * (yz * ww - wz * yw) - xz * (yy * ww - wy * yw) +
                   xw * (yy * wz - wy * yz)) * invDet;
    _result[3] = -(xy * (yz * zw - zz * yw) - xz * (yy * zw - zy * yw) +
                   xw * (yy * zz - zy * yz)) * invDet;

    _result[4] = -(yx * (zz * ww - wz * zw) - yz * (zx * ww - wx * zw) +
                   yw * (zx * wz - wx * zz)) * invDet;
    _result[5] = +(xx * (zz * ww - wz * zw) - xz * (zx * ww - wx * zw) +
                   xw * (zx * wz - wx * zz)) * invDet;
    _result[6] = -(xx * (yz * ww - wz * yw) - xz * (yx * ww - wx * yw) +
                   xw * (yx * wz - wx * yz)) * invDet;
    _result[7] = +(xx * (yz * zw - zz * yw) - xz * (yx * zw - zx * yw) +
                   xw * (yx * zz - zx * yz)) * invDet;

    _result[8] = +(yx * (zy * ww - wy * zw) - yy * (zx * ww - wx * zw) +
                   yw * (zx * wy - wx * zy)) * invDet;
    _result[9] = -(xx * (zy * ww - wy * zw) - xy * (zx * ww - wx * zw) +
                   xw * (zx * wy - wx * zy)) * invDet;
    _result[10] = +(xx * (yy * ww - wy * yw) - xy * (yx * ww - wx * yw) +
                    xw * (yx * wy - wx * yy)) * invDet;
    _result[11] = -(xx * (yy * zw - zy * yw) - xy * (yx * zw - zx * yw) +
                    xw * (yx * zy - zx * yy)) * invDet;

    _result[12] = -(yx * (zy * wz - wy * zz) - yy * (zx * wz - wx * zz) +
                    yz * (zx * wy - wx * zy)) * invDet;
    _result[13] = +(xx * (zy * wz - wy * zz) - xy * (zx * wz - wx * zz) +
                    xz * (zx * wy - wx * zy)) * invDet;
    _result[14] = -(xx * (yy * wz - wy * yz) - xy * (yx * wz - wx * yz) +
                    xz * (yx * wy - wx * yy)) * invDet;
    _result[15] = +(xx * (yy * zz - zy * yz) - xy * (yx * zz - zx * yz) +
                    xz * (yx * zy - zx * yy)) * invDet;
}

static inline void ce_mat4_quat(float *_result,
                                ce_vec4_t quat) {
    const float x = quat.x;
    const float y = quat.y;
    const float z = quat.z;
    const float w = quat.w;

    const float x2 = x + x;
    const float y2 = y + y;
    const float z2 = z + z;
    const float x2x = x2 * x;
    const float x2y = x2 * y;
    const float x2z = x2 * z;
    const float x2w = x2 * w;
    const float y2y = y2 * y;
    const float y2z = y2 * z;
    const float y2w = y2 * w;
    const float z2z = z2 * z;
    const float z2w = z2 * w;

    _result[0] = 1.0f - (y2y + z2z);
    _result[1] = x2y - z2w;
    _result[2] = x2z + y2w;
    _result[3] = 0.0f;

    _result[4] = x2y + z2w;
    _result[5] = 1.0f - (x2x + z2z);
    _result[6] = y2z - x2w;
    _result[7] = 0.0f;

    _result[8] = x2z - y2w;
    _result[9] = y2z + x2w;
    _result[10] = 1.0f - (x2x + y2y);
    _result[11] = 0.0f;

    _result[12] = 0.0f;
    _result[13] = 0.0f;
    _result[14] = 0.0f;
    _result[15] = 1.0f;
}

static inline void ce_mat4_quat_translation(float *_result,
                                            ce_vec4_t _quat,
                                            ce_vec3_t _translation) {
    ce_mat4_quat(_result, _quat);
    _result[12] = -(_result[0] * _translation.x +
                    _result[4] * _translation.y +
                    _result[8] * _translation.z);
    _result[13] = -(_result[1] * _translation.x +
                    _result[5] * _translation.y +
                    _result[9] * _translation.z);
    _result[14] = -(_result[2] * _translation.x +
                    _result[6] * _translation.y +
                    _result[10] * _translation.z);
}

static inline void ce_mat4_quat_translation_hmd(float *_result,
                                                ce_vec4_t _quat,
                                                ce_vec3_t _translation) {
    ce_vec4_t quat = {
            .x = -_quat.x,
            .y = -_quat.y,
            .z = _quat.z,
            .w = _quat.w,
    };

    ce_mat4_quat_translation(_result, quat, _translation);
}

static inline void ce_mat4_rotate_x(float *_result,
                                    float _ax) {
    const float sx = ce_fsin(_ax);
    const float cx = ce_fcos(_ax);

    memset(_result, 0, sizeof(float) * 16);
    _result[0] = 1.0f;
    _result[5] = cx;
    _result[6] = -sx;
    _result[9] = sx;
    _result[10] = cx;
    _result[15] = 1.0f;
}

static inline void ce_mat4_rotate_y(float *_result,
                                    float _ay) {
    const float sy = ce_fsin(_ay);
    const float cy = ce_fcos(_ay);

    memset(_result, 0, sizeof(float) * 16);
    _result[0] = cy;
    _result[2] = sy;
    _result[5] = 1.0f;
    _result[8] = -sy;
    _result[10] = cy;
    _result[15] = 1.0f;
}

static inline void ce_mat4_rotate_z(float *_result,
                                    float _az) {
    const float sz = ce_fsin(_az);
    const float cz = ce_fcos(_az);

    memset(_result, 0, sizeof(float) * 16);
    _result[0] = cz;
    _result[1] = -sz;
    _result[4] = sz;
    _result[5] = cz;
    _result[10] = 1.0f;
    _result[15] = 1.0f;
}

static inline void ce_mat4_rotate_xy(float *_result,
                                     float _ax,
                                     float _ay) {
    const float sx = ce_fsin(_ax);
    const float cx = ce_fcos(_ax);
    const float sy = ce_fsin(_ay);
    const float cy = ce_fcos(_ay);

    memset(_result, 0, sizeof(float) * 16);
    _result[0] = cy;
    _result[2] = sy;
    _result[4] = sx * sy;
    _result[5] = cx;
    _result[6] = -sx * cy;
    _result[8] = -cx * sy;
    _result[9] = sx;
    _result[10] = cx * cy;
    _result[15] = 1.0f;
}

static inline void ce_mat4_rotate_xyz(float *_result,
                                      float _ax,
                                      float _ay,
                                      float _az) {
    const float sx = ce_fsin(_ax);
    const float cx = ce_fcos(_ax);
    const float sy = ce_fsin(_ay);
    const float cy = ce_fcos(_ay);
    const float sz = ce_fsin(_az);
    const float cz = ce_fcos(_az);

    memset(_result, 0, sizeof(float) * 16);
    _result[0] = cy * cz;
    _result[1] = -cy * sz;
    _result[2] = sy;
    _result[4] = cz * sx * sy + cx * sz;
    _result[5] = cx * cz - sx * sy * sz;
    _result[6] = -cy * sx;
    _result[8] = -cx * cz * sy + sx * sz;
    _result[9] = cz * sx + cx * sy * sz;
    _result[10] = cx * cy;
    _result[15] = 1.0f;
}

static inline void ce_mat4_rotate_zyx(float *_result,
                                      float _ax,
                                      float _ay,
                                      float _az) {
    const float sx = ce_fsin(_ax);
    const float cx = ce_fcos(_ax);
    const float sy = ce_fsin(_ay);
    const float cy = ce_fcos(_ay);
    const float sz = ce_fsin(_az);
    const float cz = ce_fcos(_az);

    memset(_result, 0, sizeof(float) * 16);
    _result[0] = cy * cz;
    _result[1] = cz * sx * sy - cx * sz;
    _result[2] = cx * cz * sy + sx * sz;
    _result[4] = cy * sz;
    _result[5] = cx * cz + sx * sy * sz;
    _result[6] = -cz * sx + cx * sy * sz;
    _result[8] = -sy;
    _result[9] = cy * sx;
    _result[10] = cx * cy;
    _result[15] = 1.0f;
};

static inline void ce_mat4_srt(float *_result,
                               float _sx,
                               float _sy,
                               float _sz,
                               float _ax,
                               float _ay,
                               float _az,
                               float _tx,
                               float _ty,
                               float _tz) {
    const float sx = ce_fsin(_ax);
    const float cx = ce_fcos(_ax);
    const float sy = ce_fsin(_ay);
    const float cy = ce_fcos(_ay);
    const float sz = ce_fsin(_az);
    const float cz = ce_fcos(_az);

    const float sxsz = sx * sz;
    const float cycz = cy * cz;

    _result[0] = _sx * (cycz - sxsz * sy);
    _result[1] = _sx * -cx * sz;
    _result[2] = _sx * (cz * sy + cy * sxsz);
    _result[3] = 0.0f;

    _result[4] = _sy * (cz * sx * sy + cy * sz);
    _result[5] = _sy * cx * cz;
    _result[6] = _sy * (sy * sz - cycz * sx);
    _result[7] = 0.0f;

    _result[8] = _sz * -cx * sy;
    _result[9] = _sz * sx;
    _result[10] = _sz * cx * cy;
    _result[11] = 0.0f;

    _result[12] = _tx;
    _result[13] = _ty;
    _result[14] = _tz;
    _result[15] = 1.0f;
}

static inline ce_vec3_t ce_vec3_mul_mtx(ce_vec3_t _vec,
                                        const float *_mat) {
    return (ce_vec3_t) {
            .x = _vec.x * _mat[0] + _vec.y * _mat[4] + _vec.z * _mat[8] + _mat[12],
            .y = _vec.x * _mat[1] + _vec.y * _mat[5] + _vec.z * _mat[9] + _mat[13],
            .z = _vec.x * _mat[2] + _vec.y * _mat[6] + _vec.z * _mat[10] + _mat[14],
    };
}

static inline ce_vec3_t ce_vec3_mul_mat4_h(ce_vec3_t _vec, const float *_mat) {
    float xx = _vec.x * _mat[0] + _vec.y * _mat[4] + _vec.z * _mat[8] + _mat[12];
    float yy = _vec.x * _mat[1] + _vec.y * _mat[5] + _vec.z * _mat[9] + _mat[13];
    float zz = _vec.x * _mat[2] + _vec.y * _mat[6] + _vec.z * _mat[10] + _mat[14];
    float ww = _vec.x * _mat[3] + _vec.y * _mat[7] + _vec.z * _mat[11] + _mat[15];

    float invW = ce_fsign(ww) / ww;

    return (ce_vec3_t) {
            .x = xx * invW,
            .y = yy * invW,
            .z = zz * invW,
    };

}

static inline ce_vec4_t ce_vec4_mul_mtx(ce_vec4_t _vec,
                                        const float *_mat) {

    return (ce_vec4_t) {
            .x =_vec.x * _mat[0] + _vec.y * _mat[4] + _vec.z * _mat[8] + _vec.w * _mat[12],
            .y =_vec.x * _mat[1] + _vec.y * _mat[5] + _vec.z * _mat[9] + _vec.w * _mat[13],
            .z =_vec.x * _mat[2] + _vec.y * _mat[6] + _vec.z * _mat[10] + _vec.w * _mat[14],
            .w =_vec.x * _mat[3] + _vec.y * _mat[7] + _vec.z * _mat[11] + _vec.w * _mat[15],
    };

}

static inline void ce_mat4_mul(float *_result,
                               const float *_a,
                               const float *_b) {
    *((ce_vec4_t*)&_result[0]) = ce_vec4_mul_mtx(*(ce_vec4_t *) (&_a[0]), _b);
    *((ce_vec4_t*)&_result[4]) = ce_vec4_mul_mtx(*(ce_vec4_t *) (&_a[4]), _b);
    *((ce_vec4_t*)&_result[8]) = ce_vec4_mul_mtx(*(ce_vec4_t *) (&_a[8]), _b);
    *((ce_vec4_t*)&_result[12]) = ce_vec4_mul_mtx(*(ce_vec4_t *) (&_a[12]), _b);
}

static inline void ce_mat4_transpose(float *_result,
                                     const float *_a) {
    _result[0] = _a[0];
    _result[4] = _a[1];
    _result[8] = _a[2];
    _result[12] = _a[3];
    _result[1] = _a[4];
    _result[5] = _a[5];
    _result[9] = _a[6];
    _result[13] = _a[7];
    _result[2] = _a[8];
    _result[6] = _a[9];
    _result[10] = _a[10];
    _result[14] = _a[11];
    _result[3] = _a[12];
    _result[7] = _a[13];
    _result[11] = _a[14];
    _result[15] = _a[15];
}


// # Calc

static inline ce_vec3_t ce_calc_normal(ce_vec3_t _va,
                                       ce_vec3_t _vb,
                                       ce_vec3_t _vc) {
    ce_vec3_t ba = ce_vec3_sub(_vb, _va);
    ce_vec3_t ca = ce_vec3_sub(_vc, _va);
    return ce_vec3_norm(ce_vec3_cross(ba, ca));
}

static inline ce_vec4_t ce_calc_plane(ce_vec3_t _va,
                                      ce_vec3_t _vb,
                                      ce_vec3_t _vc) {

    ce_vec3_t n = ce_calc_normal(_va, _vb, _vc);

    return (ce_vec4_t) {
            .x = n.x,
            .y = n.y,
            .z = n.z,
            .w = -ce_vec3_dot(n, _va),
    };
}

static inline void ce_calc_linear_fit_2d(float *_result,
                                         const void *_points,
                                         uint32_t _stride,
                                         uint32_t _numPoints) {
    float sumX = 0.0f;
    float sumY = 0.0f;
    float sumXX = 0.0f;
    float sumXY = 0.0f;

    const uint8_t *ptr = (const uint8_t *) _points;
    for (uint32_t ii = 0; ii < _numPoints; ++ii, ptr += _stride) {
        const float *point = (const float *) ptr;
        float xx = point[0];
        float yy = point[1];
        sumX += xx;
        sumY += yy;
        sumXX += xx * xx;
        sumXY += xx * yy;
    }

    // [ sum(x^2) sum(x)    ] [ A ] = [ sum(x*y) ]
    // [ sum(x)   numPoints ] [ B ]   [ sum(y)   ]

    float det = (sumXX * _numPoints - sumX * sumX);
    float invDet = 1.0f / det;

    _result[0] = (-sumX * sumY + _numPoints * sumXY) * invDet;
    _result[1] = (sumXX * sumY - sumX * sumXY) * invDet;
}

static inline void ce_calc_linear_fit_3d(float *_result,
                                         const void *_points,
                                         uint32_t _stride,
                                         uint32_t _numPoints) {
    float sumX = 0.0f;
    float sumY = 0.0f;
    float sumZ = 0.0f;
    float sumXX = 0.0f;
    float sumXY = 0.0f;
    float sumXZ = 0.0f;
    float sumYY = 0.0f;
    float sumYZ = 0.0f;

    const uint8_t *ptr = (const uint8_t *) _points;
    for (uint32_t ii = 0; ii < _numPoints; ++ii, ptr += _stride) {
        const float *point = (const float *) ptr;
        float xx = point[0];
        float yy = point[1];
        float zz = point[2];

        sumX += xx;
        sumY += yy;
        sumZ += zz;
        sumXX += xx * xx;
        sumXY += xx * yy;
        sumXZ += xx * zz;
        sumYY += yy * yy;
        sumYZ += yy * zz;
    }

    // [ sum(x^2) sum(x*y) sum(x)    ] [ A ]   [ sum(x*z) ]
    // [ sum(x*y) sum(y^2) sum(y)    ] [ B ] = [ sum(y*z) ]
    // [ sum(x)   sum(y)   numPoints ] [ C ]   [ sum(z)   ]

    float mtx[9] =
            {
                    sumXX, sumXY, sumX,
                    sumXY, sumYY, sumY,
                    sumX, sumY, (float) _numPoints,
            };
    float invMtx[9];
    ce_mat3_inverse(invMtx, mtx);

    _result[0] = invMtx[0] * sumXZ + invMtx[1] * sumYZ + invMtx[2] * sumZ;
    _result[1] = invMtx[3] * sumXZ + invMtx[4] * sumYZ + invMtx[5] * sumZ;
    _result[2] = invMtx[6] * sumXZ + invMtx[7] * sumYZ + invMtx[8] * sumZ;
}

// # Color

static inline void ce_rgb_to_hsv(float *_hsv,
                                 const float *_rgb) {
    const float rr = _rgb[0];
    const float gg = _rgb[1];
    const float bb = _rgb[2];

    const float s0 = ce_fstep(bb, gg);

    const float px = ce_flerp(bb, gg, s0);
    const float py = ce_flerp(gg, bb, s0);
    const float pz = ce_flerp(-1.0f, 0.0f, s0);
    const float pw = ce_flerp(2.0f / 3.0f, -1.0f / 3.0f, s0);

    const float s1 = ce_fstep(px, rr);

    const float qx = ce_flerp(px, rr, s1);
    const float qy = py;
    const float qz = ce_flerp(pw, pz, s1);
    const float qw = ce_flerp(rr, px, s1);

    const float dd = qx - fmin(qw, qy);
    const float ee = 1.0e-10f;

    _hsv[0] = ce_fabsolute(qz + (qw - qy) / (6.0f * dd + ee));
    _hsv[1] = dd / (qx + ee);
    _hsv[2] = qx;
}

static inline void ce_hsv_to_rgb(float *_rgb,
                                 const float *_hsv) {
    const float hh = _hsv[0];
    const float ss = _hsv[1];
    const float vv = _hsv[2];

    const float px = ce_fabsolute(ce_ffract(hh + 1.0f) * 6.0f - 3.0f);
    const float py = ce_fabsolute(
            ce_ffract(hh + 2.0f / 3.0f) * 6.0f - 3.0f);
    const float pz = ce_fabsolute(
            ce_ffract(hh + 1.0f / 3.0f) * 6.0f - 3.0f);

    _rgb[0] = vv * ce_flerp(1.0f, ce_fsaturate(px - 1.0f), ss);
    _rgb[1] = vv * ce_flerp(1.0f, ce_fsaturate(py - 1.0f), ss);
    _rgb[2] = vv * ce_flerp(1.0f, ce_fsaturate(pz - 1.0f), ss);
}

// # Projection

// Convert LH to RH projection matrix and vice versa.
static inline void ce_mat4_proj_flip_handedness(float *_dst,
                                                const float *_src) {
    _dst[0] = -_src[0];
    _dst[1] = -_src[1];
    _dst[2] = -_src[2];
    _dst[3] = -_src[3];
    _dst[4] = _src[4];
    _dst[5] = _src[5];
    _dst[6] = _src[6];
    _dst[7] = _src[7];
    _dst[8] = -_src[8];
    _dst[9] = -_src[9];
    _dst[10] = -_src[10];
    _dst[11] = -_src[11];
    _dst[12] = _src[12];
    _dst[13] = _src[13];
    _dst[14] = _src[14];
    _dst[15] = _src[15];
}

//Convert LH to RH view matrix and vice versa.
static inline void ce_mat4_view_flip_handedness(float *_dst,
                                                const float *_src) {
    _dst[0] = -_src[0];
    _dst[1] = _src[1];
    _dst[2] = -_src[2];
    _dst[3] = _src[3];
    _dst[4] = -_src[4];
    _dst[5] = _src[5];
    _dst[6] = -_src[6];
    _dst[7] = _src[7];
    _dst[8] = -_src[8];
    _dst[9] = _src[9];
    _dst[10] = -_src[10];
    _dst[11] = _src[11];
    _dst[12] = -_src[12];
    _dst[13] = _src[13];
    _dst[14] = -_src[14];
    _dst[15] = _src[15];
}

static inline void ce_mat4_look_at_impl(float *_result,
                                        ce_vec3_t _eye,
                                        ce_vec3_t _view,
                                        ce_vec3_t _up) {
    ce_vec3_t right = ce_vec3_norm(ce_vec3_cross(_up, _view));
    ce_vec3_t up = ce_vec3_cross(_view, right);

    memset(_result, 0, sizeof(float) * 16);

    _result[0] = right.x;
    _result[1] = up.x;
    _result[2] = _view.x;

    _result[4] = right.y;
    _result[5] = up.y;
    _result[6] = _view.y;

    _result[8] = right.z;
    _result[9] = up.z;
    _result[10] = _view.z;

    _result[12] = -ce_vec3_dot(right, _eye);
    _result[13] = -ce_vec3_dot(up, _eye);
    _result[14] = -ce_vec3_dot(_view, _eye);
    _result[15] = 1.0f;
}

static inline void ce_mat4_look_at_lh(float *_result,
                                      ce_vec3_t _eye,
                                      ce_vec3_t _at,
                                      ce_vec3_t _up) {
    ce_vec3_t dir = ce_vec3_sub(_at, _eye);
    struct ce_vec3_t view = ce_vec3_norm(dir);

    ce_mat4_look_at_impl(_result, _eye, view, _up);
}

static inline void ce_mat4_look_at_rh(float *_result,
                                      ce_vec3_t _eye,
                                      ce_vec3_t _at,
                                      ce_vec3_t _up) {
    ce_vec3_t dir = ce_vec3_sub(_eye, _at);
    struct ce_vec3_t view = ce_vec3_norm(dir);
    ce_mat4_look_at_impl(_result, _eye, view, _up);
}

static inline void ce_mat4_look_at(float *_result,
                                   ce_vec3_t _eye,
                                   ce_vec3_t _at,
                                   ce_vec3_t _up) {
    ce_mat4_look_at_lh(_result, _eye, _at, _up);
}

static inline void ce_mat4_ortho_lh(float *_result,
                                    float _left,
                                    float _right,
                                    float _bottom,
                                    float _top,
                                    float _near,
                                    float _far,
                                    float _offset,
                                    bool _oglNdc) {
    const float aa = 2.0f / (_right - _left);
    const float bb = 2.0f / (_top - _bottom);
    const float cc = (_oglNdc ? 2.0f : 1.0f) / (_far - _near);
    const float dd = (_left + _right) / (_left - _right);
    const float ee = (_top + _bottom) / (_bottom - _top);
    const float ff = _oglNdc
                     ? (_near + _far) / (_near - _far)
                     : _near / (_near - _far);

    memset(_result, 0, sizeof(float) * 16);
    _result[0] = aa;
    _result[5] = bb;
    _result[10] = cc;
    _result[12] = dd + _offset;
    _result[13] = ee;
    _result[14] = ff;
    _result[15] = 1.0f;

}

static inline void ce_mat4_ortho(float *_result,
                                 float _left,
                                 float _right,
                                 float _bottom,
                                 float _top,
                                 float _near,
                                 float _far,
                                 float _offset,
                                 bool _oglNdc) {
    ce_mat4_ortho_lh(_result, _left, _right, _bottom, _top,
                     _near, _far, _offset, _oglNdc);
}

static inline void ce_mat4_ortho_rh(float *_result,
                                    float _left,
                                    float _right,
                                    float _bottom,
                                    float _top,
                                    float _near,
                                    float _far,
                                    float _offset,
                                    bool _oglNdc) {
    const float aa = 2.0f / (_right - _left);
    const float bb = 2.0f / (_top - _bottom);
    const float cc = (_oglNdc ? 2.0f : 1.0f) / (_far - _near);
    const float dd = (_left + _right) / (_left - _right);
    const float ee = (_top + _bottom) / (_bottom - _top);
    const float ff = _oglNdc
                     ? (_near + _far) / (_near - _far)
                     : _near / (_near - _far);

    memset(_result, 0, sizeof(float) * 16);
    _result[0] = aa;
    _result[5] = bb;
    _result[10] = -cc;
    _result[12] = dd + _offset;
    _result[13] = ee;
    _result[14] = ff;
    _result[15] = 1.0f;
}

enum Handness {
    Left = 0,
    Right,
};

enum NearFar {
    Default = 0,
    Reverse,
};

static inline void ce_mat4_proj_xywh(float *_result,
                                     float _x,
                                     float _y,
                                     float _width,
                                     float _height,
                                     float _near,
                                     float _far,
                                     bool _oglNdc,
                                     enum Handness handness) {

    const float diff = _far - _near;
    const float aa = _oglNdc ? (_far + _near) / diff : _far / diff;
    const float bb = _oglNdc ? (2.0f * _far * _near) / diff : _near * aa;

    memset(_result, 0, sizeof(float) * 16);
    _result[0] = _width;
    _result[5] = _height;
    _result[8] = (Right == handness) ? _x : -_x;
    _result[9] = (Right == handness) ? _y : -_y;
    _result[10] = (Right == handness) ? -aa : aa;
    _result[11] = (Right == handness) ? -1.0f : 1.0f;
    _result[14] = -bb;
}

static inline void ce_mat4_proj_impl(float *_result,
                                     float _ut,
                                     float _dt,
                                     float _lt,
                                     float _rt,
                                     float _near,
                                     float _far,
                                     bool _oglNdc,
                                     enum Handness handness) {
    const float invDiffRl = 1.0f / (_rt - _lt);
    const float invDiffUd = 1.0f / (_ut - _dt);
    const float width = 2.0f * _near * invDiffRl;
    const float height = 2.0f * _near * invDiffUd;
    const float xx = (_rt + _lt) * invDiffRl;
    const float yy = (_ut + _dt) * invDiffUd;
    ce_mat4_proj_xywh(_result, xx, yy, width, height, _near, _far,
                      _oglNdc, handness);
}

static inline void mat4_proj_impl_fov(float *_result,
                                      const float *_fov,
                                      float _near,
                                      float _far,
                                      bool _oglNdc,
                                      enum Handness handness) {
    ce_mat4_proj_impl(_result, _fov[0], _fov[1], _fov[2], _fov[3],
                      _near, _far, _oglNdc, handness);
}

static inline void ce_mat4_proj_impl_fovy(float *_result,
                                          float _fovy,
                                          float _aspect,
                                          float _near,
                                          float _far,
                                          bool _oglNdc,
                                          enum Handness handness) {
    const float height = 1.0f / ce_ftan(ce_to_rad(_fovy) * 0.5f);
    const float width = height * 1.0f / _aspect;
    ce_mat4_proj_xywh(_result, 0.0f, 0.0f, width, height, _near,
                      _far,
                      _oglNdc, handness);
}

static inline void ce_mat4_proj(float *_result,
                                float _ut,
                                float _dt,
                                float _lt,
                                float _rt,
                                float _near,
                                float _far,
                                bool _oglNdc) {
    ce_mat4_proj_impl(_result, _ut, _dt, _lt, _rt, _near, _far,
                      _oglNdc, Left);
}

static inline void ce_mat4_proj_fov(float *_result,
                                    const float *_fov,
                                    float _near,
                                    float _far,
                                    bool _oglNdc) {
    mat4_proj_impl_fov(_result, _fov, _near, _far, _oglNdc, Left);
}

static inline void ce_mat4_proj_fovy(float *_result,
                                     float _fovy,
                                     float _aspect,
                                     float _near,
                                     float _far,
                                     bool _oglNdc) {
    ce_mat4_proj_impl_fovy(_result, _fovy, _aspect, _near, _far,
                           _oglNdc, Left);
}

static inline void ce_mat4_proj_lh(float *_result,
                                   float _ut,
                                   float _dt,
                                   float _lt,
                                   float _rt,
                                   float _near,
                                   float _far,
                                   bool _oglNdc) {
    ce_mat4_proj_impl(_result, _ut, _dt, _lt, _rt, _near, _far,
                      _oglNdc, Left);
}

static inline void ce_mat4_proj_lh_fov(float *_result,
                                       const float *_fov,
                                       float _near,
                                       float _far,
                                       bool _oglNdc) {
    mat4_proj_impl_fov(_result, _fov, _near, _far, _oglNdc, Left);
}

static inline void ce_mat4_proj_lh_fovy(float *_result,
                                        float _fovy,
                                        float _aspect,
                                        float _near,
                                        float _far,
                                        bool _oglNdc) {
    ce_mat4_proj_impl_fovy(_result, _fovy, _aspect, _near, _far,
                           _oglNdc, Left);
}

static inline void ce_mat4_proj_rh(float *_result,
                                   float _ut,
                                   float _dt,
                                   float _lt,
                                   float _rt,
                                   float _near,
                                   float _far,
                                   bool _oglNdc) {
    ce_mat4_proj_impl(_result, _ut, _dt, _lt, _rt, _near, _far,
                      _oglNdc, Right);
}

static inline void ce_mat4_proj_rh_fov(float *_result,
                                       const float *_fov,
                                       float _near,
                                       float _far,
                                       bool _oglNdc) {
    mat4_proj_impl_fov(_result, _fov, _near, _far, _oglNdc, Right);
}

static inline void ce_mat4_proj_rh_fovy(float *_result,
                                        float _fovy,
                                        float _aspect,
                                        float _near,
                                        float _far,
                                        bool _oglNdc) {
    ce_mat4_proj_impl_fovy(_result, _fovy, _aspect, _near, _far,
                           _oglNdc, Right);
}

static inline void ce_mat4_proj_inf_xywh(float *_result,
                                         float _x,
                                         float _y,
                                         float _width,
                                         float _height,
                                         float _near,
                                         bool _oglNdc,
                                         enum NearFar nearfar,
                                         enum Handness handness) {
    float aa;
    float bb;
    if (Reverse == nearfar) {
        aa = _oglNdc ? -1.0f : 0.0f;
        bb = _oglNdc ? -2.0f * _near : -_near;
    } else {
        aa = 1.0f;
        bb = _oglNdc ? 2.0f * _near : _near;
    }

    memset(_result, 0, sizeof(float) * 16);
    _result[0] = _width;
    _result[5] = _height;
    _result[8] = (Right == handness) ? _x : -_x;
    _result[9] = (Right == handness) ? _y : -_y;
    _result[10] = (Right == handness) ? -aa : aa;
    _result[11] = (Right == handness) ? -1.0f : 1.0f;
    _result[14] = -bb;
}

static inline void mat4_proj_inf_impl(float *_result,
                                      float _ut,
                                      float _dt,
                                      float _lt,
                                      float _rt,
                                      float _near,
                                      bool _oglNdc,
                                      enum NearFar nearfar,
                                      enum Handness handness) {
    const float invDiffRl = 1.0f / (_rt - _lt);
    const float invDiffUd = 1.0f / (_ut - _dt);
    const float width = 2.0f * _near * invDiffRl;
    const float height = 2.0f * _near * invDiffUd;
    const float xx = (_rt + _lt) * invDiffRl;
    const float yy = (_ut + _dt) * invDiffUd;
    ce_mat4_proj_inf_xywh(_result, xx, yy, width, height,
                          _near, _oglNdc, nearfar, handness);
}

static inline void ce_mat4_proj_inf_iml_fov(float *_result,
                                            const float *_fov,
                                            float _near,
                                            bool _oglNdc,
                                            enum NearFar nearfar,
                                            enum Handness handness) {
    mat4_proj_inf_impl(_result, _fov[0], _fov[1], _fov[2],
                       _fov[3], _near, _oglNdc, nearfar, handness);
}

static inline void ce_mat4_proj_inf_impl_fovy(float *_result,
                                              float _fovy,
                                              float _aspect,
                                              float _near,
                                              bool _oglNdc,
                                              enum NearFar nearfar,
                                              enum Handness handness) {
    const float height = 1.0f / ce_ftan(ce_to_rad(_fovy) * 0.5f);
    const float width = height * 1.0f / _aspect;
    ce_mat4_proj_inf_xywh(_result, 0.0f, 0.0f, width,
                          height,
                          _near, _oglNdc, nearfar, handness);
}

static inline void ce_mat4_proj_inf_fov(float *_result,
                                        const float *_fov,
                                        float _near,
                                        bool _oglNdc) {
    ce_mat4_proj_inf_iml_fov(_result, _fov, _near,
                             _oglNdc, Default, Left);
}

static inline void ce_mat4_proj_inf(float *_result,
                                    float _ut,
                                    float _dt,
                                    float _lt,
                                    float _rt,
                                    float _near,
                                    bool _oglNdc) {
    mat4_proj_inf_impl(_result, _ut, _dt, _lt,
                       _rt, _near, _oglNdc, Default, Left);
}

static inline void ce_mat4_proj_inf_fovy(float *_result,
                                         float _fovy,
                                         float _aspect,
                                         float _near,
                                         bool _oglNdc) {
    ce_mat4_proj_inf_impl_fovy(_result, _fovy,
                               _aspect, _near,
                               _oglNdc, Default, Left);
}

static inline void ce_mat4_proj_inf_lh(float *_result,
                                       float _ut,
                                       float _dt,
                                       float _lt,
                                       float _rt,
                                       float _near,
                                       bool _oglNdc) {
    mat4_proj_inf_impl(_result, _ut, _dt, _lt,
                       _rt, _near, _oglNdc, Default, Left);
}

static inline void ce_mat4_proj_inf_lh_fov(float *_result,
                                           const float *_fov,
                                           float _near,
                                           bool _oglNdc) {
    ce_mat4_proj_inf_iml_fov(_result, _fov, _near,
                             _oglNdc, Default, Left);
}

static inline void ce_mat4_proj_inf_lh_fovy(float *_result,
                                            float _fovy,
                                            float _aspect,
                                            float _near,
                                            bool _oglNdc) {
    ce_mat4_proj_inf_impl_fovy(_result, _fovy,
                               _aspect, _near,
                               _oglNdc, Default, Left);
}

static inline void ce_mat4_proj_inf_rh(float *_result,
                                       float _ut,
                                       float _dt,
                                       float _lt,
                                       float _rt,
                                       float _near,
                                       bool _oglNdc) {
    mat4_proj_inf_impl(_result, _ut, _dt,
                       _lt, _rt, _near,
                       _oglNdc, Default, Right);
}

static inline void ce_mat4_proj_inf_rh_fov(float *_result,
                                           const float *_fov,
                                           float _near,
                                           bool _oglNdc) {
    ce_mat4_proj_inf_iml_fov(_result, _fov, _near,
                             _oglNdc, Default, Right);
}

static inline void ce_mat4_proj_inf_rh_fovy(float *_result,
                                            float _fovy,
                                            float _aspect,
                                            float _near,
                                            bool _oglNdc) {
    ce_mat4_proj_inf_impl_fovy(_result, _fovy,
                               _aspect, _near,
                               _oglNdc, Default, Right);
}

static inline void ce_mat4_proj_rev_inf_lh(float *_result,
                                           float _ut,
                                           float _dt,
                                           float _lt,
                                           float _rt,
                                           float _near,
                                           bool _oglNdc) {
    mat4_proj_inf_impl(_result, _ut, _dt, _lt,
                       _rt, _near, _oglNdc, Reverse, Left);
}

static inline void ce_mat4_proj_rev_inf_lh_fov(float *_result,
                                               const float *_fov,
                                               float _near,
                                               bool _oglNdc) {
    ce_mat4_proj_inf_iml_fov(_result, _fov, _near,
                             _oglNdc, Reverse, Left);
}

static inline void ce_mat4_proj_rev_inf_lh_fovy(float *_result,
                                                float _fovy,
                                                float _aspect,
                                                float _near,
                                                bool _oglNdc) {
    ce_mat4_proj_inf_impl_fovy(_result, _fovy,
                               _aspect, _near,
                               _oglNdc, Reverse, Left);
}

static inline void ce_mat4_proj_rev_inf_rh(float *_result,
                                           float _ut,
                                           float _dt,
                                           float _lt,
                                           float _rt,
                                           float _near,
                                           bool _oglNdc) {
    mat4_proj_inf_impl(_result, _ut, _dt,
                       _lt, _rt, _near,
                       _oglNdc, Reverse, Right);
}

static inline void ce_mat4_proj_rev_inf_rh_fov(float *_result,
                                               const float *_fov,
                                               float _near,
                                               bool _oglNdc) {
    ce_mat4_proj_inf_iml_fov(_result, _fov, _near,
                             _oglNdc, Reverse, Right);
}

static inline void ce_mat4_proj_rev_inf_rh_fovy(float *_result,
                                                float _fovy,
                                                float _aspect,
                                                float _near,
                                                bool _oglNdc) {
    ce_mat4_proj_inf_impl_fovy(_result, _fovy,
                               _aspect, _near,
                               _oglNdc, Reverse, Right);
}

#ifdef __cplusplus
};
#endif

#endif //CE_FMATH_H
