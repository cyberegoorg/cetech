#pragma once

#include "math_types.h"
#include "math.h"
#include "simd/simd.h"

namespace cetech1 {
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

    CE_INLINE Matrix44 operator* (const Matrix44 &m1, const Matrix44 &m2);



    Matrix44 operator* (const Matrix44 &m1, const Matrix44 &m2) {
        Matrix44 res;
        simd::mat44_mult(&m1, &m2, &res);
        return res;
    }

}
