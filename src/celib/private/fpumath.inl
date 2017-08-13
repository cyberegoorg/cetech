/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */


#ifndef CELIB_FPUMATH_INL
#define CELIB_FPUMATH_INL

//==============================================================================
// Includes
//==============================================================================

#include <math.h>


namespace celib {

    inline uint32_t float_to_bits(float _a) {
        union {
            float f;
            uint32_t ui;
        } u = {_a};
        return u.ui;
    }

    inline float bits_to_float(uint32_t _a) {
        union {
            uint32_t ui;
            float f;
        } u = {_a};
        return u.f;
    }

    inline uint64_t double_to_bits(double _a) {
        union {
            double f;
            uint64_t ui;
        } u = {_a};
        return u.ui;
    }

    inline double bits_to_double(uint64_t _a) {
        union {
            uint64_t ui;
            double f;
        } u = {_a};
        return u.f;
    }


    inline float fabsolute(float _a) {
        return ::fabsf(_a);
    }

    inline float fsin(float _a) {
        return ::sinf(_a);
    }

    inline float fasin(float _a) {
        return ::asinf(_a);
    }

    inline float fcos(float _a) {
        return ::cosf(_a);
    }

    inline float ftan(float _a) {
        return ::tanf(_a);
    }

    inline float facos(float _a) {
        return ::acosf(_a);
    }

    inline float fatan2(float _y,
                        float _x) {
        return ::atan2f(_y, _x);
    }

    inline float fpow(float _a,
                      float _b) {
        return ::powf(_a, _b);
    }

    inline float flog(float _a) {
        return ::logf(_a);
    }

    inline float fsqrt(float _a) {
        return ::sqrtf(_a);
    }

    inline float ffloor(float _f) {
        return ::floorf(_f);
    }

    inline float fceil(float _f) {
        return ::ceilf(_f);
    }

    inline float fmod(float _a,
                      float _b) {
        return ::fmodf(_a, _b);
    }

    inline bool is_nan(float _f) {
        const uint32_t tmp = float_to_bits(_f) & INT32_MAX;
        return tmp > UINT32_C(0x7f800000);
    }

    inline bool is_nan(double _f) {
        const uint64_t tmp = double_to_bits(_f) & INT64_MAX;
        return tmp > UINT64_C(0x7ff0000000000000);
    }

    inline bool is_finite(float _f) {
        const uint32_t tmp = float_to_bits(_f) & INT32_MAX;
        return tmp < UINT32_C(0x7f800000);
    }

    inline bool is_finite(double _f) {
        const uint64_t tmp = double_to_bits(_f) & INT64_MAX;
        return tmp < UINT64_C(0x7ff0000000000000);
    }

    inline bool is_infinite(float _f) {
        const uint32_t tmp = float_to_bits(_f) & INT32_MAX;
        return tmp == UINT32_C(0x7f800000);
    }

    inline bool is_infinite(double _f) {
        const uint64_t tmp = double_to_bits(_f) & INT64_MAX;
        return tmp == UINT64_C(0x7ff0000000000000);
    }

    inline float fround(float _f) {
        return ffloor(_f + 0.5f);
    }

    inline float fmin(float _a,
                      float _b) {
        return _a < _b ? _a : _b;
    }

    inline float fmax(float _a,
                      float _b) {
        return _a > _b ? _a : _b;
    }

    inline float fmin3(float _a,
                       float _b,
                       float _c) {
        return fmin(_a, fmin(_b, _c));
    }

    inline float fmax3(float _a,
                       float _b,
                       float _c) {
        return fmax(_a, fmax(_b, _c));
    }

    inline float fclamp(float _a,
                        float _min,
                        float _max) {
        return fmin(fmax(_a, _min), _max);
    }

    inline float fsaturate(float _a) {
        return fclamp(_a, 0.0f, 1.0f);
    }

    inline float flerp(float _a,
                       float _b,
                       float _t) {
        return _a + (_b - _a) * _t;
    }

    inline float fsign(float _a) {
        return _a < 0.0f ? -1.0f : 1.0f;
    }

    inline float fsq(float _a) {
        return _a * _a;
    }

    inline float fexp2(float _a) {
        return fpow(2.0f, _a);
    }

    inline float flog2(float _a) {
        return flog(_a) * INV_LOG_NAT_2;
    }

    inline float frsqrt(float _a) {
        return 1.0f / fsqrt(_a);
    }

    inline float ffract(float _a) {
        return _a - ffloor(_a);
    }

    inline bool fequal(float _a,
                       float _b,
                       float _epsilon) {
        // http://realtimecollisiondetection.net/blog/?p=89
        const float lhs = fabsolute(_a - _b);
        const float rhs = _epsilon * fmax3(1.0f, fabsolute(_a), fabsolute(_b));
        return lhs <= rhs;
    }

    inline bool fequal(const float *_a,
                       const float *_b,
                       uint32_t _num,
                       float _epsilon) {
        bool equal = fequal(_a[0], _b[0], _epsilon);
        for (uint32_t ii = 1; equal && ii < _num; ++ii) {
            equal = fequal(_a[ii], _b[ii], _epsilon);
        }
        return equal;
    }

    inline float fwrap(float _a,
                       float _wrap) {
        const float mod = fmod(_a, _wrap);
        const float result = mod < 0.0f ? _wrap + mod : mod;
        return result;
    }

    inline float fstep(float _edge,
                       float _a) {
        return _a < _edge ? 0.0f : 1.0f;
    }

    inline float fpulse(float _a,
                        float _start,
                        float _end) {
        return fstep(_a, _start) - fstep(_a, _end);
    }

    inline float fsmoothstep(float _a) {
        return fsq(_a) * (3.0f - 2.0f * _a);
    }

    inline float fbias(float _time,
                       float _bias) {
        return _time / (((1.0f / _bias - 2.0f) * (1.0f - _time)) + 1.0f);
    }

    inline float fgain(float _time,
                       float _gain) {
        if (_time < 0.5f) {
            return fbias(_time * 2.0f, _gain) * 0.5f;
        }

        return fbias(_time * 2.0f - 1.0f, 1.0f - _gain) * 0.5f + 0.5f;
    }


    inline float to_rad(float _deg) {
        return _deg * PI / 180.0f;
    }

    inline float to_deg(float _rad) {
        return _rad * 180.0f / PI;
    }


    inline float angle_diff(float _a,
                            float _b) {
        const float dist = fwrap(_b - _a, PI * 2.0f);
        return fwrap(dist * 2.0f, PI * 2.0f) - dist;
    }

    inline float angle_lerp(float _a,
                            float _b,
                            float _t) {
        return _a + angle_diff(_a, _b) * _t;
    }

    inline void vec3_move(float *_result,
                          const float *_a) {
        _result[0] = _a[0];
        _result[1] = _a[1];
        _result[2] = _a[2];
    }

    inline void vec3_abs(float *_result,
                         const float *_a) {
        _result[0] = fabsolute(_a[0]);
        _result[1] = fabsolute(_a[1]);
        _result[2] = fabsolute(_a[2]);
    }

    inline void vec3_neg(float *_result,
                         const float *_a) {
        _result[0] = -_a[0];
        _result[1] = -_a[1];
        _result[2] = -_a[2];
    }

    inline void vec3_add(float *_result,
                         const float *_a,
                         const float *_b) {
        _result[0] = _a[0] + _b[0];
        _result[1] = _a[1] + _b[1];
        _result[2] = _a[2] + _b[2];
    }

    inline void vec3_add(float *_result,
                         const float *_a,
                         float _b) {
        _result[0] = _a[0] + _b;
        _result[1] = _a[1] + _b;
        _result[2] = _a[2] + _b;
    }

    inline void vec3_sub(float *_result,
                         const float *_a,
                         const float *_b) {
        _result[0] = _a[0] - _b[0];
        _result[1] = _a[1] - _b[1];
        _result[2] = _a[2] - _b[2];
    }

    inline void vec3_sub(float *_result,
                         const float *_a,
                         float _b) {
        _result[0] = _a[0] - _b;
        _result[1] = _a[1] - _b;
        _result[2] = _a[2] - _b;
    }

    inline void vec3_mul(float *_result,
                         const float *_a,
                         const float *_b) {
        _result[0] = _a[0] * _b[0];
        _result[1] = _a[1] * _b[1];
        _result[2] = _a[2] * _b[2];
    }

    inline void vec3_mul(float *_result,
                         const float *_a,
                         float _b) {
        _result[0] = _a[0] * _b;
        _result[1] = _a[1] * _b;
        _result[2] = _a[2] * _b;
    }

    inline float vec3_dot(const float *_a,
                          const float *_b) {
        return _a[0] * _b[0] + _a[1] * _b[1] + _a[2] * _b[2];
    }

    inline void vec3_cross(float *_result,
                           const float *_a,
                           const float *_b) {
        _result[0] = _a[1] * _b[2] - _a[2] * _b[1];
        _result[1] = _a[2] * _b[0] - _a[0] * _b[2];
        _result[2] = _a[0] * _b[1] - _a[1] * _b[0];
    }

    inline float vec3_length(const float *_a) {
        return fsqrt(vec3_dot(_a, _a));
    }

    inline void vec3_lerp(float *_result,
                          const float *_a,
                          const float *_b,
                          float _t) {
        _result[0] = flerp(_a[0], _b[0], _t);
        _result[1] = flerp(_a[1], _b[1], _t);
        _result[2] = flerp(_a[2], _b[2], _t);
    }

    inline void vec3_lerp(float *_result,
                          const float *_a,
                          const float *_b,
                          const float *_c) {
        _result[0] = flerp(_a[0], _b[0], _c[0]);
        _result[1] = flerp(_a[1], _b[1], _c[1]);
        _result[2] = flerp(_a[2], _b[2], _c[2]);
    }

    inline float vec3_norm(float *_result,
                           const float *_a) {
        const float len = vec3_length(_a);
        const float invLen = 1.0f / len;
        _result[0] = _a[0] * invLen;
        _result[1] = _a[1] * invLen;
        _result[2] = _a[2] * invLen;
        return len;
    }

    inline void vec3_min(float *_result,
                         const float *_a,
                         const float *_b) {
        _result[0] = fmin(_a[0], _b[0]);
        _result[1] = fmin(_a[1], _b[1]);
        _result[2] = fmin(_a[2], _b[2]);
    }

    inline void vec3_max(float *_result,
                         const float *_a,
                         const float *_b) {
        _result[0] = fmax(_a[0], _b[0]);
        _result[1] = fmax(_a[1], _b[1]);
        _result[2] = fmax(_a[2], _b[2]);
    }

    inline void vec3_rcp(float *_result,
                         const float *_a) {
        _result[0] = 1.0f / _a[0];
        _result[1] = 1.0f / _a[1];
        _result[2] = 1.0f / _a[2];
    }

    inline void vec3_tangent_frame(const float *_n,
                                   float *_t,
                                   float *_b) {
        const float nx = _n[0];
        const float ny = _n[1];
        const float nz = _n[2];

        if (celib::fabsolute(nx) > celib::fabsolute(nz)) {
            float invLen = 1.0f / celib::fsqrt(nx * nx + nz * nz);
            _t[0] = -nz * invLen;
            _t[1] = 0.0f;
            _t[2] = nx * invLen;
        } else {
            float invLen = 1.0f / celib::fsqrt(ny * ny + nz * nz);
            _t[0] = 0.0f;
            _t[1] = nz * invLen;
            _t[2] = -ny * invLen;
        }

        celib::vec3_cross(_b, _n, _t);
    }

    inline void vec3_tangent_frame(const float *_n,
                                   float *_t,
                                   float *_b,
                                   float _angle) {
        vec3_tangent_frame(_n, _t, _b);

        const float sa = fsin(_angle);
        const float ca = fcos(_angle);

        _t[0] = -sa * _b[0] + ca * _t[0];
        _t[1] = -sa * _b[1] + ca * _t[1];
        _t[2] = -sa * _b[2] + ca * _t[2];

        celib::vec3_cross(_b, _n, _t);
    }

    inline void vec3_from_lat_long(float *_vec,
                                   float _u,
                                   float _v) {
        const float phi = _u * celib::PI2;
        const float theta = _v * celib::PI;

        const float st = celib::fsin(theta);
        const float sp = celib::fsin(phi);
        const float ct = celib::fcos(theta);
        const float cp = celib::fcos(phi);

        _vec[0] = -st * sp;
        _vec[1] = ct;
        _vec[2] = -st * cp;
    }

    inline void vec3_to_lat_long(float *_u,
                                 float *_v,
                                 const float *_vec) {
        const float phi = celib::fatan2(_vec[0], _vec[2]);
        const float theta = celib::facos(_vec[1]);

        *_u = (celib::PI + phi) * celib::INV_PI * 0.5f;
        *_v = theta * celib::INV_PI;
    }

    inline void vec4_move(float *_result,
                          const float *_a) {
        _result[0] = _a[0];
        _result[1] = _a[1];
        _result[2] = _a[2];
        _result[3] = _a[3];
    }

    inline void quat_identity(float *_result) {
        _result[0] = 0.0f;
        _result[1] = 0.0f;
        _result[2] = 0.0f;
        _result[3] = 1.0f;
    }

    inline void quat_move(float *_result,
                          const float *_a) {
        _result[0] = _a[0];
        _result[1] = _a[1];
        _result[2] = _a[2];
        _result[3] = _a[3];
    }

    inline void quat_mul_xyz(float *_result,
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

    inline void quat_mul(float *_result,
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

    inline void quat_invert(float *_result,
                            const float *_quat) {
        _result[0] = -_quat[0];
        _result[1] = -_quat[1];
        _result[2] = -_quat[2];
        _result[3] = _quat[3];
    }

    inline float quat_dot(const float *_a,
                          const float *_b) {
        return _a[0] * _b[0]
               + _a[1] * _b[1]
               + _a[2] * _b[2]
               + _a[3] * _b[3];
    }

    inline void quat_norm(float *_result,
                          const float *_quat) {
        const float norm = quat_dot(_quat, _quat);
        if (0.0f < norm) {
            const float invNorm = 1.0f / fsqrt(norm);
            _result[0] = _quat[0] * invNorm;
            _result[1] = _quat[1] * invNorm;
            _result[2] = _quat[2] * invNorm;
            _result[3] = _quat[3] * invNorm;
        } else {
            quat_identity(_result);
        }
    }

    inline void quat_to_euler(float *_result,
                              const float *_quat) {
        const float x = _quat[0];
        const float y = _quat[1];
        const float z = _quat[2];
        const float w = _quat[3];

        const float yy = y * y;
        const float zz = z * z;

        const float xx = x * x;
        _result[0] = fatan2(2.0f * (x * w - y * z), 1.0f - 2.0f * (xx + zz));
        _result[1] = fatan2(2.0f * (y * w + x * z), 1.0f - 2.0f * (yy + zz));
        _result[2] = fasin(2.0f * (x * y + z * w));
    }

    inline void quatFromEuler(float *result,
                              float heading,
                              float attitude,
                              float bank) {

        const float sx = fsin(heading * 0.5f);
        const float sy = fsin(attitude * 0.5f);
        const float sz = fsin(bank * 0.5f);
        const float cx = fcos(heading * 0.5f);
        const float cy = fcos(attitude * 0.5f);
        const float cz = fcos(bank * 0.5f);

        result[0] = sx * cy * cz - cx * sy * sz;
        result[1] = cx * sy * cz + sx * cy * sz;
        result[2] = cx * cy * sz - sx * sy * cz;
        result[3] = cx * cy * cz + sx * sy * sz;
    }

    inline void quat_rotate_axis(float *_result,
                                 const float *_axis,
                                 float _angle) {
        const float ha = _angle * 0.5f;
        const float ca = fcos(ha);
        const float sa = fsin(ha);
        _result[0] = _axis[0] * sa;
        _result[1] = _axis[1] * sa;
        _result[2] = _axis[2] * sa;
        _result[3] = ca;
    }

    inline void quat_rotate_x(float *_result,
                              float _ax) {
        const float hx = _ax * 0.5f;
        const float cx = fcos(hx);
        const float sx = fsin(hx);
        _result[0] = sx;
        _result[1] = 0.0f;
        _result[2] = 0.0f;
        _result[3] = cx;
    }

    inline void quat_rotate_y(float *_result,
                              float _ay) {
        const float hy = _ay * 0.5f;
        const float cy = fcos(hy);
        const float sy = fsin(hy);
        _result[0] = 0.0f;
        _result[1] = sy;
        _result[2] = 0.0f;
        _result[3] = cy;
    }

    inline void quat_rotate_z(float *_result,
                              float _az) {
        const float hz = _az * 0.5f;
        const float cz = fcos(hz);
        const float sz = fsin(hz);
        _result[0] = 0.0f;
        _result[1] = 0.0f;
        _result[2] = sz;
        _result[3] = cz;
    }

    inline void vec3_mul_quat(float *_result,
                              const float *_vec,
                              const float *_quat) {
        float tmp0[4];
        quat_invert(tmp0, _quat);

        float qv[4];
        qv[0] = _vec[0];
        qv[1] = _vec[1];
        qv[2] = _vec[2];
        qv[3] = 0.0f;

        float tmp1[4];
        quat_mul(tmp1, tmp0, qv);

        quat_mul_xyz(_result, tmp1, _quat);
    }

    inline void mat4_identity(float *_result) {
        mem_set(_result, 0, sizeof(float) * 16);
        _result[0] = _result[5] = _result[10] = _result[15] = 1.0f;
    }

    inline void mat4_translate(float *_result,
                               float _tx,
                               float _ty,
                               float _tz) {
        mat4_identity(_result);
        _result[12] = _tx;
        _result[13] = _ty;
        _result[14] = _tz;
    }

    inline void mat4_scale(float *_result,
                           float _sx,
                           float _sy,
                           float _sz) {
        mem_set(_result, 0, sizeof(float) * 16);
        _result[0] = _sx;
        _result[5] = _sy;
        _result[10] = _sz;
        _result[15] = 1.0f;
    }

    inline void mat4_scale(float *_result,
                           float _scale) {
        mat4_scale(_result, _scale, _scale, _scale);
    }

    inline void mat4_from_normal(float *_result,
                                 const float *_normal,
                                 float _scale,
                                 const float *_pos) {
        float tangent[3];
        float bitangent[3];
        vec3_tangent_frame(_normal, tangent, bitangent);

        vec3_mul(&_result[0], bitangent, _scale);
        vec3_mul(&_result[4], _normal, _scale);
        vec3_mul(&_result[8], tangent, _scale);

        _result[3] = 0.0f;
        _result[7] = 0.0f;
        _result[11] = 0.0f;
        _result[12] = _pos[0];
        _result[13] = _pos[1];
        _result[14] = _pos[2];
        _result[15] = 1.0f;
    }

    inline void mat4_from_normal(float *_result,
                                 const float *_normal,
                                 float _scale,
                                 const float *_pos,
                                 float _angle) {
        float tangent[3];
        float bitangent[3];
        vec3_tangent_frame(_normal, tangent, bitangent, _angle);

        vec3_mul(&_result[0], bitangent, _scale);
        vec3_mul(&_result[4], _normal, _scale);
        vec3_mul(&_result[8], tangent, _scale);

        _result[3] = 0.0f;
        _result[7] = 0.0f;
        _result[11] = 0.0f;
        _result[12] = _pos[0];
        _result[13] = _pos[1];
        _result[14] = _pos[2];
        _result[15] = 1.0f;
    }

    inline void mat3_inverse(float *_result,
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

    inline void mat4_move(float *_result,
                          const float *_a) {
        mem_copy(_result, _a, sizeof(float) * 16);
    }

    inline void mat4_inverse(float *_result,
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

    inline void mat4_quat(float *_result,
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

    inline void mat4_quat_translation(float *_result,
                                      const float *_quat,
                                      const float *_translation) {
        mat4_quat(_result, _quat);
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

    inline void mat4_quat_translation_hmd(float *_result,
                                          const float *_quat,
                                          const float *_translation) {
        float quat[4];
        quat[0] = -_quat[0];
        quat[1] = -_quat[1];
        quat[2] = _quat[2];
        quat[3] = _quat[3];
        mat4_quat_translation(_result, quat, _translation);
    }

    inline void mat4_look_AtImpl(float *_result,
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
        vec3_cross(tmp, up, _view);

        float right[4];
        vec3_norm(right, tmp);

        vec3_cross(up, _view, right);

        mem_set(_result, 0, sizeof(float) * 16);
        _result[0] = right[0];
        _result[1] = up[0];
        _result[2] = _view[0];

        _result[4] = right[1];
        _result[5] = up[1];
        _result[6] = _view[1];

        _result[8] = right[2];
        _result[9] = up[2];
        _result[10] = _view[2];

        _result[12] = -vec3_dot(right, _eye);
        _result[13] = -vec3_dot(up, _eye);
        _result[14] = -vec3_dot(_view, _eye);
        _result[15] = 1.0f;
    }

    inline void mat4_look_at_lh(float *_result,
                                const float *_eye,
                                const float *_at,
                                const float *_up) {
        float tmp[4];
        vec3_sub(tmp, _at, _eye);

        float view[4];
        vec3_norm(view, tmp);

        mat4_look_AtImpl(_result, _eye, view, _up);
    }

    inline void mat4_look_at_rh(float *_result,
                                const float *_eye,
                                const float *_at,
                                const float *_up) {
        float tmp[4];
        vec3_sub(tmp, _eye, _at);

        float view[4];
        vec3_norm(view, tmp);

        mat4_look_AtImpl(_result, _eye, view, _up);
    }

    inline void mat4_look_at(float *_result,
                             const float *_eye,
                             const float *_at,
                             const float *_up) {
        mat4_look_at_lh(_result, _eye, _at, _up);
    }

    template<Handness::Enum HandnessT>
    inline void mat4_proj_xywh(float *_result,
                               float _x,
                               float _y,
                               float _width,
                               float _height,
                               float _near,
                               float _far,
                               bool _oglNdc) {
        const float diff = _far - _near;
        const float aa = _oglNdc ? (_far + _near) / diff : _far / diff;
        const float bb = _oglNdc ? (2.0f * _far * _near) / diff : _near * aa;

        mem_set(_result, 0, sizeof(float) * 16);
        _result[0] = _width;
        _result[5] = _height;
        _result[8] = (Handness::Right == HandnessT) ? _x : -_x;
        _result[9] = (Handness::Right == HandnessT) ? _y : -_y;
        _result[10] = (Handness::Right == HandnessT) ? -aa : aa;
        _result[11] = (Handness::Right == HandnessT) ? -1.0f : 1.0f;
        _result[14] = -bb;
    }

    template<Handness::Enum HandnessT>
    inline void mtxProjImpl(float *_result,
                            float _ut,
                            float _dt,
                            float _lt,
                            float _rt,
                            float _near,
                            float _far,
                            bool _oglNdc) {
        const float invDiffRl = 1.0f / (_rt - _lt);
        const float invDiffUd = 1.0f / (_ut - _dt);
        const float width = 2.0f * _near * invDiffRl;
        const float height = 2.0f * _near * invDiffUd;
        const float xx = (_rt + _lt) * invDiffRl;
        const float yy = (_ut + _dt) * invDiffUd;
        mat4_proj_xywh<HandnessT>(_result, xx, yy, width, height, _near, _far,
                                  _oglNdc);
    }

    template<Handness::Enum HandnessT>
    inline void mtxProjImpl(float *_result,
                            const float _fov[4],
                            float _near,
                            float _far,
                            bool _oglNdc) {
        mtxProjImpl<HandnessT>(_result, _fov[0], _fov[1], _fov[2], _fov[3],
                               _near, _far, _oglNdc);
    }

    template<Handness::Enum HandnessT>
    inline void mtxProjImpl(float *_result,
                            float _fovy,
                            float _aspect,
                            float _near,
                            float _far,
                            bool _oglNdc) {
        const float height = 1.0f / ftan(to_rad(_fovy) * 0.5f);
        const float width = height * 1.0f / _aspect;
        mat4_proj_xywh<HandnessT>(_result, 0.0f, 0.0f, width, height, _near,
                                  _far,
                                  _oglNdc);
    }

    inline void mat4_proj(float *_result,
                          float _ut,
                          float _dt,
                          float _lt,
                          float _rt,
                          float _near,
                          float _far,
                          bool _oglNdc) {
        mtxProjImpl<Handness::Left>(_result, _ut, _dt, _lt, _rt, _near, _far,
                                    _oglNdc);
    }

    inline void mat4_proj(float *_result,
                          const float *_fov,
                          float _near,
                          float _far,
                          bool _oglNdc) {
        mtxProjImpl<Handness::Left>(_result, _fov, _near, _far, _oglNdc);
    }

    inline void mat4_proj(float *_result,
                          float _fovy,
                          float _aspect,
                          float _near,
                          float _far,
                          bool _oglNdc) {
        mtxProjImpl<Handness::Left>(_result, _fovy, _aspect, _near, _far,
                                    _oglNdc);
    }

    inline void mat4_proj_lh(float *_result,
                             float _ut,
                             float _dt,
                             float _lt,
                             float _rt,
                             float _near,
                             float _far,
                             bool _oglNdc) {
        mtxProjImpl<Handness::Left>(_result, _ut, _dt, _lt, _rt, _near, _far,
                                    _oglNdc);
    }

    inline void mat4_proj_lh(float *_result,
                             const float *_fov,
                             float _near,
                             float _far,
                             bool _oglNdc) {
        mtxProjImpl<Handness::Left>(_result, _fov, _near, _far, _oglNdc);
    }

    inline void mat4_proj_lh(float *_result,
                             float _fovy,
                             float _aspect,
                             float _near,
                             float _far,
                             bool _oglNdc) {
        mtxProjImpl<Handness::Left>(_result, _fovy, _aspect, _near, _far,
                                    _oglNdc);
    }

    inline void mat4_proj_rh(float *_result,
                             float _ut,
                             float _dt,
                             float _lt,
                             float _rt,
                             float _near,
                             float _far,
                             bool _oglNdc) {
        mtxProjImpl<Handness::Right>(_result, _ut, _dt, _lt, _rt, _near, _far,
                                     _oglNdc);
    }

    inline void mat4_proj_rh(float *_result,
                             const float *_fov,
                             float _near,
                             float _far,
                             bool _oglNdc) {
        mtxProjImpl<Handness::Right>(_result, _fov, _near, _far, _oglNdc);
    }

    inline void mat4_proj_rh(float *_result,
                             float _fovy,
                             float _aspect,
                             float _near,
                             float _far,
                             bool _oglNdc) {
        mtxProjImpl<Handness::Right>(_result, _fovy, _aspect, _near, _far,
                                     _oglNdc);
    }

    template<NearFar::Enum NearFarT, Handness::Enum HandnessT>
    inline void mat4_proj_inf_xywh(float *_result,
                                   float _x,
                                   float _y,
                                   float _width,
                                   float _height,
                                   float _near,
                                   bool _oglNdc) {
        float aa;
        float bb;
        if (CEL_ENABLED(NearFar::Reverse == NearFarT)) {
            aa = _oglNdc ? -1.0f : 0.0f;
            bb = _oglNdc ? -2.0f * _near : -_near;
        } else {
            aa = 1.0f;
            bb = _oglNdc ? 2.0f * _near : _near;
        }

        mem_set(_result, 0, sizeof(float) * 16);
        _result[0] = _width;
        _result[5] = _height;
        _result[8] = (Handness::Right == HandnessT) ? _x : -_x;
        _result[9] = (Handness::Right == HandnessT) ? _y : -_y;
        _result[10] = (Handness::Right == HandnessT) ? -aa : aa;
        _result[11] = (Handness::Right == HandnessT) ? -1.0f : 1.0f;
        _result[14] = -bb;
    }

    template<NearFar::Enum NearFarT, Handness::Enum HandnessT>
    inline void mtxProjInfImpl(float *_result,
                               float _ut,
                               float _dt,
                               float _lt,
                               float _rt,
                               float _near,
                               bool _oglNdc) {
        const float invDiffRl = 1.0f / (_rt - _lt);
        const float invDiffUd = 1.0f / (_ut - _dt);
        const float width = 2.0f * _near * invDiffRl;
        const float height = 2.0f * _near * invDiffUd;
        const float xx = (_rt + _lt) * invDiffRl;
        const float yy = (_ut + _dt) * invDiffUd;
        mat4_proj_inf_xywh<NearFarT, HandnessT>(_result, xx, yy, width, height,
                                                _near, _oglNdc);
    }

    template<NearFar::Enum NearFarT, Handness::Enum HandnessT>
    inline void mtxProjInfImpl(float *_result,
                               const float _fov[4],
                               float _near,
                               bool _oglNdc) {
        mtxProjInfImpl<NearFarT, HandnessT>(_result, _fov[0], _fov[1], _fov[2],
                                            _fov[3], _near, _oglNdc);
    }

    template<NearFar::Enum NearFarT, Handness::Enum HandnessT>
    inline void mtxProjInfImpl(float *_result,
                               float _fovy,
                               float _aspect,
                               float _near,
                               bool _oglNdc) {
        const float height = 1.0f / ftan(to_rad(_fovy) * 0.5f);
        const float width = height * 1.0f / _aspect;
        mat4_proj_inf_xywh<NearFarT, HandnessT>(_result, 0.0f, 0.0f, width,
                                                height,
                                                _near, _oglNdc);
    }

    inline void mat4_proj_inf(float *_result,
                              const float *_fov,
                              float _near,
                              bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Left>(_result, _fov, _near,
                                                         _oglNdc);
    }

    inline void mat4_proj_inf(float *_result,
                              float _ut,
                              float _dt,
                              float _lt,
                              float _rt,
                              float _near,
                              bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Left>(_result, _ut, _dt, _lt,
                                                         _rt, _near, _oglNdc);
    }

    inline void mat4_proj_inf(float *_result,
                              float _fovy,
                              float _aspect,
                              float _near,
                              bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Left>(_result, _fovy,
                                                         _aspect, _near,
                                                         _oglNdc);
    }

    inline void mat4_proj_inf_lh(float *_result,
                                 float _ut,
                                 float _dt,
                                 float _lt,
                                 float _rt,
                                 float _near,
                                 bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Left>(_result, _ut, _dt, _lt,
                                                         _rt, _near, _oglNdc);
    }

    inline void mat4_proj_inf_lh(float *_result,
                                 const float *_fov,
                                 float _near,
                                 bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Left>(_result, _fov, _near,
                                                         _oglNdc);
    }

    inline void mat4_proj_inf_lh(float *_result,
                                 float _fovy,
                                 float _aspect,
                                 float _near,
                                 bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Left>(_result, _fovy,
                                                         _aspect, _near,
                                                         _oglNdc);
    }

    inline void mat4_proj_inf_rh(float *_result,
                                 float _ut,
                                 float _dt,
                                 float _lt,
                                 float _rt,
                                 float _near,
                                 bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Right>(_result, _ut, _dt,
                                                          _lt, _rt, _near,
                                                          _oglNdc);
    }

    inline void mat4_proj_inf_rh(float *_result,
                                 const float *_fov,
                                 float _near,
                                 bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Right>(_result, _fov, _near,
                                                          _oglNdc);
    }

    inline void mat4_proj_inf_rh(float *_result,
                                 float _fovy,
                                 float _aspect,
                                 float _near,
                                 bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Right>(_result, _fovy,
                                                          _aspect, _near,
                                                          _oglNdc);
    }

    inline void mat4_proj_rev_inf_lh(float *_result,
                                     float _ut,
                                     float _dt,
                                     float _lt,
                                     float _rt,
                                     float _near,
                                     bool _oglNdc) {
        mtxProjInfImpl<NearFar::Reverse, Handness::Left>(_result, _ut, _dt, _lt,
                                                         _rt, _near, _oglNdc);
    }

    inline void mat4_proj_rev_inf_lh(float *_result,
                                     const float *_fov,
                                     float _near,
                                     bool _oglNdc) {
        mtxProjInfImpl<NearFar::Reverse, Handness::Left>(_result, _fov, _near,
                                                         _oglNdc);
    }

    inline void mat4_proj_rev_inf_lh(float *_result,
                                     float _fovy,
                                     float _aspect,
                                     float _near,
                                     bool _oglNdc) {
        mtxProjInfImpl<NearFar::Reverse, Handness::Left>(_result, _fovy,
                                                         _aspect, _near,
                                                         _oglNdc);
    }

    inline void mat4_proj_rev_inf_rh(float *_result,
                                     float _ut,
                                     float _dt,
                                     float _lt,
                                     float _rt,
                                     float _near,
                                     bool _oglNdc) {
        mtxProjInfImpl<NearFar::Reverse, Handness::Right>(_result, _ut, _dt,
                                                          _lt, _rt, _near,
                                                          _oglNdc);
    }

    inline void mat4_proj_rev_inf_rh(float *_result,
                                     const float *_fov,
                                     float _near,
                                     bool _oglNdc) {
        mtxProjInfImpl<NearFar::Reverse, Handness::Right>(_result, _fov, _near,
                                                          _oglNdc);
    }

    inline void mat4_proj_rev_inf_rh(float *_result,
                                     float _fovy,
                                     float _aspect,
                                     float _near,
                                     bool _oglNdc) {
        mtxProjInfImpl<NearFar::Reverse, Handness::Right>(_result, _fovy,
                                                          _aspect, _near,
                                                          _oglNdc);
    }

    template<Handness::Enum HandnessT>
    inline void mtxOrthoImpl(float *_result,
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

        mem_set(_result, 0, sizeof(float) * 16);
        _result[0] = aa;
        _result[5] = bb;
        _result[10] = (Handness::Right == HandnessT) ? -cc : cc;
        _result[12] = dd + _offset;
        _result[13] = ee;
        _result[14] = ff;
        _result[15] = 1.0f;
    }

    inline void mat4_ortho(float *_result,
                           float _left,
                           float _right,
                           float _bottom,
                           float _top,
                           float _near,
                           float _far,
                           float _offset,
                           bool _oglNdc) {
        mtxOrthoImpl<Handness::Left>(_result, _left, _right, _bottom, _top,
                                     _near, _far, _offset, _oglNdc);
    }

    inline void mat4_ortho_lh(float *_result,
                              float _left,
                              float _right,
                              float _bottom,
                              float _top,
                              float _near,
                              float _far,
                              float _offset,
                              bool _oglNdc) {
        mtxOrthoImpl<Handness::Left>(_result, _left, _right, _bottom, _top,
                                     _near, _far, _offset, _oglNdc);
    }

    inline void mat4_ortho_rh(float *_result,
                              float _left,
                              float _right,
                              float _bottom,
                              float _top,
                              float _near,
                              float _far,
                              float _offset,
                              bool _oglNdc) {
        mtxOrthoImpl<Handness::Right>(_result, _left, _right, _bottom, _top,
                                      _near, _far, _offset, _oglNdc);
    }

    inline void mat4_rotate_x(float *_result,
                              float _ax) {
        const float sx = fsin(_ax);
        const float cx = fcos(_ax);

        mem_set(_result, 0, sizeof(float) * 16);
        _result[0] = 1.0f;
        _result[5] = cx;
        _result[6] = -sx;
        _result[9] = sx;
        _result[10] = cx;
        _result[15] = 1.0f;
    }

    inline void mat4_rotate_y(float *_result,
                              float _ay) {
        const float sy = fsin(_ay);
        const float cy = fcos(_ay);

        mem_set(_result, 0, sizeof(float) * 16);
        _result[0] = cy;
        _result[2] = sy;
        _result[5] = 1.0f;
        _result[8] = -sy;
        _result[10] = cy;
        _result[15] = 1.0f;
    }

    inline void mat4_rotate_z(float *_result,
                              float _az) {
        const float sz = fsin(_az);
        const float cz = fcos(_az);

        mem_set(_result, 0, sizeof(float) * 16);
        _result[0] = cz;
        _result[1] = -sz;
        _result[4] = sz;
        _result[5] = cz;
        _result[10] = 1.0f;
        _result[15] = 1.0f;
    }

    inline void mat4_rotate_xy(float *_result,
                               float _ax,
                               float _ay) {
        const float sx = fsin(_ax);
        const float cx = fcos(_ax);
        const float sy = fsin(_ay);
        const float cy = fcos(_ay);

        mem_set(_result, 0, sizeof(float) * 16);
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

    inline void mat4_rotate_xyz(float *_result,
                                float _ax,
                                float _ay,
                                float _az) {
        const float sx = fsin(_ax);
        const float cx = fcos(_ax);
        const float sy = fsin(_ay);
        const float cy = fcos(_ay);
        const float sz = fsin(_az);
        const float cz = fcos(_az);

        mem_set(_result, 0, sizeof(float) * 16);
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

    inline void mat4_rotate_zyx(float *_result,
                                float _ax,
                                float _ay,
                                float _az) {
        const float sx = fsin(_ax);
        const float cx = fcos(_ax);
        const float sy = fsin(_ay);
        const float cy = fcos(_ay);
        const float sz = fsin(_az);
        const float cz = fcos(_az);

        mem_set(_result, 0, sizeof(float) * 16);
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

    inline void mtxSRT(float *_result,
                       float _sx,
                       float _sy,
                       float _sz,
                       float _ax,
                       float _ay,
                       float _az,
                       float _tx,
                       float _ty,
                       float _tz) {
        const float sx = fsin(_ax);
        const float cx = fcos(_ax);
        const float sy = fsin(_ay);
        const float cy = fcos(_ay);
        const float sz = fsin(_az);
        const float cz = fcos(_az);

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

    inline void vec3_mul_mtx(float *_result,
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

    inline void vec3_mul_mat4_h(float *_result,
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
        float invW = fsign(ww) / ww;
        _result[0] = xx * invW;
        _result[1] = yy * invW;
        _result[2] = zz * invW;
    }

    inline void vec4_mul_mtx(float *_result,
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

    inline void mat4_mul(float *_result,
                         const float *_a,
                         const float *_b) {
        vec4_mul_mtx(&_result[0], &_a[0], _b);
        vec4_mul_mtx(&_result[4], &_a[4], _b);
        vec4_mul_mtx(&_result[8], &_a[8], _b);
        vec4_mul_mtx(&_result[12], &_a[12], _b);
    }

    inline void mat4_transpose(float *_result,
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
    inline void mat4_proj_flip_handedness(float *_dst,
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
    inline void mat4_view_flip_handedness(float *_dst,
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

    inline void calc_normal(float *_result,
                            float *_va,
                            float *_vb,
                            float *_vc) {
        float ba[3];
        vec3_sub(ba, _vb, _va);

        float ca[3];
        vec3_sub(ca, _vc, _va);

        float baxca[3];
        vec3_cross(baxca, ba, ca);

        vec3_norm(_result, baxca);
    }

    inline void calc_plane(float *_result,
                           float *_va,
                           float *_vb,
                           float *_vc) {
        float normal[3];
        calc_normal(normal, _va, _vb, _vc);

        _result[0] = normal[0];
        _result[1] = normal[1];
        _result[2] = normal[2];
        _result[3] = -vec3_dot(normal, _va);
    }

    inline void calc_linear_fit_2d(float *_result,
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

    inline void calc_linear_fit_3d(float *_result,
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
                        sumX, sumY, float(_numPoints),
                };
        float invMtx[9];
        mat3_inverse(invMtx, mtx);

        _result[0] = invMtx[0] * sumXZ + invMtx[1] * sumYZ + invMtx[2] * sumZ;
        _result[1] = invMtx[3] * sumXZ + invMtx[4] * sumYZ + invMtx[5] * sumZ;
        _result[2] = invMtx[6] * sumXZ + invMtx[7] * sumYZ + invMtx[8] * sumZ;
    }

    inline void rgb_to_hsv(float *_hsv,
                           const float *_rgb) {
        const float rr = _rgb[0];
        const float gg = _rgb[1];
        const float bb = _rgb[2];

        const float s0 = fstep(bb, gg);

        const float px = flerp(bb, gg, s0);
        const float py = flerp(gg, bb, s0);
        const float pz = flerp(-1.0f, 0.0f, s0);
        const float pw = flerp(2.0f / 3.0f, -1.0f / 3.0f, s0);

        const float s1 = fstep(px, rr);

        const float qx = flerp(px, rr, s1);
        const float qy = py;
        const float qz = flerp(pw, pz, s1);
        const float qw = flerp(rr, px, s1);

        const float dd = qx - fmin(qw, qy);
        const float ee = 1.0e-10f;

        _hsv[0] = fabsolute(qz + (qw - qy) / (6.0f * dd + ee));
        _hsv[1] = dd / (qx + ee);
        _hsv[2] = qx;
    }

    inline void hsv_to_rgb(float *_rgb,
                           const float *_hsv) {
        const float hh = _hsv[0];
        const float ss = _hsv[1];
        const float vv = _hsv[2];

        const float px = fabsolute(ffract(hh + 1.0f) * 6.0f - 3.0f);
        const float py = fabsolute(ffract(hh + 2.0f / 3.0f) * 6.0f - 3.0f);
        const float pz = fabsolute(ffract(hh + 1.0f / 3.0f) * 6.0f - 3.0f);

        _rgb[0] = vv * flerp(1.0f, fsaturate(px - 1.0f), ss);
        _rgb[1] = vv * flerp(1.0f, fsaturate(py - 1.0f), ss);
        _rgb[2] = vv * flerp(1.0f, fsaturate(pz - 1.0f), ss);
    }

}

#endif //CELIB_FPUMATH_INL
