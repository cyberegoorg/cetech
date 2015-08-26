#pragma once

#include "math_types.h"
#include "math.h"
#include "simd/simd.h"

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
	
	/*! Make matrix44 from axis and angle.
         * \param axis Axis
         * \param angle_deg Angle in deg
         * \return New rotation matrix
         */
        CE_INLINE Matrix44 from_axis_angle(const Vector3& axis, const float angle_deg);
    }

    CE_INLINE Matrix44 operator* (const Matrix44 &m1, const Matrix44 &m2);


    namespace matrix44 {
	Matrix44 make_matrix44(const Vector4& x, const Vector4& y, const Vector4& z, const Vector4& t) {
	    Matrix44 res = {x, y, z, t};
	    return res;
	}
	
	Matrix44 from_axis_angle ( const Vector3& axis, const float angle_deg ) {
	    float sin, cos, C;

	    sin = cos = 0.0f;
	    C = 1.0f - cos;
	    
	    math::fast_sincos(angle_deg, sin, cos);
	    
	    return make_matrix44(
		{(axis.x * axis.x * C) + cos, (axis.x * axis.y * C) - (axis.z * sin), (axis.x * axis.z * C) + (axis.y * sin), 0.0f},
		{(axis.y * axis.x * C) + (axis.z * sin), (axis.y * axis.y * C) + cos, (axis.y * axis.z * C) - (axis.x * sin), 0.0f},
		{(axis.z * axis.x * C) - (axis.y * sin), (axis.z * axis.y * C) + (axis.x * sin), (axis.z * axis.z * C) + cos, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
	    );
	}
    }

    Matrix44 operator* (const Matrix44 &m1, const Matrix44 &m2) {
        Matrix44 res;
        simd::mat44_mult(&m1, &m2, &res);
        return res;
    }

}
