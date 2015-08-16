#pragma once

#include "math_types.h"
#include "math.h"
#include "simd/simd.h"

namespace cetech1 {
    /*! Quat functions.
     */
    namespace quat {
        /*! Make Quat
         * \param x X
         * \param y Y
         * \param z Z
	 * \param w W
         * \return New vector
         */
        FORCE_INLINE Quat make_quat(float x, float y, float z, float w);


        /*! Quaternion len
         * \param q1 Quaternion.
         * \return Quaternion len.
         */
        FORCE_INLINE float len(const Quat& q1);

        /*! Quaternion squared len.
         * \param q1 Quaternion.
         * \return Quaternion squared len.
         */
        FORCE_INLINE float len_sq(const Quat& q1);

        /*! Quaternion 1/len
         * \param q1 Quaternion.
         * \return Quaternion 1/len.
         */
        FORCE_INLINE float len_inv(const Quat& q1);


        /*! Return normalized vector.
         * \param q1 Quaternion.
         * \return Normalized vector.
         */
        FORCE_INLINE Quat normalized(const Quat& q1);
    }

    /*! Return negativ vector.
     * \param q1 Quaternion.
     * \return -q1
     */
    FORCE_INLINE Quat operator - (const Quat &q1);


    /*! V1 + V2
     * \param q1 Quaternion1.
     * \param q2 Quat.
     * \return V1 + V2.
     */
    FORCE_INLINE Quat operator + (const Quat &q1, const Quat &q2);

    /*! V1 - V2
     * \param q1 Quaternion1.
     * \param q2 Quat.
     * \return V1 - V2.
     */
    FORCE_INLINE Quat operator - (const Quat &q1, const Quat &q2);

    FORCE_INLINE Quat operator* (const Quat &q1, const Quat &q2);
    
    /*! V1 * scalar
     * \param q1 Quaternion1.
     * \param s ScalarQuat.
     * \return V1 * scalar
     */
    FORCE_INLINE Quat operator* (const Quat &q1, const float s);

    /*! V1 / scalar
     * \param q1 Quaternion1.
     * \param s ScalarQuat.
     * \return V1 / scalar
     */
    FORCE_INLINE Quat operator / (const Quat &q1, const float s);



    namespace quat {
        FORCE_INLINE Quat make_quat(float x, float y, float z, float w) {
            return {
                       x, y, z, w
            };
        }

        FORCE_INLINE float len(const Quat& q1) {
            return math::fast_sqrt(len_sq(q1));
        }

        FORCE_INLINE float len_sq(const Quat& q1) {
            return (q1.x * q1.x) + (q1.y * q1.y) + (q1.z * q1.z)+ (q1.w * q1.w);
        }

        FORCE_INLINE float len_inv(const Quat& q1) {
            return math::inv_sqrt(len_sq(q1));
        }

        FORCE_INLINE Quat normalized(const Quat& q1) {
            return q1 * len_inv(q1);
        }

    }

    FORCE_INLINE Quat operator - (const Quat &q1) {
        return quat::make_quat(
            -q1.x,
            -q1.y,
            -q1.z,
	    -q1.w
            );
    }

    FORCE_INLINE Quat operator + (const Quat &q1, const Quat &q2) {
        return quat::make_quat(
            q1.x + q2.x,
            q1.y + q2.y,
            q1.z + q2.z,
	    q1.w + q2.w
            );
    }

    FORCE_INLINE Quat operator - (const Quat &q1, const Quat &q2) {
        return quat::make_quat(
            q1.x - q2.x,
            q1.y - q2.y,
            q1.z - q2.z,
	    q1.w - q2.w
            );
    }

    FORCE_INLINE Quat operator* (const Quat &q1, const Quat &q2) {
      Quat res;
      simd::quat_mult(&q1, &q2, &res);
      return res;
    }
    
    FORCE_INLINE Quat operator* (const Quat &q1, const float s) {
        return quat::make_quat(
            q1.x * s,
            q1.y * s,
            q1.z * s,
            q1.w * s
            );
    }

    FORCE_INLINE Quat operator / (const Quat &q1, const float s) {
        const float inv_s = 1 / s;

        return quat::make_quat(
            q1.x * inv_s,
            q1.y * inv_s,
            q1.z * inv_s,
	    q1.w * inv_s
            );
    }
}
