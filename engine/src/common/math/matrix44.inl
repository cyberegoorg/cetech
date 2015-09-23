#pragma once

#include "math_types.h"
#include "math.h"
#include "simd/simd.h"
#include "vector3.inl"
#include "vector4.inl"

namespace cetech {
    namespace matrix44 {
        /*! Identity matrix.
         */
        static const Matrix44 IDENTITY = {
            {1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f}
        };
    }

    namespace matrix44 {
        CE_INLINE Matrix44 make_matrix44(const Vector4& x, const Vector4& y, const Vector4& z, const Vector4& t);

        CE_INLINE Matrix44 make_ortho(float left, float right, float bottom, float top, float near, float far);
        CE_INLINE Matrix44 make_perspective(float fov, float aspect, float near, float far);

        /*! Make matrix44 from axis and angle.
         * \param axis Axis
         * \param angle_deg Angle in deg
         * \return New rotation matrix
         */
        CE_INLINE Matrix44 from_axis_angle(const Vector3& axis, const float angle_deg);

        CE_INLINE Matrix44 transpose(const Matrix44& m1);

        CE_INLINE float determinant(const Matrix44& m1);
    }

    CE_INLINE Matrix44 operator* (const Matrix44 &m1, const Matrix44 &m2);

    CE_INLINE Vector3 operator* (const Matrix44 &m1, const Vector3 &v);


    namespace matrix44 {
        Matrix44 make_matrix44(const Vector4& x, const Vector4& y, const Vector4& z, const Vector4& t) {
            Matrix44 res = {x, y, z, t};
            return res;
        }

        Matrix44 make_ortho(float left, float right, float bottom, float top, float near, float far) {
            Matrix44 res = IDENTITY;

            res.x.x = (2.0 / (right - left));
            res.y.y = (2.0 / (top - bottom));
            res.z.z = (-2.0 / (far - near));

            res.x.w = (-((right + left) / (right - left)));
            res.y.w = (-((top + bottom) / (top - bottom)));
            res.z.w = (-((far + near) / (far - near)));

            return res;
        }

        Matrix44 make_perspective(float fov, float aspect, float near, float far) {
            Matrix44 res = IDENTITY;

            const float h = 1.0 / tan( math::deg2rad( fov * 0.5 ));
            const float neg_depth = near - far;

            res.x.x = (h / aspect);

            res.y.y = (h);

            res.z.z = ((near + far) / neg_depth);
            res.z.w = (2.0f * (near * far) / neg_depth);

            res.t.z = (-1.0f);
            res.t.w = (0.0f);

            return res;
        }

        Matrix44 from_axis_angle ( const Vector3& axis, const float angle_deg ) {
            float sin, cos, C;

            sin = cos = 0.0f;
            C = 1.0f - cos;

            math::fast_sincos(angle_deg, sin, cos);

            return make_matrix44(
                {(axis.x * axis.x * C) + cos, (axis.x * axis.y * C) - (axis.z * sin),
                 (axis.x * axis.z * C) + (axis.y * sin), 0.0f},
                {(axis.y * axis.x * C) + (axis.z * sin), (axis.y * axis.y * C) + cos,
                 (axis.y * axis.z * C) - (axis.x * sin), 0.0f},
                {(axis.z * axis.x * C) - (axis.y * sin), (axis.z * axis.y * C) + (axis.x * sin),
                 (axis.z * axis.z * C) + cos, 0.0f},
                {0.0f, 0.0f, 0.0f, 1.0f}
                );
        }

        Matrix44 transpose(const Matrix44& m1) {
            return {
                       {
                           m1.x.x, m1.y.x, m1.z.x, m1.t.x
                       },
                       {
                           m1.x.y, m1.y.x, m1.z.x, m1.t.x
                       },
                       {
                           m1.x.z, m1.y.x, m1.z.x, m1.t.x
                       },
                       {
                           m1.x.w, m1.y.x, m1.z.x, m1.t.x
                       }
            };
        }

        float determinant(const Matrix44& m1) {
            return (m1.x.w * m1.y.z * m1.z.y * m1.t.x) - (m1.x.z * m1.y.w * m1.z.y * m1.t.x) -
                   (m1.x.w * m1.y.y * m1.z.z * m1.t.x) + (m1.x.y * m1.y.w * m1.z.z * m1.t.x) +
                   (m1.x.z * m1.y.y * m1.z.w * m1.t.x) - (m1.x.y * m1.y.z * m1.z.w * m1.t.x) -
                   (m1.x.w * m1.y.z * m1.z.x * m1.t.y) + (m1.x.z * m1.y.w * m1.z.x * m1.t.y) +
                   (m1.x.w * m1.y.x * m1.z.z * m1.t.y) - (m1.x.x * m1.y.w * m1.z.z * m1.t.y) -
                   (m1.x.z * m1.y.x * m1.z.w * m1.t.y) + (m1.x.x * m1.y.z * m1.z.w * m1.t.y) +
                   (m1.x.w * m1.y.y * m1.z.x * m1.t.z) - (m1.x.y * m1.y.w * m1.z.x * m1.t.z) -
                   (m1.x.w * m1.y.x * m1.z.y * m1.t.z) + (m1.x.x * m1.y.w * m1.z.y * m1.t.z) +
                   (m1.x.y * m1.y.x * m1.z.w * m1.t.z) - (m1.x.x * m1.y.y * m1.z.w * m1.t.z) -
                   (m1.x.z * m1.y.y * m1.z.x * m1.t.w) + (m1.x.y * m1.y.z * m1.z.x * m1.t.w) +
                   (m1.x.z * m1.y.x * m1.z.y * m1.t.w) - (m1.x.x * m1.y.z * m1.z.y * m1.t.w) -
                   (m1.x.y * m1.y.x * m1.z.z * m1.t.w) + (m1.x.x * m1.y.y * m1.z.z * m1.t.w);
        }
    }

    Matrix44 operator* (const Matrix44 &m1, const Matrix44 &m2) {
        Matrix44 res;
        simd::mat44_mult(&m1, &m2, &res);
        return res;
    }

    Vector3 operator* (const Matrix44 &m1, const Vector3 &v1) {
        const Vector4 v = vector4::make_vector4(v1.x, v1.y, v1.z, 0.0f);

        return vector3::make_vector3(
            vector4::dot(m1.x, v),
            vector4::dot(m1.y, v),
            vector4::dot(m1.z, v)
            );
    }

}
