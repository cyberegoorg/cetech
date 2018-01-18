/* Bassed on bx math lib.
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "../fmath.h"

#include <math.h>
#include <string.h>

uint32_t cel_float_to_bits(float _a) {
    union {
        float f;
        uint32_t ui;
    } u = {_a};
    return u.ui;
}

float cel_bits_to_float(uint32_t _a) {
    union {
        uint32_t ui;
        float f;
    } u = {_a};
    return u.f;
}

uint64_t cel_double_to_bits(double _a) {
    union {
        double f;
        uint64_t ui;
    } u = {_a};
    return u.ui;
}

double cel_bits_to_double(uint64_t _a) {
    union {
        uint64_t ui;
        double f;
    } u = {_a};
    return u.f;
}


float cel_fabsolute(float _a) {
    return fabsf(_a);
}

float cel_fsin(float _a) {
    return sinf(_a);
}

float cel_fasin(float _a) {
    return asinf(_a);
}

float cel_fcos(float _a) {
    return cosf(_a);
}

float cel_ftan(float _a) {
    return tanf(_a);
}

float cel_facos(float _a) {
    return acosf(_a);
}

float cel_fatan2(float _y,
                 float _x) {
    return atan2f(_y, _x);
}

float cel_fpow(float _a,
               float _b) {
    return powf(_a, _b);
}

float cel_flog(float _a) {
    return logf(_a);
}

float cel_fsqrt(float _a) {
    return sqrtf(_a);
}

float cel_ffloor(float _f) {
    return floorf(_f);
}

float cel_fceil(float _f) {
    return ceilf(_f);
}

float cel_fmod(float _a,
               float _b) {
    return fmodf(_a, _b);
}

bool cel_is_nan(float _f) {
    const uint32_t tmp = cel_float_to_bits(_f) & INT32_MAX;
    return tmp > UINT32_C(0x7f800000);
}

bool cel_is_nand(double _f) {
    const uint64_t tmp = cel_double_to_bits(_f) & INT64_MAX;
    return tmp > UINT64_C(0x7ff0000000000000);
}

bool cel_is_finite(float _f) {
    const uint32_t tmp = cel_float_to_bits(_f) & INT32_MAX;
    return tmp < UINT32_C(0x7f800000);
}

bool cel_is_finited(double _f) {
    const uint64_t tmp = cel_double_to_bits(_f) & INT64_MAX;
    return tmp < UINT64_C(0x7ff0000000000000);
}

bool cel_is_infinite(float _f) {
    const uint32_t tmp = cel_float_to_bits(_f) & INT32_MAX;
    return tmp == UINT32_C(0x7f800000);
}

bool cel_is_infinited(double _f) {
    const uint64_t tmp = cel_double_to_bits(_f) & INT64_MAX;
    return tmp == UINT64_C(0x7ff0000000000000);
}

float cel_fround(float _f) {
    return cel_ffloor(_f + 0.5f);
}

float cel_fmin(float _a,
               float _b) {
    return _a < _b ? _a : _b;
}

float cel_fmax(float _a,
               float _b) {
    return _a > _b ? _a : _b;
}

float cel_fmin3(float _a,
                float _b,
                float _c) {
    return cel_fmin(_a, cel_fmin(_b, _c));
}

float cel_fmax3(float _a,
                float _b,
                float _c) {
    return cel_fmax(_a, cel_fmax(_b, _c));
}

float cel_fclamp(float _a,
                 float _min,
                 float _max) {
    return cel_fmin(cel_fmax(_a, _min), _max);
}

float cel_fsaturate(float _a) {
    return cel_fclamp(_a, 0.0f, 1.0f);
}

float cel_flerp(float _a,
                float _b,
                float _t) {
    return _a + (_b - _a) * _t;
}

float cel_fsign(float _a) {
    return _a < 0.0f ? -1.0f : 1.0f;
}

float cel_fsq(float _a) {
    return _a * _a;
}

float cel_fexp2(float _a) {
    return cel_fpow(2.0f, _a);
}

float cel_flog2(float _a) {
    return cel_flog(_a) * CEL_INV_LOG_NAT_2;
}

float cel_frsqrt(float _a) {
    return 1.0f / cel_fsqrt(_a);
}

float cel_ffract(float _a) {
    return _a - cel_ffloor(_a);
}

bool cel_fequal(float _a,
                float _b,
                float _epsilon) {
    // http://realtimecollisiondetection.net/blog/?p=89
    const float lhs = cel_fabsolute(_a - _b);
    const float rhs =
            _epsilon * cel_fmax3(1.0f, cel_fabsolute(_a), cel_fabsolute(_b));
    return lhs <= rhs;
}

bool cel_fequal_n(const float *_a,
                  const float *_b,
                  uint32_t _num,
                  float _epsilon) {
    bool equal = cel_fequal(_a[0], _b[0], _epsilon);
    for (uint32_t ii = 1; equal && ii < _num; ++ii) {
        equal = cel_fequal(_a[ii], _b[ii], _epsilon);
    }
    return equal;
}

float cel_fwrap(float _a,
                float _wrap) {
    const float mod = cel_fmod(_a, _wrap);
    const float result = mod < 0.0f ? _wrap + mod : mod;
    return result;
}

float cel_fstep(float _edge,
                float _a) {
    return _a < _edge ? 0.0f : 1.0f;
}

float cel_fpulse(float _a,
                 float _start,
                 float _end) {
    return cel_fstep(_a, _start) - cel_fstep(_a, _end);
}

float cel_fsmoothstep(float _a) {
    return cel_fsq(_a) * (3.0f - 2.0f * _a);
}

float cel_fbias(float _time,
                float _bias) {
    return _time / (((1.0f / _bias - 2.0f) * (1.0f - _time)) + 1.0f);
}

float cel_fgain(float _time,
                float _gain) {
    if (_time < 0.5f) {
        return cel_fbias(_time * 2.0f, _gain) * 0.5f;
    }

    return cel_fbias(_time * 2.0f - 1.0f, 1.0f - _gain) * 0.5f + 0.5f;
}


float cel_to_rad(float _deg) {
    return _deg * CEL_DEG_TO_RAD;
}

float cel_to_deg(float _rad) {
    return _rad * CEL_RAD_TO_DEG;
}


float cel_angle_diff(float _a,
                     float _b) {
    const float dist = cel_fwrap(_b - _a, CEL_PI * 2.0f);
    return cel_fwrap(dist * 2.0f, CEL_PI * 2.0f) - dist;
}

float cel_angle_lerp(float _a,
                     float _b,
                     float _t) {
    return _a + cel_angle_diff(_a, _b) * _t;
}


// Vec

void cel_vec3_move(float *_result,
                   const float *_a) {
    _result[0] = _a[0];
    _result[1] = _a[1];
    _result[2] = _a[2];
}

void cel_vec3_abs(float *_result,
                  const float *_a) {
    _result[0] = cel_fabsolute(_a[0]);
    _result[1] = cel_fabsolute(_a[1]);
    _result[2] = cel_fabsolute(_a[2]);
}

void cel_vec3_neg(float *_result,
                  const float *_a) {
    _result[0] = -_a[0];
    _result[1] = -_a[1];
    _result[2] = -_a[2];
}

void cel_vec3_add(float *_result,
                  const float *_a,
                  const float *_b) {
    _result[0] = _a[0] + _b[0];
    _result[1] = _a[1] + _b[1];
    _result[2] = _a[2] + _b[2];
}

void cel_vec3_add_s(float *_result,
                    const float *_a,
                    float _b) {
    _result[0] = _a[0] + _b;
    _result[1] = _a[1] + _b;
    _result[2] = _a[2] + _b;
}

void cel_vec3_sub(float *_result,
                  const float *_a,
                  const float *_b) {
    _result[0] = _a[0] - _b[0];
    _result[1] = _a[1] - _b[1];
    _result[2] = _a[2] - _b[2];
}

void cel_vec3_sub_s(float *_result,
                    const float *_a,
                    float _b) {
    _result[0] = _a[0] - _b;
    _result[1] = _a[1] - _b;
    _result[2] = _a[2] - _b;
}

void cel_vec3_mul(float *_result,
                  const float *_a,
                  const float *_b) {
    _result[0] = _a[0] * _b[0];
    _result[1] = _a[1] * _b[1];
    _result[2] = _a[2] * _b[2];
}

void cel_vec3_mul_s(float *_result,
                    const float *_a,
                    float _b) {
    _result[0] = _a[0] * _b;
    _result[1] = _a[1] * _b;
    _result[2] = _a[2] * _b;
}

float cel_vec3_dot(const float *_a,
                   const float *_b) {
    return _a[0] * _b[0] + _a[1] * _b[1] + _a[2] * _b[2];
}

void cel_vec3_cross(float *_result,
                    const float *_a,
                    const float *_b) {
    _result[0] = _a[1] * _b[2] - _a[2] * _b[1];
    _result[1] = _a[2] * _b[0] - _a[0] * _b[2];
    _result[2] = _a[0] * _b[1] - _a[1] * _b[0];
}

float cel_vec3_length(const float *_a) {
    return cel_fsqrt(cel_vec3_dot(_a, _a));
}

void cel_vec3_lerp_s(float *_result,
                     const float *_a,
                     const float *_b,
                     float _t) {
    _result[0] = cel_flerp(_a[0], _b[0], _t);
    _result[1] = cel_flerp(_a[1], _b[1], _t);
    _result[2] = cel_flerp(_a[2], _b[2], _t);
}

void cel_vec3_lerp(float *_result,
                   const float *_a,
                   const float *_b,
                   const float *_c) {
    _result[0] = cel_flerp(_a[0], _b[0], _c[0]);
    _result[1] = cel_flerp(_a[1], _b[1], _c[1]);
    _result[2] = cel_flerp(_a[2], _b[2], _c[2]);
}

float cel_vec3_norm(float *_result,
                    const float *_a) {
    const float len = cel_vec3_length(_a);
    const float invLen = 1.0f / len;
    _result[0] = _a[0] * invLen;
    _result[1] = _a[1] * invLen;
    _result[2] = _a[2] * invLen;
    return len;
}

void cel_vec3_min(float *_result,
                  const float *_a,
                  const float *_b) {
    _result[0] = fmin(_a[0], _b[0]);
    _result[1] = fmin(_a[1], _b[1]);
    _result[2] = fmin(_a[2], _b[2]);
}

void cel_vec3_max(float *_result,
                  const float *_a,
                  const float *_b) {
    _result[0] = fmax(_a[0], _b[0]);
    _result[1] = fmax(_a[1], _b[1]);
    _result[2] = fmax(_a[2], _b[2]);
}

void cel_vec3_rcp(float *_result,
                  const float *_a) {
    _result[0] = 1.0f / _a[0];
    _result[1] = 1.0f / _a[1];
    _result[2] = 1.0f / _a[2];
}

void cel_vec3_tangent_frame(const float *_n,
                            float *_t,
                            float *_b) {
    const float nx = _n[0];
    const float ny = _n[1];
    const float nz = _n[2];

    if (cel_fabsolute(nx) > cel_fabsolute(nz)) {
        float invLen = 1.0f / cel_fsqrt(nx * nx + nz * nz);
        _t[0] = -nz * invLen;
        _t[1] = 0.0f;
        _t[2] = nx * invLen;
    } else {
        float invLen = 1.0f / cel_fsqrt(ny * ny + nz * nz);
        _t[0] = 0.0f;
        _t[1] = nz * invLen;
        _t[2] = -ny * invLen;
    }

    cel_vec3_cross(_b, _n, _t);
}

void cel_vec3_tangent_frame_a(const float *_n,
                              float *_t,
                              float *_b,
                              float _angle) {
    cel_vec3_tangent_frame(_n, _t, _b);

    const float sa = cel_fsin(_angle);
    const float ca = cel_fcos(_angle);

    _t[0] = -sa * _b[0] + ca * _t[0];
    _t[1] = -sa * _b[1] + ca * _t[1];
    _t[2] = -sa * _b[2] + ca * _t[2];

    cel_vec3_cross(_b, _n, _t);
}

void cel_vec3_from_lat_long(float *_vec,
                            float _u,
                            float _v) {
    const float phi = _u * CEL_PI2;
    const float theta = _v * CEL_PI;

    const float st = cel_fsin(theta);
    const float sp = cel_fsin(phi);
    const float ct = cel_fcos(theta);
    const float cp = cel_fcos(phi);

    _vec[0] = -st * sp;
    _vec[1] = ct;
    _vec[2] = -st * cp;
}

void cel_vec3_to_lat_long(float *_u,
                          float *_v,
                          const float *_vec) {
    const float phi = cel_fatan2(_vec[0], _vec[2]);
    const float theta = cel_facos(_vec[1]);

    *_u = (CEL_PI + phi) * CEL_INV_PI * 0.5f;
    *_v = theta * CEL_INV_PI;
}

void cel_vec4_move(float *_result,
                   const float *_a) {
    _result[0] = _a[0];
    _result[1] = _a[1];
    _result[2] = _a[2];
    _result[3] = _a[3];
}

void cel_quat_identity(float *_result) {
    _result[0] = 0.0f;
    _result[1] = 0.0f;
    _result[2] = 0.0f;
    _result[3] = 1.0f;
}

void cel_quat_move(float *_result,
                   const float *_a) {
    _result[0] = _a[0];
    _result[1] = _a[1];
    _result[2] = _a[2];
    _result[3] = _a[3];
}

void cel_quat_mul_xyz(float *_result,
                      const float *_qa,
                      const float *_qb) {
    const float ax = _qa[0];
    const float ay = _qa[1];
    const float az = _qa[2];
    const float aw = _qa[3];

    const float bx = _qb[0];
    const float by = _qb[1];
    const float bz = _qb[2];
    const float bw = _qb[3];

    _result[0] = aw * bx + ax * bw + ay * bz - az * by;
    _result[1] = aw * by - ax * bz + ay * bw + az * bx;
    _result[2] = aw * bz + ax * by - ay * bx + az * bw;
}

void cel_quat_mul(float *_result,
                  const float *_qa,
                  const float *_qb) {
    const float ax = _qa[0];
    const float ay = _qa[1];
    const float az = _qa[2];
    const float aw = _qa[3];

    const float bx = _qb[0];
    const float by = _qb[1];
    const float bz = _qb[2];
    const float bw = _qb[3];

    _result[0] = aw * bx + ax * bw + ay * bz - az * by;
    _result[1] = aw * by - ax * bz + ay * bw + az * bx;
    _result[2] = aw * bz + ax * by - ay * bx + az * bw;
    _result[3] = aw * bw - ax * bx - ay * by - az * bz;
}

void cel_quat_invert(float *_result,
                     const float *_quat) {
    _result[0] = -_quat[0];
    _result[1] = -_quat[1];
    _result[2] = -_quat[2];
    _result[3] = _quat[3];
}

float cel_quat_dot(const float *_a,
                   const float *_b) {
    return _a[0] * _b[0]
           + _a[1] * _b[1]
           + _a[2] * _b[2]
           + _a[3] * _b[3];
}

void cel_quat_norm(float *_result,
                   const float *_quat) {
    const float norm = cel_quat_dot(_quat, _quat);
    if (0.0f < norm) {
        const float invNorm = 1.0f / cel_fsqrt(norm);
        _result[0] = _quat[0] * invNorm;
        _result[1] = _quat[1] * invNorm;
        _result[2] = _quat[2] * invNorm;
        _result[3] = _quat[3] * invNorm;
    } else {
        cel_quat_identity(_result);
    }
}

void cel_quat_to_euler(float *_result,
                       const float *_quat) {
    const float x = _quat[0];
    const float y = _quat[1];
    const float z = _quat[2];
    const float w = _quat[3];

    const float yy = y * y;
    const float zz = z * z;
    const float xx = x * x;

    _result[0] = cel_fatan2(2.0f * (x * w - y * z),
                            1.0f - 2.0f * (xx + zz));
    _result[1] = cel_fatan2(2.0f * (y * w + x * z),
                            1.0f - 2.0f * (yy + zz));
    _result[2] = cel_fasin(2.0f * (x * y + z * w));
}

void cel_quat_from_euler(float *result,
                         float heading,
                         float attitude,
                         float bank) {

    const float sx = cel_fsin(heading * 0.5f);
    const float sy = cel_fsin(attitude * 0.5f);
    const float sz = cel_fsin(bank * 0.5f);
    const float cx = cel_fcos(heading * 0.5f);
    const float cy = cel_fcos(attitude * 0.5f);
    const float cz = cel_fcos(bank * 0.5f);

    result[0] = sx * cy * cz - cx * sy * sz;
    result[1] = cx * sy * cz + sx * cy * sz;
    result[2] = cx * cy * sz - sx * sy * cz;
    result[3] = cx * cy * cz + sx * sy * sz;
}

void cel_quat_rotate_axis(float *_result,
                          const float *_axis,
                          float _angle) {
    const float ha = _angle * 0.5f;
    const float ca = cel_fcos(ha);
    const float sa = cel_fsin(ha);
    _result[0] = _axis[0] * sa;
    _result[1] = _axis[1] * sa;
    _result[2] = _axis[2] * sa;
    _result[3] = ca;
}

void cel_quat_rotate_x(float *_result,
                       float _ax) {
    const float hx = _ax * 0.5f;
    const float cx = cel_fcos(hx);
    const float sx = cel_fsin(hx);
    _result[0] = sx;
    _result[1] = 0.0f;
    _result[2] = 0.0f;
    _result[3] = cx;
}

void cel_quat_rotate_y(float *_result,
                       float _ay) {
    const float hy = _ay * 0.5f;
    const float cy = cel_fcos(hy);
    const float sy = cel_fsin(hy);
    _result[0] = 0.0f;
    _result[1] = sy;
    _result[2] = 0.0f;
    _result[3] = cy;
}

void cel_quat_rotate_z(float *_result,
                       float _az) {
    const float hz = _az * 0.5f;
    const float cz = cel_fcos(hz);
    const float sz = cel_fsin(hz);
    _result[0] = 0.0f;
    _result[1] = 0.0f;
    _result[2] = sz;
    _result[3] = cz;
}

void cel_vec3_mul_quat(float *_result,
                       const float *_vec,
                       const float *_quat) {
    float tmp0[4];
    cel_quat_invert(tmp0, _quat);

    float qv[4];
    qv[0] = _vec[0];
    qv[1] = _vec[1];
    qv[2] = _vec[2];
    qv[3] = 0.0f;

    float tmp1[4];
    cel_quat_mul(tmp1, tmp0, qv);

    cel_quat_mul_xyz(_result, tmp1, _quat);
}

void cel_mat4_identity(float *_result) {
    memset(_result, 0, sizeof(float) * 16);
    _result[0] = _result[5] = _result[10] = _result[15] = 1.0f;
}

void cel_mat4_translate(float *_result,
                        float _tx,
                        float _ty,
                        float _tz) {
    cel_mat4_identity(_result);
    _result[12] = _tx;
    _result[13] = _ty;
    _result[14] = _tz;
}

void cel_mat4_scale(float *_result,
                    float _sx,
                    float _sy,
                    float _sz) {
    memset(_result, 0, sizeof(float) * 16);
    _result[0] = _sx;
    _result[5] = _sy;
    _result[10] = _sz;
    _result[15] = 1.0f;
}

void cel_mat4_scale_s(float *_result,
                      float _scale) {
    cel_mat4_scale(_result, _scale, _scale, _scale);
}

void cel_mat4_from_normal(float *_result,
                          const float *_normal,
                          float _scale,
                          const float *_pos) {
    float tangent[3];
    float bitangent[3];
    cel_vec3_tangent_frame(_normal, tangent, bitangent);

    cel_vec3_mul_s(&_result[0], bitangent, _scale);
    cel_vec3_mul_s(&_result[4], _normal, _scale);
    cel_vec3_mul_s(&_result[8], tangent, _scale);

    _result[3] = 0.0f;
    _result[7] = 0.0f;
    _result[11] = 0.0f;
    _result[12] = _pos[0];
    _result[13] = _pos[1];
    _result[14] = _pos[2];
    _result[15] = 1.0f;
}

void cel_mat4_from_normal_a(float *_result,
                            const float *_normal,
                            float _scale,
                            const float *_pos,
                            float _angle) {
    float tangent[3];
    float bitangent[3];
    cel_vec3_tangent_frame_a(_normal, tangent, bitangent, _angle);

    cel_vec3_mul_s(&_result[0], bitangent, _scale);
    cel_vec3_mul_s(&_result[4], _normal, _scale);
    cel_vec3_mul_s(&_result[8], tangent, _scale);

    _result[3] = 0.0f;
    _result[7] = 0.0f;
    _result[11] = 0.0f;
    _result[12] = _pos[0];
    _result[13] = _pos[1];
    _result[14] = _pos[2];
    _result[15] = 1.0f;
}

void cel_mat3_inverse(float *_result,
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

void cel_mat4_move(float *_result,
                   const float *_a) {
    memcpy(_result, _a, sizeof(float) * 16);
}

void cel_mat4_inverse(float *_result,
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

void cel_mat4_quat(float *_result,
                   const float *_quat) {
    const float x = _quat[0];
    const float y = _quat[1];
    const float z = _quat[2];
    const float w = _quat[3];

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

void cel_mat4_quat_translation(float *_result,
                               const float *_quat,
                               const float *_translation) {
    cel_mat4_quat(_result, _quat);
    _result[12] = -(_result[0] * _translation[0] +
                    _result[4] * _translation[1] +
                    _result[8] * _translation[2]);
    _result[13] = -(_result[1] * _translation[0] +
                    _result[5] * _translation[1] +
                    _result[9] * _translation[2]);
    _result[14] = -(_result[2] * _translation[0] +
                    _result[6] * _translation[1] +
                    _result[10] * _translation[2]);
}

void cel_mat4_quat_translation_hmd(float *_result,
                                   const float *_quat,
                                   const float *_translation) {
    float quat[4];
    quat[0] = -_quat[0];
    quat[1] = -_quat[1];
    quat[2] = _quat[2];
    quat[3] = _quat[3];
    cel_mat4_quat_translation(_result, quat, _translation);
}

void cel_mat4_rotate_x(float *_result,
                       float _ax) {
    const float sx = cel_fsin(_ax);
    const float cx = cel_fcos(_ax);

    memset(_result, 0, sizeof(float) * 16);
    _result[0] = 1.0f;
    _result[5] = cx;
    _result[6] = -sx;
    _result[9] = sx;
    _result[10] = cx;
    _result[15] = 1.0f;
}

void cel_mat4_rotate_y(float *_result,
                       float _ay) {
    const float sy = cel_fsin(_ay);
    const float cy = cel_fcos(_ay);

    memset(_result, 0, sizeof(float) * 16);
    _result[0] = cy;
    _result[2] = sy;
    _result[5] = 1.0f;
    _result[8] = -sy;
    _result[10] = cy;
    _result[15] = 1.0f;
}

void cel_mat4_rotate_z(float *_result,
                       float _az) {
    const float sz = cel_fsin(_az);
    const float cz = cel_fcos(_az);

    memset(_result, 0, sizeof(float) * 16);
    _result[0] = cz;
    _result[1] = -sz;
    _result[4] = sz;
    _result[5] = cz;
    _result[10] = 1.0f;
    _result[15] = 1.0f;
}

void cel_mat4_rotate_xy(float *_result,
                        float _ax,
                        float _ay) {
    const float sx = cel_fsin(_ax);
    const float cx = cel_fcos(_ax);
    const float sy = cel_fsin(_ay);
    const float cy = cel_fcos(_ay);

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

void cel_mat4_rotate_xyz(float *_result,
                         float _ax,
                         float _ay,
                         float _az) {
    const float sx = cel_fsin(_ax);
    const float cx = cel_fcos(_ax);
    const float sy = cel_fsin(_ay);
    const float cy = cel_fcos(_ay);
    const float sz = cel_fsin(_az);
    const float cz = cel_fcos(_az);

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

void cel_mat4_rotate_zyx(float *_result,
                         float _ax,
                         float _ay,
                         float _az) {
    const float sx = cel_fsin(_ax);
    const float cx = cel_fcos(_ax);
    const float sy = cel_fsin(_ay);
    const float cy = cel_fcos(_ay);
    const float sz = cel_fsin(_az);
    const float cz = cel_fcos(_az);

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

void cel_mat4_srt(float *_result,
                  float _sx,
                  float _sy,
                  float _sz,
                  float _ax,
                  float _ay,
                  float _az,
                  float _tx,
                  float _ty,
                  float _tz) {
    const float sx = cel_fsin(_ax);
    const float cx = cel_fcos(_ax);
    const float sy = cel_fsin(_ay);
    const float cy = cel_fcos(_ay);
    const float sz = cel_fsin(_az);
    const float cz = cel_fcos(_az);

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

void cel_vec3_mul_mtx(float *_result,
                      const float *_vec,
                      const float *_mat) {
    _result[0] = _vec[0] * _mat[0] + _vec[1] * _mat[4] + _vec[2] * _mat[8] +
                 _mat[12];
    _result[1] = _vec[0] * _mat[1] + _vec[1] * _mat[5] + _vec[2] * _mat[9] +
                 _mat[13];
    _result[2] =
            _vec[0] * _mat[2] + _vec[1] * _mat[6] + _vec[2] * _mat[10] +
            _mat[14];
}

void cel_vec3_mul_mat4_h(float *_result,
                         const float *_vec,
                         const float *_mat) {
    float xx = _vec[0] * _mat[0] + _vec[1] * _mat[4] + _vec[2] * _mat[8] +
               _mat[12];
    float yy = _vec[0] * _mat[1] + _vec[1] * _mat[5] + _vec[2] * _mat[9] +
               _mat[13];
    float zz = _vec[0] * _mat[2] + _vec[1] * _mat[6] + _vec[2] * _mat[10] +
               _mat[14];
    float ww = _vec[0] * _mat[3] + _vec[1] * _mat[7] + _vec[2] * _mat[11] +
               _mat[15];
    float invW = cel_fsign(ww) / ww;
    _result[0] = xx * invW;
    _result[1] = yy * invW;
    _result[2] = zz * invW;
}

void cel_vec4_mul_mtx(float *_result,
                      const float *_vec,
                      const float *_mat) {
    _result[0] = _vec[0] * _mat[0] + _vec[1] * _mat[4] + _vec[2] * _mat[8] +
                 _vec[3] * _mat[12];
    _result[1] = _vec[0] * _mat[1] + _vec[1] * _mat[5] + _vec[2] * _mat[9] +
                 _vec[3] * _mat[13];
    _result[2] =
            _vec[0] * _mat[2] + _vec[1] * _mat[6] + _vec[2] * _mat[10] +
            _vec[3] * _mat[14];
    _result[3] =
            _vec[0] * _mat[3] + _vec[1] * _mat[7] + _vec[2] * _mat[11] +
            _vec[3] * _mat[15];
}

void cel_mat4_mul(float *_result,
                  const float *_a,
                  const float *_b) {
    cel_vec4_mul_mtx(&_result[0], &_a[0], _b);
    cel_vec4_mul_mtx(&_result[4], &_a[4], _b);
    cel_vec4_mul_mtx(&_result[8], &_a[8], _b);
    cel_vec4_mul_mtx(&_result[12], &_a[12], _b);
}

void cel_mat4_transpose(float *_result,
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

/// Convert LH to RH projection matrix and vice versa.
void cel_mat4_proj_flip_handedness(float *_dst,
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

/// Convert LH to RH view matrix and vice versa.
void cel_mat4_view_flip_handedness(float *_dst,
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

void cel_calc_normal(float *_result,
                     float *_va,
                     float *_vb,
                     float *_vc) {
    float ba[3];
    cel_vec3_sub(ba, _vb, _va);

    float ca[3];
    cel_vec3_sub(ca, _vc, _va);

    float baxca[3];
    cel_vec3_cross(baxca, ba, ca);

    cel_vec3_norm(_result, baxca);
}

void cel_calc_plane(float *_result,
                    float *_va,
                    float *_vb,
                    float *_vc) {
    float normal[3];
    cel_calc_normal(normal, _va, _vb, _vc);

    _result[0] = normal[0];
    _result[1] = normal[1];
    _result[2] = normal[2];
    _result[3] = -cel_vec3_dot(normal, _va);
}

void cel_calc_linear_fit_2d(float *_result,
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

void cel_calc_linear_fit_3d(float *_result,
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
                    sumX, sumY, _numPoints,
            };
    float invMtx[9];
    cel_mat3_inverse(invMtx, mtx);

    _result[0] = invMtx[0] * sumXZ + invMtx[1] * sumYZ + invMtx[2] * sumZ;
    _result[1] = invMtx[3] * sumXZ + invMtx[4] * sumYZ + invMtx[5] * sumZ;
    _result[2] = invMtx[6] * sumXZ + invMtx[7] * sumYZ + invMtx[8] * sumZ;
}

void cel_rgb_to_hsv(float *_hsv,
                    const float *_rgb) {
    const float rr = _rgb[0];
    const float gg = _rgb[1];
    const float bb = _rgb[2];

    const float s0 = cel_fstep(bb, gg);

    const float px = cel_flerp(bb, gg, s0);
    const float py = cel_flerp(gg, bb, s0);
    const float pz = cel_flerp(-1.0f, 0.0f, s0);
    const float pw = cel_flerp(2.0f / 3.0f, -1.0f / 3.0f, s0);

    const float s1 = cel_fstep(px, rr);

    const float qx = cel_flerp(px, rr, s1);
    const float qy = py;
    const float qz = cel_flerp(pw, pz, s1);
    const float qw = cel_flerp(rr, px, s1);

    const float dd = qx - fmin(qw, qy);
    const float ee = 1.0e-10f;

    _hsv[0] = cel_fabsolute(qz + (qw - qy) / (6.0f * dd + ee));
    _hsv[1] = dd / (qx + ee);
    _hsv[2] = qx;
}

void cel_hsv_to_rgb(float *_rgb,
                    const float *_hsv) {
    const float hh = _hsv[0];
    const float ss = _hsv[1];
    const float vv = _hsv[2];

    const float px = cel_fabsolute(cel_ffract(hh + 1.0f) * 6.0f - 3.0f);
    const float py = cel_fabsolute(
            cel_ffract(hh + 2.0f / 3.0f) * 6.0f - 3.0f);
    const float pz = cel_fabsolute(
            cel_ffract(hh + 1.0f / 3.0f) * 6.0f - 3.0f);

    _rgb[0] = vv * cel_flerp(1.0f, cel_fsaturate(px - 1.0f), ss);
    _rgb[1] = vv * cel_flerp(1.0f, cel_fsaturate(py - 1.0f), ss);
    _rgb[2] = vv * cel_flerp(1.0f, cel_fsaturate(pz - 1.0f), ss);
}

void cel_mat4_look_at_impl(float *_result,
                           const float *_eye,
                           const float *_view,
                           const float *_up) {
    float up[3] = {0.0f, 1.0f, 0.0f};
    if (NULL != _up) {
        up[0] = _up[0];
        up[1] = _up[1];
        up[2] = _up[2];
    }

    float tmp[4];
    cel_vec3_cross(tmp, up, _view);

    float right[4];
    cel_vec3_norm(right, tmp);

    cel_vec3_cross(up, _view, right);

    memset(_result, 0, sizeof(float) * 16);
    _result[0] = right[0];
    _result[1] = up[0];
    _result[2] = _view[0];

    _result[4] = right[1];
    _result[5] = up[1];
    _result[6] = _view[1];

    _result[8] = right[2];
    _result[9] = up[2];
    _result[10] = _view[2];

    _result[12] = -cel_vec3_dot(right, _eye);
    _result[13] = -cel_vec3_dot(up, _eye);
    _result[14] = -cel_vec3_dot(_view, _eye);
    _result[15] = 1.0f;
}

void cel_mat4_look_at_lh(float *_result,
                         const float *_eye,
                         const float *_at,
                         const float *_up) {
    float tmp[4];
    cel_vec3_sub(tmp, _at, _eye);

    float view[4];
    cel_vec3_norm(view, tmp);

    cel_mat4_look_at_impl(_result, _eye, view, _up);
}

void cel_mat4_look_at_rh(float *_result,
                         const float *_eye,
                         const float *_at,
                         const float *_up) {
    float tmp[4];
    cel_vec3_sub(tmp, _eye, _at);

    float view[4];
    cel_vec3_norm(view, tmp);

    cel_mat4_look_at_impl(_result, _eye, view, _up);
}

void cel_mat4_look_at(float *_result,
                      const float *_eye,
                      const float *_at,
                      const float *_up) {
    cel_mat4_look_at_lh(_result, _eye, _at, _up);
}

void cel_mat4_ortho(float *_result,
                    float _left,
                    float _right,
                    float _bottom,
                    float _top,
                    float _near,
                    float _far,
                    float _offset,
                    bool _oglNdc) {
    cel_mat4_ortho_lh(_result, _left, _right, _bottom, _top,
                      _near, _far, _offset, _oglNdc);
}

void cel_mat4_ortho_lh(float *_result,
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

void cel_mat4_ortho_rh(float *_result,
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
    Left,
    Right,
};

enum NearFar {
    Default,
    Reverse,
};

void cel_mat4_proj_xywh(float *_result,
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

void cel_mat4_proj_impl(float *_result,
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
    cel_mat4_proj_xywh(_result, xx, yy, width, height, _near, _far,
                       _oglNdc, handness);
}

void mat4_proj_impl_fov(float *_result,
                        const float *_fov,
                        float _near,
                        float _far,
                        bool _oglNdc,
                        enum Handness handness) {
    cel_mat4_proj_impl(_result, _fov[0], _fov[1], _fov[2], _fov[3],
                       _near, _far, _oglNdc, handness);
}

void cel_mat4_proj_impl_fovy(float *_result,
                             float _fovy,
                             float _aspect,
                             float _near,
                             float _far,
                             bool _oglNdc,
                             enum Handness handness) {
    const float height = 1.0f / cel_ftan(cel_to_rad(_fovy) * 0.5f);
    const float width = height * 1.0f / _aspect;
    cel_mat4_proj_xywh(_result, 0.0f, 0.0f, width, height, _near,
                       _far,
                       _oglNdc, handness);
}

void cel_mat4_proj(float *_result,
                   float _ut,
                   float _dt,
                   float _lt,
                   float _rt,
                   float _near,
                   float _far,
                   bool _oglNdc) {
    cel_mat4_proj_impl(_result, _ut, _dt, _lt, _rt, _near, _far,
                       _oglNdc, Left);
}

void cel_mat4_proj_fov(float *_result,
                       const float *_fov,
                       float _near,
                       float _far,
                       bool _oglNdc) {
    mat4_proj_impl_fov(_result, _fov, _near, _far, _oglNdc, Left);
}

void cel_mat4_proj_fovy(float *_result,
                        float _fovy,
                        float _aspect,
                        float _near,
                        float _far,
                        bool _oglNdc) {
    cel_mat4_proj_impl_fovy(_result, _fovy, _aspect, _near, _far,
                            _oglNdc, Left);
}

void cel_mat4_proj_lh(float *_result,
                      float _ut,
                      float _dt,
                      float _lt,
                      float _rt,
                      float _near,
                      float _far,
                      bool _oglNdc) {
    cel_mat4_proj_impl(_result, _ut, _dt, _lt, _rt, _near, _far,
                       _oglNdc, Left);
}

void cel_mat4_proj_lh_fov(float *_result,
                          const float *_fov,
                          float _near,
                          float _far,
                          bool _oglNdc) {
    mat4_proj_impl_fov(_result, _fov, _near, _far, _oglNdc, Left);
}

void cel_mat4_proj_lh_fovy(float *_result,
                           float _fovy,
                           float _aspect,
                           float _near,
                           float _far,
                           bool _oglNdc) {
    cel_mat4_proj_impl_fovy(_result, _fovy, _aspect, _near, _far,
                            _oglNdc, Left);
}

void cel_mat4_proj_rh(float *_result,
                      float _ut,
                      float _dt,
                      float _lt,
                      float _rt,
                      float _near,
                      float _far,
                      bool _oglNdc) {
    cel_mat4_proj_impl(_result, _ut, _dt, _lt, _rt, _near, _far,
                       _oglNdc, Right);
}

void cel_mat4_proj_rh_fov(float *_result,
                          const float *_fov,
                          float _near,
                          float _far,
                          bool _oglNdc) {
    mat4_proj_impl_fov(_result, _fov, _near, _far, _oglNdc, Right);
}

void cel_at4_proj_rh_fovy(float *_result,
                          float _fovy,
                          float _aspect,
                          float _near,
                          float _far,
                          bool _oglNdc) {
    cel_mat4_proj_impl_fovy(_result, _fovy, _aspect, _near, _far,
                            _oglNdc, Right);
}

void cel_mat4_proj_inf_xywh(float *_result,
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

void mat4_proj_inf_impl(float *_result,
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
    cel_mat4_proj_inf_xywh(_result, xx, yy, width, height,
                           _near, _oglNdc, nearfar, handness);
}

void cel_mat4_proj_inf_iml_fov(float *_result,
                               const float *_fov,
                               float _near,
                               bool _oglNdc,
                               enum NearFar nearfar,
                               enum Handness handness) {
    mat4_proj_inf_impl(_result, _fov[0], _fov[1], _fov[2],
                       _fov[3], _near, _oglNdc, nearfar, handness);
}

void cel_mat4_proj_inf_impl_fovy(float *_result,
                                 float _fovy,
                                 float _aspect,
                                 float _near,
                                 bool _oglNdc,
                                 enum NearFar nearfar,
                                 enum Handness handness) {
    const float height = 1.0f / cel_ftan(cel_to_rad(_fovy) * 0.5f);
    const float width = height * 1.0f / _aspect;
    cel_mat4_proj_inf_xywh(_result, 0.0f, 0.0f, width,
                           height,
                           _near, _oglNdc, nearfar, handness);
}

void cel_mat4_proj_inf_fov(float *_result,
                           const float *_fov,
                           float _near,
                           bool _oglNdc) {
    cel_mat4_proj_inf_iml_fov(_result, _fov, _near,
                              _oglNdc, Default, Left);
}

void cel_mat4_proj_inf(float *_result,
                       float _ut,
                       float _dt,
                       float _lt,
                       float _rt,
                       float _near,
                       bool _oglNdc) {
    mat4_proj_inf_impl(_result, _ut, _dt, _lt,
                       _rt, _near, _oglNdc, Default, Left);
}

void cel_mat4_proj_inf_fovy(float *_result,
                            float _fovy,
                            float _aspect,
                            float _near,
                            bool _oglNdc) {
    cel_mat4_proj_inf_impl_fovy(_result, _fovy,
                                _aspect, _near,
                                _oglNdc, Default, Left);
}

void cel_mat4_proj_inf_lh(float *_result,
                          float _ut,
                          float _dt,
                          float _lt,
                          float _rt,
                          float _near,
                          bool _oglNdc) {
    mat4_proj_inf_impl(_result, _ut, _dt, _lt,
                       _rt, _near, _oglNdc, Default, Left);
}

void cel_mat4_proj_inf_lh_fov(float *_result,
                              const float *_fov,
                              float _near,
                              bool _oglNdc) {
    cel_mat4_proj_inf_iml_fov(_result, _fov, _near,
                              _oglNdc, Default, Left);
}

void cel_mat4_proj_inf_lh_fovy(float *_result,
                               float _fovy,
                               float _aspect,
                               float _near,
                               bool _oglNdc) {
    cel_mat4_proj_inf_impl_fovy(_result, _fovy,
                                _aspect, _near,
                                _oglNdc, Default, Left);
}

void cel_mat4_proj_inf_rh(float *_result,
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

void cel_mat4_proj_inf_rh_fov(float *_result,
                              const float *_fov,
                              float _near,
                              bool _oglNdc) {
    cel_mat4_proj_inf_iml_fov(_result, _fov, _near,
                              _oglNdc, Default, Right);
}

void cel_mat4_proj_inf_rh_fovy(float *_result,
                               float _fovy,
                               float _aspect,
                               float _near,
                               bool _oglNdc) {
    cel_mat4_proj_inf_impl_fovy(_result, _fovy,
                                _aspect, _near,
                                _oglNdc, Default, Right);
}

void cel_mat4_proj_rev_inf_lh(float *_result,
                              float _ut,
                              float _dt,
                              float _lt,
                              float _rt,
                              float _near,
                              bool _oglNdc) {
    mat4_proj_inf_impl(_result, _ut, _dt, _lt,
                       _rt, _near, _oglNdc, Reverse, Left);
}

void cel_mat4_proj_rev_inf_lh_fov(float *_result,
                                  const float *_fov,
                                  float _near,
                                  bool _oglNdc) {
    cel_mat4_proj_inf_iml_fov(_result, _fov, _near,
                              _oglNdc, Reverse, Left);
}

void cel_mat4_proj_rev_inf_lh_fovy(float *_result,
                                   float _fovy,
                                   float _aspect,
                                   float _near,
                                   bool _oglNdc) {
    cel_mat4_proj_inf_impl_fovy(_result, _fovy,
                                _aspect, _near,
                                _oglNdc, Reverse, Left);
}

void cel_mat4_proj_rev_inf_rh(float *_result,
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

void cel_mat4_proj_rev_inf_rh_fov(float *_result,
                                  const float *_fov,
                                  float _near,
                                  bool _oglNdc) {
    cel_mat4_proj_inf_iml_fov(_result, _fov, _near,
                              _oglNdc, Reverse, Right);
}

void cel_mat4_proj_rev_inf_rh_fovy(float *_result,
                                   float _fovy,
                                   float _aspect,
                                   float _near,
                                   bool _oglNdc) {
    cel_mat4_proj_inf_impl_fovy(_result, _fovy,
                                _aspect, _near,
                                _oglNdc, Reverse, Right);
}


