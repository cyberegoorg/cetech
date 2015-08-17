#pragma once

#include "simd_types.h"

#if defined(CETECH_SIMD_FPU)
  #include "fpu.h"
#elif defined(CETECH_SIMD_SSE)
  #include "sse.h"
#endif

namespace cetech1 {

    /*! SIMD procesor.
     */
    namespace simd {
        /*! Crate new simd vector.
         * \param x X.
         * \param y Y.
         * \param z Z.
         * \param w W.
         */
        CE_INLINE SimdVector make_simd_vector(const float x, const float y, const float z, const float w);

        /*! Get elment from vector.
         * \param v1 Vector.
         * \param idx Element index 0, 1, 2, 3
         */
        CE_INLINE float get_element(const SimdVector& v1, const uint32_t idx);

        /*! V1 + V2
         * \param v1 Vector 1.
         * \param v2 Vector 2.
         * \return Final vector.
         */
        CE_INLINE SimdVector add(const SimdVector& v1, const SimdVector& v2);

        /*! V1 - V2
         * \param v1 Vector 1.
         * \param v2 Vector 2.
         * \return Final vector.
         */
        CE_INLINE SimdVector sub(const SimdVector& v1, const SimdVector& v2);

        /*! V1 * V2
         * \param v1 Vector 1.
         * \param v2 Vector 2.
         * \return Final vector.
         */
        CE_INLINE SimdVector mul(const SimdVector& v1, const SimdVector& v2);

        /*! V1 / V2
         * \param v1 Vector 1.
         * \param v2 Vector 2.
         * \return Final vector.
         */
        CE_INLINE SimdVector div(const SimdVector& v1, const SimdVector& v2);

        /*! -V1
         * \param v1 Vector 1.
         * \return -V1.
         */
        CE_INLINE SimdVector negate(const SimdVector& v1);

        /*! abs(V1)
         * \param v1 Vector 1.
         * \return abs(v1)
         */
        CE_INLINE SimdVector abs(const SimdVector& v1);

        /*! Create vector from float array.
         * \param src source.
         * \return Vector
         */
        CE_INLINE SimdVector load(const void* src);

        /*! Create vector from float array. W == 0
         * \param src source.
         * \return Vector
         */
        CE_INLINE SimdVector load3(const void* src);

        /*! Create vector from float array. W == 0
         * \param src source.
         * \return Vector
         */
        CE_INLINE SimdVector load3_w0(const void* src);

        /*! Create vector from float array. W == 1
         * \param src source.
         * \return Vector
         */
        CE_INLINE SimdVector load3_w1(const void* src);

        /*! Store vector to float 4 array.
         * \param v1 Vector.
         * \param dst Destination.
         */
        CE_INLINE void store(const SimdVector& v1, void* dst);

        /*! Store vector (x, y, z) to float array.
         * \param v1 Vector.
         * \param dst Destination.
         */
        CE_INLINE void store3(const SimdVector& v1, void* dst);

        /*! Store vector (x) to float array.
         * \param v1 Vector.
         * \param dst Destination.
         */
        CE_INLINE void store1(const SimdVector& v1, void* dst);

        /*! Quaternion multiplication.
         * \param q1 Quaternion data.
         * \param q2 Quaternion data.
         * \param dst Destination.
         */
        CE_INLINE void quat_mult(const void* q1, const void* q2, void* dst);
    }

    namespace simd {
        /*! Zero vector
         */
        static const SimdVector ZERO = make_simd_vector(0.0f, 0.0f, 0.0f, 0.0f);

        /*! One vector
         */
        static const SimdVector ONE = make_simd_vector(1.0f, 1.0f, 1.0f, 1.0f);

        /*! Minus one vector
         */
        static const SimdVector MINUS_ONE = make_simd_vector(-1.0f, -1.0f, -1.0f, -1.0f);
    }
}
