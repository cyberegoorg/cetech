#pragma once

#include "common/debug/asserts.h"
#include "common/math/math.h"
#include "common/memory/memory.h"

#include "fpu_types.h"

namespace cetech {
    namespace simd {
        CE_INLINE SimdVector make_simd_vector(const float x, const float y, const float z, const float w) {
            SimdVector ret = {{x, y, z, w}};
            return ret;
        }

        CE_INLINE float get_element(const SimdVector& v1, const uint32_t idx) {
            return v1.v[idx];
        }

        CE_INLINE SimdVector add(const SimdVector& v1, const SimdVector& v2) {
            SimdVector ret;

            ret.v[0] = v1.v[0] + v2.v[0];
            ret.v[1] = v1.v[1] + v2.v[1];
            ret.v[2] = v1.v[2] + v2.v[2];
            ret.v[3] = v1.v[3] + v2.v[3];

            return ret;
        }

        CE_INLINE SimdVector sub(const SimdVector& v1, const SimdVector& v2) {
            SimdVector ret;

            ret.v[0] = v1.v[0] - v2.v[0];
            ret.v[1] = v1.v[1] - v2.v[1];
            ret.v[2] = v1.v[2] - v2.v[2];
            ret.v[3] = v1.v[3] - v2.v[3];

            return ret;
        }

        CE_INLINE SimdVector mul(const SimdVector& v1, const SimdVector& v2) {
            SimdVector ret;

            ret.v[0] = v1.v[0] * v2.v[0];
            ret.v[1] = v1.v[1] * v2.v[1];
            ret.v[2] = v1.v[2] * v2.v[2];
            ret.v[3] = v1.v[3] * v2.v[3];

            return ret;
        }

        CE_INLINE SimdVector div(const SimdVector& v1, const SimdVector& v2) {
            SimdVector ret;

            ret.v[0] = v1.v[0] / v2.v[0];
            ret.v[1] = v1.v[1] / v2.v[1];
            ret.v[2] = v1.v[2] / v2.v[2];
            ret.v[3] = v1.v[3] / v2.v[3];

            return ret;
        }

        CE_INLINE SimdVector negate(const SimdVector& v1) {
            return make_simd_vector(
                -v1.v[0],
                -v1.v[1],
                -v1.v[2],
                -v1.v[3]
                );
        }

        CE_INLINE SimdVector abs(const SimdVector& v1) {
            return make_simd_vector(
                math::abs(v1.v[0]),
                math::abs(v1.v[1]),
                math::abs(v1.v[2]),
                math::abs(v1.v[3])
                );
        }

        CE_INLINE SimdVector load(const void* src) {
            CE_CHECK_PTR(src);

            return make_simd_vector(
                ((const float*) src)[0],
                ((const float*) src)[1],
                ((const float*) src)[2],
                ((const float*) src)[3]
                );
        }

        CE_INLINE SimdVector load3(const void* src) {
            CE_CHECK_PTR(src);

            return make_simd_vector(
                ((const float*) src)[0],
                ((const float*) src)[1],
                ((const float*) src)[2],
                0.0f
                );
        }

        CE_INLINE SimdVector load3_w0(const void* src) {
            CE_CHECK_PTR(src);

            return make_simd_vector(
                ((const float*) src)[0],
                ((const float*) src)[1],
                ((const float*) src)[2],
                0.0f
                );
        }

        CE_INLINE SimdVector load3_w1(const void* src) {
            CE_CHECK_PTR(src);

            return make_simd_vector(
                ((const float*) src)[0],
                ((const float*) src)[1],
                ((const float*) src)[2],
                1.0f
                );
        }

        CE_INLINE void store(const SimdVector& v1, void* dst) {
            CE_CHECK_PTR(dst);

            memory::memcpy(dst, v1.v, 16);
        }

        CE_INLINE void store3(const SimdVector& v1, void* dst) {
            CE_CHECK_PTR(dst);

            memory::memcpy(dst, v1.v, 12);
        }

        CE_INLINE void store1(const SimdVector& v1, void* dst) {
            CE_CHECK_PTR(dst);

            memory::memcpy(dst, v1.v, 4);
        }

        CE_INLINE void quat_mult(const void* q1, const void* q2, void* dst) {
            CE_CHECK_PTR(q1);
            CE_CHECK_PTR(q2);
            CE_CHECK_PTR(dst);

            const float* q1f = (const float*)q1;
            const float* q2f = (const float*)q2;
            float* dstf = (float*)dst;

            dstf[0] = q1f[0] * q2f[0] - q1f[1] * q2f[1] - q1f[2] * q2f[2] - q1f[3] * q2f[3];
            dstf[1] = q1f[0] * q2f[1] + q1f[1] * q2f[0] + q1f[2] * q2f[3] - q1f[3] * q2f[2];
            dstf[2] = q1f[0] * q2f[2] - q1f[1] * q2f[3] + q1f[2] * q2f[0] + q1f[3] * q2f[1];
            dstf[3] = q1f[0] * q2f[3] + q1f[1] * q2f[2] - q1f[2] * q2f[1] + q1f[3] * q2f[0];
        }

        #define mi(i, j, s) (((i) * (s)) + (j))
        #define mi3(i, j) mi(i, j, 3)
        #define mi4(i, j) mi(i, j, 4)

        CE_INLINE void mat33_mult(const void* m1, const void* m2, void* dst) {
            CE_CHECK_PTR(m1);
            CE_CHECK_PTR(m2);
            CE_CHECK_PTR(dst);

            const float* m1f = (const float*)m1;
            const float* m2f = (const float*)m2;
            float* dstf = (float*)dst;

            float sum;
            for (int32_t i = 0; i < 3; ++i) {
                for (int32_t j = 0; j < 3; ++j) {
                    sum = 0;

                    for (int32_t k = 0; k < 3; ++k) {
                        sum += m1f[mi3(i, k)] * m2f[mi3(k, j)];
                    }

                    dstf[mi3(i, j)] = sum;
                }
            }
        }

        CE_INLINE void mat44_mult(const void* m1, const void* m2, void* dst) {
            CE_CHECK_PTR(m1);
            CE_CHECK_PTR(m2);
            CE_CHECK_PTR(dst);

            const float* m1f = (const float*)m1;
            const float* m2f = (const float*)m2;
            float* dstf = (float*)dst;

            float sum;
            for (int32_t i = 0; i < 4; ++i) {
                for (int32_t j = 0; j < 4; ++j) {
                    sum = 0;

                    for (int32_t k = 0; k < 4; ++k) {
                        sum += m1f[mi3(i, k)] * m2f[mi3(k, j)];
                    }

                    dstf[mi3(i, j)] = sum;
                }
            }
        }

        #undef mi
        #undef mi3
        #undef mi4
    }
};