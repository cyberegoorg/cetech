#pragma once

#include "math_types.h"
#include "math.inl.h"
#include "simd/simd.h"
#include "vector3.inl.h"

namespace cetech {
    namespace matrix33 {
        /*! Identity matrix.
         */
        static const Matrix33 IDENTITY = {
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f}
        };
    }

    namespace matrix33 {
        CE_INLINE Matrix33 make_matrix33(const Vector3& x,
                                         const Vector3& y,
                                         const Vector3& z);

        /*! Make matrix33 from axis and angle.
         * \param axis Axis
         * \param angle_deg Angle in deg
         * \return New rotation matrix
         */
        CE_INLINE Matrix33 from_axis_angle(const Vector3& axis,
                                           const float angle_deg);

    }

    CE_INLINE Matrix33 operator* (const Matrix33 &m1, const Matrix33 &m2);
    CE_INLINE Vector3 operator* (const Matrix33 &m1, const Vector3 &v1);


    namespace matrix33 {
        CE_INLINE Matrix33 make_matrix33(const Vector3& x,
                                         const Vector3& y,
                                         const Vector3& z) {
            Matrix33 res = {x, y, z};
            return res;
        }

        Matrix33 from_axis_angle ( const Vector3& axis,
                                   const float angle_deg ) {
            float sin, cos, C;

            sin = cos = 0.0f;
            C = 1.0f - cos;

            math::fast_sincos(angle_deg, sin, cos);

            return make_matrix33(
                {(axis.x * axis.x * C) + cos, (axis.x * axis.y * C) - (axis.z * sin),
                 (axis.x * axis.z * C) + (axis.y * sin)},
                {(axis.y * axis.x * C) + (axis.z * sin), (axis.y * axis.y * C) + cos,
                 (axis.y * axis.z * C) - (axis.x * sin)},
                {(axis.z * axis.x * C) - (axis.y * sin), (axis.z * axis.y * C) + (axis.x * sin), (axis.z * axis.z * C) + cos}
                );
        }
    }

    Matrix33 operator* (const Matrix33 &m1, const Matrix33 &m2) {
        Matrix33 res;
        simd::mat33_mult(&m1, &m2, &res);
        return res;
    }

    Vector3 operator* (const Matrix33 &m1, const Vector3 &v1) {
        return vector3::make_vector3(
            vector3::dot(m1.x, v1),
            vector3::dot(m1.y, v1),
            vector3::dot(m1.z, v1)
            );
    }

}
