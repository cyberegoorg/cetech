#pragma once

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "celib/math_types.h"
#include "celib/math/math.inl.h"

namespace cetech {

    /***************************************************************************
    **** Vector3 constatns
    ***************************************************************************/
    namespace vector3 {

        /***********************************************************************
        **** Zero vector
        ***********************************************************************/
        static const Vector3 ZERO = {0.0f, 0.0f, 0.0f};

        /***********************************************************************
        **** Unit vector
        ***********************************************************************/
        static const Vector3 UNIT = {1.0f, 1.0f, 1.0f};

        /***********************************************************************
        **** Up vector
        ***********************************************************************/
        static const Vector3 UP = {0.0f, 1.0f, 0.0f};

        /***********************************************************************
        **** Down vector
        ***********************************************************************/
        static const Vector3 DOWN = {0.0f, -1.0f, 0.0f};

        /***********************************************************************
        **** Forward vector
        ***********************************************************************/
        static const Vector3 FORWARDS = {0.0f, 0.0f, 1.0f};

        /***********************************************************************
        **** Backwards vector
        ***********************************************************************/
        static const Vector3 BACKWARDS = {0.0f, 0.0f, -1.0f};

        /***********************************************************************
        **** Right vector
        ***********************************************************************/
        static const Vector3 RIGHT = {1.0f, 0.0f, 0.0f};

        /***********************************************************************
        **** Left vector
        ***********************************************************************/
        static const Vector3 LEFT = {-1.0f, 0.0f, 0.0f};
    }

    /***************************************************************************
    **** Vector3 interface
    ***************************************************************************/
    namespace vector3 {

        /***********************************************************************
        **** Make Vector3
        ***********************************************************************/
        CE_INLINE Vector3 make_vector3(const float x,
                                       const float y,
                                       const float z);

        /***********************************************************************
        **** Vector len
        ***********************************************************************/
        CE_INLINE float len(const Vector3& v1);

        /***********************************************************************
        **** Vector squared len.
        ***********************************************************************/
        CE_INLINE float len_sq(const Vector3& v1);

        /***********************************************************************
        **** Vector 1/len
        ***********************************************************************/
        CE_INLINE float len_inv(const Vector3& v1);

        /***********************************************************************
        **** Return normalized vector.
        ***********************************************************************/
        CE_INLINE Vector3 normalized(const Vector3& v1);

        /***********************************************************************
        **** Is vector normalized?
        ***********************************************************************/
        CE_INLINE bool is_normalized(const Vector3& v1);

        /***********************************************************************
        **** Dot product.
        ***********************************************************************/
        CE_INLINE float dot(const Vector3& v1,
                            const Vector3& v2);

        /***********************************************************************
        **** Cross product.
        ***********************************************************************/
        CE_INLINE Vector3 cross(const Vector3& v1,
                                const Vector3& v2);

        /***********************************************************************
        **** Squared distance between two point.
        ***********************************************************************/
        CE_INLINE float distance_sq(const Vector3& v1,
                                    const Vector3& v2);

        /***********************************************************************
        **** Distance between two point.
        ***********************************************************************/
        CE_INLINE float distance(const Vector3& v1,
                                 const Vector3& v2);


        /***********************************************************************
        **** Rotate vector by given angle.
        ***********************************************************************/
        CE_INLINE Vector3 rotated(const Vector3& v1,
                                  const float angle_deg);

        /***********************************************************************
        **** Get minimal value from vector element.
        ***********************************************************************/
        CE_INLINE float min(const Vector3& v1);

        /***********************************************************************
        **** Get maximum value from vector element.
        ***********************************************************************/
        CE_INLINE float max(const Vector3& v1);

        /***********************************************************************
        **** Get maximum absolute value from vector element.
        ***********************************************************************/
        CE_INLINE float abs_max(const Vector3& v1);

        /***********************************************************************
        **** Get sign vector.
        ***********************************************************************/
        CE_INLINE Vector3 sign_vector(const Vector3& v1);
    }

    /***************************************************************************
    **** Return negativ vector.
    ***************************************************************************/
    CE_INLINE Vector3 operator - (const Vector3 &v1);


    /***************************************************************************
    **** V1 + V2
    ***************************************************************************/
    CE_INLINE Vector3 operator + (const Vector3 &v1, const Vector3 &v2);

    /***************************************************************************
    **** V1 - V2
    ***************************************************************************/
    CE_INLINE Vector3 operator - (const Vector3 &v1, const Vector3 &v2);

    /***************************************************************************
    **** V1 * V2 [x1 * x2, y1 * y2, z1 * z2]
    ***************************************************************************/
    CE_INLINE Vector3 operator* (const Vector3 &v1, const Vector3 &v2);

    /***************************************************************************
    **** V1 / V2 [x1 / x2, y1 / y2, z1 / z2]
    ***************************************************************************/
    CE_INLINE Vector3 operator / (const Vector3 &v1, const Vector3 &v2);

    /***************************************************************************
    **** V1 * scalar
    ***************************************************************************/
    CE_INLINE Vector3 operator* (const Vector3 &v1, const float s);

    /***************************************************************************
    **** V1 / scalar
    ***************************************************************************/
    CE_INLINE Vector3 operator / (const Vector3 &v1, const float s);

    /***************************************************************************
    **** Vector3 implementation
    ***************************************************************************/
    namespace vector3 {
        Vector3 make_vector3(const float x,
                             const float y,
                             const float z) {
            return {
                       x, y, z
            };
        }

        float len(const Vector3& v1) {
            return math::fast_sqrt(len_sq(v1));
        }

        float len_sq(const Vector3& v1) {
            return dot(v1, v1);
        }

        float len_inv(const Vector3& v1) {
            return math::fast_inv_sqrt(len_sq(v1));
        }

        Vector3 normalized(const Vector3& v1) {
            return v1 * len_inv(v1);
        }

        bool is_normalized(const Vector3& v1) {
            return math::abs(1.0f - len_sq(v1)) < 0.001f;
        }

        float dot(const Vector3& v1,
                  const Vector3& v2) {
            return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
        }

        Vector3 cross(const Vector3& v1,
                      const Vector3& v2) {
            return make_vector3(
                (v1.y * v2.z) - (v1.z * v2.y),
                (v1.z * v2.x) - (v1.x * v2.z),
                (v1.x * v2.y) - (v1.y * v2.x)
                );
        }

        float distance_sq(const Vector3& v1,
                          const Vector3& v2) {
            return math::square(v2.x - v1.x) + math::square(v2.y - v1.y) + math::square(v2.z - v1.z);
        }

        float distance(const Vector3& v1,
                       const Vector3& v2) {
            return math::fast_sqrt(distance_sq(v1, v2));
        }

        float min(const Vector3& v1) {
            return math::min(math::min(v1.x, v1.y), v1.z);
        }

        float max(const Vector3& v1) {
            return math::max(math::max(v1.x, v1.y), v1.z);
        }

        float abs_max(const Vector3& v1) {
            return math::max(math::max(math::abs(v1.x), math::abs(v1.y)), math::abs(v1.z));
        }

        Vector3 sign_vector(const Vector3& v1) {
            return make_vector3(
                math::float_select(v1.x, 1.0f, -1.0f),
                math::float_select(v1.y, 1.0f, -1.0f),
                math::float_select(v1.z, 1.0f, -1.0f)
                );
        }
    }

    Vector3 operator - (const Vector3 &v1) {
        return vector3::make_vector3(
            -v1.x,
            -v1.y,
            -v1.z
            );
    }

    Vector3 operator + (const Vector3 &v1, const Vector3 &v2) {
        return vector3::make_vector3(
            v1.x + v2.x,
            v1.y + v2.y,
            v1.z + v2.z
            );
    }

    Vector3 operator - (const Vector3 &v1, const Vector3 &v2) {
        return vector3::make_vector3(
            v1.x - v2.x,
            v1.y - v2.y,
            v1.z - v2.z
            );
    }

    Vector3 operator* (const Vector3 &v1, const Vector3 &v2) {
        return vector3::make_vector3(
            v1.x * v2.x,
            v1.y * v2.y,
            v1.z * v2.z
            );
    }

    Vector3 operator / (const Vector3 &v1, const Vector3 &v2) {
        return vector3::make_vector3(
            v1.x / v2.x,
            v1.y / v2.y,
            v1.z / v2.z
            );
    }

    Vector3 operator* (const Vector3 &v1, const float s) {
        return vector3::make_vector3(
            v1.x * s,
            v1.y * s,
            v1.z * s
            );
    }

    Vector3 operator / (const Vector3 &v1, const float s) {
        const float inv_s = 1 / s;

        return vector3::make_vector3(
            v1.x * inv_s,
            v1.y * inv_s,
            v1.z * inv_s
            );
    }
}
