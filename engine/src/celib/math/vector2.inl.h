#pragma once

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "celib/defines.h"
#include "celib/math/types.h"
#include "celib/math/math.inl.h"


namespace cetech {

    /***************************************************************************
    **** Vector2 constatns
    ***************************************************************************/
    namespace vector2 {

        /***********************************************************************
        **** Zero vector
        ***********************************************************************/
        static const Vector2 ZERO = {0.0f, 0.0f};

        /***********************************************************************
        **** Unit vector
        ***********************************************************************/
        static const Vector2 UNIT = {1.0f, 1.0f};

        /***********************************************************************
        **** Up vector
        ***********************************************************************/
        static const Vector2 UP = {0.0f, 1.0f};

        /***********************************************************************
        **** Down vector
        ***********************************************************************/
        static const Vector2 DOWN = {0.0f, -1.0f};

        /***********************************************************************
        **** Right vector
        ***********************************************************************/
        static const Vector2 RIGHT = {1.0f, 0.0f};

        /***********************************************************************
        **** Left vector
        ***********************************************************************/
        static const Vector2 LEFT = {-1.0f, 0.0f};
    }

    /***************************************************************************
    **** Vector2 interface
    ***************************************************************************/
    namespace vector2 {

        /***********************************************************************
        **** Make Vector2
        ***********************************************************************/
        CE_INLINE Vector2 make_vector2(const float x,
                                       const float y);

        /***********************************************************************
        **** Vector len
        ***********************************************************************/
        CE_INLINE float len(const Vector2& v1);

        /***********************************************************************
        **** Vector squared len.
        ***********************************************************************/
        CE_INLINE float len_sq(const Vector2& v1);

        /***********************************************************************
        **** Vector 1/len
        ***********************************************************************/
        CE_INLINE float len_inv(const Vector2& v1);

        /***********************************************************************
        **** Return normalized vector.
        ***********************************************************************/
        CE_INLINE Vector2 normalized(const Vector2& v1);

        /***********************************************************************
        **** Is vector normalized?
        ***********************************************************************/
        CE_INLINE bool is_normalized(const Vector2& v1);

        /***********************************************************************
        **** Dot product.
        ***********************************************************************/
        CE_INLINE float dot(const Vector2& v1,
                            const Vector2& v2);

        /***********************************************************************
        **** Cross product.
        ***********************************************************************/
        CE_INLINE float cross(const Vector2& v1,
                              const Vector2& v2);

        /***********************************************************************
        **** Squared distance between two point.
        ***********************************************************************/
        CE_INLINE float distance_sq(const Vector2& v1,
                                    const Vector2& v2);

        /***********************************************************************
        **** Distance between two point.
        ***********************************************************************/
        CE_INLINE float distance(const Vector2& v1,
                                 const Vector2& v2);

        /***********************************************************************
        **** Rotate vector by given angle.
        ***********************************************************************/
        CE_INLINE Vector2 rotated(const Vector2& v1,
                                  const float angle_deg);

        /***********************************************************************
        **** Get minimal value from vector element.
        ***********************************************************************/
        CE_INLINE float min(const Vector2& v1);

        /***********************************************************************
        **** Get maximum value from vector element.
        ***********************************************************************/
        CE_INLINE float max(const Vector2& v1);

        /***********************************************************************
        **** Get maximum absolute value from vector element.
        ***********************************************************************/
        CE_INLINE float abs_max(const Vector2& v1);

        /***********************************************************************
        **** Get sign vector.
        ***********************************************************************/
        CE_INLINE Vector2 sign_vector(const Vector2& v1);
    }

    /***************************************************************************
    **** Return negativ vector.
    ***************************************************************************/
    CE_INLINE Vector2 operator - (const Vector2 &v1);


    /***************************************************************************
    **** V1 + V2
    ***************************************************************************/
    CE_INLINE Vector2 operator + (const Vector2 &v1, const Vector2 &v2);

    /***************************************************************************
    **** V1 - V2
    ***************************************************************************/
    CE_INLINE Vector2 operator - (const Vector2 &v1, const Vector2 &v2);

    /***************************************************************************
    **** V1 * V2 [x1 * x2, y1 * y2]
    ***************************************************************************/
    CE_INLINE Vector2 operator* (const Vector2 &v1, const Vector2 &v2);

    /***************************************************************************
    **** V1 / V2 [x1 / x2, y1 / y2]
    ***************************************************************************/
    CE_INLINE Vector2 operator / (const Vector2 &v1, const Vector2 &v2);

    /***************************************************************************
    **** V1 * scalar
    ***************************************************************************/
    CE_INLINE Vector2 operator* (const Vector2 &v1, const float s);

    /***************************************************************************
    ****  V1 / scalar
    ***************************************************************************/
    CE_INLINE Vector2 operator / (const Vector2 &v1, const float s);


    /***************************************************************************
    **** Vector2 implementation
    ***************************************************************************/
    namespace vector2 {
        Vector2 make_vector2(const float x,
                             const float y) {
            return {
                       x, y
            };
        }

        float len(const Vector2& v1) {
            return math::fast_sqrt(len_sq(v1));
        }

        float len_sq(const Vector2& v1) {
            return dot(v1, v1);
        }

        float len_inv(const Vector2& v1) {
            return math::fast_inv_sqrt(len_sq(v1));
        }

        Vector2 normalized(const Vector2& v1) {
            return v1 * len_inv(v1);
        }

        bool is_normalized(const Vector2& v1) {
            return math::abs(1.0f - len_sq(v1)) < 0.001f;
        }

        float dot(const Vector2& v1,
                  const Vector2& v2) {
            return (v1.x * v2.x) + (v1.y * v2.y);
        }

        float cross(const Vector2& v1,
                    const Vector2& v2) {
            return (v1.x * v2.y) - (v1.y * v2.x);
        }

        float distance_sq(const Vector2& v1,
                          const Vector2& v2) {
            return math::square(v2.x - v1.x) + math::square(v2.y - v1.y);
        }

        float distance(const Vector2& v1,
                       const Vector2& v2) {
            return math::fast_sqrt(distance_sq(v1, v2));
        }

        Vector2 rotated(const Vector2& v1,
                        const float angle_deg) {
            float sin, cos;
            math::fast_sincos(angle_deg, sin, cos);

            return make_vector2(
                (cos * v1.x) - (sin * v1.y),
                (sin * v1.x) + (cos * v1.y)
                );
        }

        float min(const Vector2& v1) {
            return math::min(v1.x, v1.y);
        }

        float max(const Vector2& v1) {
            return math::max(v1.x, v1.y);
        }

        float abs_max(const Vector2& v1) {
            return math::max(math::abs(v1.x), math::abs(v1.y));
        }

        Vector2 sign_vector(const Vector2& v1) {
            return make_vector2(
                math::float_select(v1.x, 1.0f, -1.0f),
                math::float_select(v1.y, 1.0f, -1.0f)
                );
        }
    }

    Vector2 operator - (const Vector2 &v1) {
        return vector2::make_vector2(
            -v1.x,
            -v1.y
            );
    }

    Vector2 operator + (const Vector2 &v1, const Vector2 &v2) {
        return vector2::make_vector2(
            v1.x + v2.x,
            v1.y + v2.y
            );
    }

    Vector2 operator - (const Vector2 &v1, const Vector2 &v2) {
        return vector2::make_vector2(
            v1.x - v2.x,
            v1.y - v2.y
            );
    }

    Vector2 operator* (const Vector2 &v1, const Vector2 &v2) {
        return vector2::make_vector2(
            v1.x * v2.x,
            v1.y * v2.y
            );
    }

    Vector2 operator / (const Vector2 &v1, const Vector2 &v2) {
        return vector2::make_vector2(
            v1.x / v2.x,
            v1.y / v2.y
            );
    }

    Vector2 operator* (const Vector2 &v1, const float s) {
        return vector2::make_vector2(
            v1.x * s,
            v1.y * s
            );
    }

    Vector2 operator / (const Vector2 &v1, const float s) {
        const float inv_s = 1 / s;

        return vector2::make_vector2(
            v1.x * inv_s,
            v1.y * inv_s
            );
    }
}
