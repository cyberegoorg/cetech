#pragma once

#include "math_types.h"
#include "math.h"
#include "simd/simd.h"

namespace cetech1 {
    namespace matrix33 {
        /*! Identity matrix.
         */
        static const Matrix33 IDENTITY = {
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f}
        };
    }

    CE_INLINE Matrix33 operator* (const Matrix33 &m1, const Matrix33 &m2);



    Matrix33 operator* (const Matrix33 &m1, const Matrix33 &m2) {
        Matrix33 res;
        simd::mat33_mult(&m1, &m2, &res);
        return res;
    }

}
