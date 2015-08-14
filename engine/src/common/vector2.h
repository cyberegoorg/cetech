#pragma once

#include "math_types.h"
#include "math.h"

namespace cetech1 {
    namespace vector2 {
        /*! Zero vector [0.0f, 0.0f]
         */
        static const Vector2 ZERO = {0.0f, 0.0f};

        /*! Unit vector [1.0f, 1.0f]
         */
        static const Vector2 UNIT = {1.0f, 1.0f};


        /*! Up vector [0.0f, 1.0f]
         */
        static const Vector2 UP = {0.0f, 1.0f};

        /*! Down vector [0.0f, -1.0f]
         */
        static const Vector2 DOWN = {0.0f, -1.0f};

        /*! Right vector [1.0f, 0.0f]
         */
        static const Vector2 RIGHT = {1.0f, 0.0f};

        /*! Left vector [-1.0f, 0.0f]
         */
        static const Vector2 LEFT = {-1.0f, 0.0f};
    }

    namespace vector2 {
        /*! Make Vector2
         * \param x X
         * \param y Y
         * \return New vector
         */
        FORCE_INLINE Vector2 make_vector2(float x, float y);


        /*! Vector len
         * \param v1 Vector.
         * \return Vector len.
         */
        FORCE_INLINE float len(const Vector2& v1);

        /*! Vector squared len.
         * \param v1 Vector.
         * \return Vector squared len.
         */
        FORCE_INLINE float len_sq(const Vector2& v1);

        /*! Vector 1/len
         * \param v1 Vector.
         * \return Vector 1/len.
         */
        FORCE_INLINE float len_inv(const Vector2& v1);


        /*! Return normalized vector.
         * \param v1 Vector.
         * \return Normalized vector.
         */
        FORCE_INLINE Vector2 normalized(const Vector2& v1);


        /*! Dot product.
         * \param v1 Vector 1.
         * \param v2 Vector 2.
         * \return Dot product.
         */
        FORCE_INLINE float dot(const Vector2& v1, const Vector2& v2);

        /*! Cross product.
         * \param v1 Vector 1.
         * \param v2 Vector 2.
         * \return Cross product.
         */
        FORCE_INLINE float cross(const Vector2& v1, const Vector2& v2);

        /*! Squared distance between two point.
         * \param v1 Point 1.
         * \param v2 Point 2.
         * \return Squared distace.
         */
        FORCE_INLINE float distance_sq(const Vector2& v1, const Vector2& v2);

        /*! Distance between two point.
         * \param v1 Point 1.
         * \param v2 Point 2.
         * \return Distace.
         */
        FORCE_INLINE float distance(const Vector2& v1, const Vector2& v2);


        /*! Rotate vector by given angle.
         * \param v1 Vector.
         * \param angle_deg Angle in deg.
         * \return Rotated vector.
         */
        FORCE_INLINE Vector2 rotated(const Vector2& v1, const float angle_deg);


        /*! Get minimal value from vector element.
         * \param v1 Vector.
         * \return Minimal value from vector element.
         */
        FORCE_INLINE float min(const Vector2& v1);

        /*! Get maximum value from vector element.
         * \param v1 Vector.
         * \return Maximum value from vector element.
         */
        FORCE_INLINE float max(const Vector2& v1);

        /*! Get maximum absolute value from vector element.
         * \param v1 Vector.
         * \return Absolute maximum value from vector element.
         */
        FORCE_INLINE float abs_max(const Vector2& v1);

        /*! Get sign vector.
         * if element is positive return 1.0f else -1.0f.
         * \param v1 Vector.
         * \return Sign vector.
         */
        FORCE_INLINE Vector2 sign_vector(const Vector2& v1);
    }

    /*! Return negativ vector.
     * \param v1 Vector.
     * \return -v1
     */
    FORCE_INLINE Vector2 operator - (const Vector2 &v1);


    /*! V1 + V2
     * \param v1 Vector1.
     * \param v2 Vector2.
     * \return V1 + V2.
     */
    FORCE_INLINE Vector2 operator + (const Vector2 &v1, const Vector2 &v2);

    /*! V1 - V2
     * \param v1 Vector1.
     * \param v2 Vector2.
     * \return V1 - V2.
     */
    FORCE_INLINE Vector2 operator - (const Vector2 &v1, const Vector2 &v2);

    /*! V1 * V2 [x1 * x2, y1 * y2]
     * \param v1 Vector1.
     * \param v2 Vector2.
     * \return [x1*x2, y1 * y2]
     */
    FORCE_INLINE Vector2 operator* (const Vector2 &v1, const Vector2 &v2);

    /*! V1 / V2 [x1 / x2, y1 / y2]
     * \param v1 Vector1.
     * \param v2 Vector2.
     * \return [x1 / x2, y1 / y2]
     */
    FORCE_INLINE Vector2 operator / (const Vector2 &v1, const Vector2 &v2);


    /*! V1 * scalar
     * \param v1 Vector1.
     * \param s ScalarVector2.
     * \return V1 * scalar
     */
    FORCE_INLINE Vector2 operator* (const Vector2 &v1, const float s);

    /*! V1 / scalar
     * \param v1 Vector1.
     * \param s ScalarVector2.
     * \return V1 / scalar
     */
    FORCE_INLINE Vector2 operator / (const Vector2 &v1, const float s);



    namespace vector2 {
        FORCE_INLINE Vector2 make_vector2(float x, float y) {
            return {
                       x, y
            };
        }

        FORCE_INLINE float len(const Vector2& v1) {
            return math::fast_sqrt(len_sq(v1));
        }

        FORCE_INLINE float len_sq(const Vector2& v1) {
            return dot(v1, v1);
        }

        FORCE_INLINE float len_inv(const Vector2& v1) {
            return math::inv_sqrt(len_sq(v1));
        }

        FORCE_INLINE Vector2 normalized(const Vector2& v1) {
            return v1 * len_inv(v1);
        }

        FORCE_INLINE float dot(const Vector2& v1, const Vector2& v2) {
            return (v1.x * v2.x) + (v1.y * v2.y);
        }

        FORCE_INLINE float cross(const Vector2& v1, const Vector2& v2) {
            return (v1.x * v2.y) - (v1.y * v2.x);
        }

        FORCE_INLINE float distance_sq(const Vector2& v1, const Vector2& v2) {
            return math::square(v2.x - v1.x) + math::square(v2.y - v1.y);
        }

        FORCE_INLINE float distance(const Vector2& v1, const Vector2& v2) {
            return math::fast_sqrt(distance_sq(v1, v2));
        }

        FORCE_INLINE Vector2 rotated(const Vector2& v1, const float angle_deg) {
            float sin, cos;
            math::fast_sincos(angle_deg, sin, cos);

            return make_vector2(
                (cos * v1.x) - (sin * v1.y),
                (sin * v1.x) + (cos * v1.y)
                );
        }

        FORCE_INLINE float min(const Vector2& v1) {
            return math::min(v1.x, v1.y);
        }

        FORCE_INLINE float max(const Vector2& v1) {
            return math::max(v1.x, v1.y);
        }

        FORCE_INLINE float abs_max(const Vector2& v1) {
            return math::max(math::abs(v1.x), math::abs(v1.y));
        }

        FORCE_INLINE Vector2 sign_vector(const Vector2& v1) {
            return make_vector2(
                math::float_select(v1.x, 1.0f, -1.0f),
                math::float_select(v1.y, 1.0f, -1.0f)
                );
        }
    }

    FORCE_INLINE Vector2 operator - (const Vector2 &v1) {
        return vector2::make_vector2(
            -v1.x,
            -v1.y
            );
    }

    FORCE_INLINE Vector2 operator + (const Vector2 &v1, const Vector2 &v2) {
        return vector2::make_vector2(
            v1.x + v2.x,
            v1.y + v2.y
            );
    }

    FORCE_INLINE Vector2 operator - (const Vector2 &v1, const Vector2 &v2) {
        return vector2::make_vector2(
            v1.x - v2.x,
            v1.y - v2.y
            );
    }

    FORCE_INLINE Vector2 operator* (const Vector2 &v1, const Vector2 &v2) {
        return vector2::make_vector2(
            v1.x * v2.x,
            v1.y * v2.y
            );
    }

    FORCE_INLINE Vector2 operator / (const Vector2 &v1, const Vector2 &v2) {
        return vector2::make_vector2(
            v1.x / v2.x,
            v1.y / v2.y
            );
    }

    FORCE_INLINE Vector2 operator* (const Vector2 &v1, const float s) {
        return vector2::make_vector2(
            v1.x * s,
            v1.y * s
            );
    }

    FORCE_INLINE Vector2 operator / (const Vector2 &v1, const float s) {
        const float inv_s = 1 / s;

        return vector2::make_vector2(
            v1.x * inv_s,
            v1.y * inv_s
            );
    }
}
