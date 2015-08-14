#pragma once

#include <cmath>

#include "types.h"

#define PI (3.1415926535897932f)
#define INV_PI  (0.31830988618f)
#define HALF_PI (1.57079632679f)

#define DOUBLE_SMALL_NUMBER (1.e-8f)
#define FLOAT_SMALL_NUMBER  (1.e-4f)
#define BIG_NUMBER          (3.4e+38f)
#define EULERS_NUMBER       (2.71828182845904523536f)
#define MAX_FLT (3.402823466e+38f)

namespace cetech1 {
    namespace math {
        /*! Return greater of two numbers.
	 * \param a First number.
	 * \param b Second number.
	 * \return greater of two numbers.
	 */
	template<class T>
        FORCE_INLINE T max( const T a, const T b);

        /*! Return smaller of two numbers.
	 * \param a First number.
	 * \param b Second number.
	 * \return smaller of two numbers.
	 */
	template<class T>
	FORCE_INLINE T min( const T a, const T b);

        /*! Clamps a value between a minimum and maximum value.
	 * \param a First number.
	 * \param b Second number.
	 * \return Value between a minimum and maximum.
	 */
	template<class T>
        FORCE_INLINE T clamp( const T a, const T max, const T min);

        /*! Return absolute value.
	 * \param a Number.
	 * \return Absolute value
	 */
        template<class T> FORCE_INLINE T
        abs( const T a);

	/*! Compare two floats.
	 * \param f1 Float 1.
	 * \param f2 Float 2.
	 * \param epsilon Epsilon.
	 */
        FORCE_INLINE bool fcmp(const float f1, const float f2, float epsilon = FLOAT_SMALL_NUMBER);


	FORCE_INLINE float square(float x);
        FORCE_INLINE float fast_sqrt(const float number);
        FORCE_INLINE float inv_sqrt(const float number);

	FORCE_INLINE float deg2rad(float deg);
        FORCE_INLINE float rad2deg(float rad);

        FORCE_INLINE float fast_sin(float angle_deg);
        FORCE_INLINE void fast_sincos(float angle_deg, float& sin, float& cos);

        FORCE_INLINE float float_select(const float a, const float ge_zero, const float lt_zero);
    }


    namespace math {
        template<class T>
        FORCE_INLINE T max( const T a, const T b) {
            return (a >= b) ? a : b;
        }

        template<class T>
        FORCE_INLINE T min( const T a, const T b) {
            return (a <= b) ? a : b;
        }

        template<class T>
        FORCE_INLINE T abs( const T a) {
            return (a >= 0) ? a : -a;
        }

        template<>
        FORCE_INLINE float abs( const float a) {
            const int y = (int&)a & 0x7FFFFFFF;
            return (float&)y;
        }

        template<class T>
        FORCE_INLINE T clamp( const T a, const T max, const T min) {
            return (a < min ? min : (a < max ? a : max));
        }

        FORCE_INLINE bool fcmp(const float f1, const float f2, float epsilon) {
            return abs(f1 - f2) < epsilon;
        }

        FORCE_INLINE float fast_sqrt(const float number) {
            unsigned int i = *(unsigned int*) &number;
            // adjust bias
            i += 127 << 23;
            // approximation of square root
            i >>= 1;
            return *(float*) &i;
        }

        FORCE_INLINE float inv_sqrt(const float number) {
            long i;
            float x2, y;
            const float threehalfs = 1.5F;

            x2 = number * 0.5F;
            y = number;
            i = *(long*) &y;           // evil floating point bit level hacking
            i = 0x5f3759df - (i >> 1); // what the fuck?
            y = *(float*) &i;
            y = y * (threehalfs - (x2 * y * y));    // 1st iteration
            //y  = y * ( threehalfs - ( x2 * y * y ) ); // 2nd iteration, this can be removed

            return y;
        }

        FORCE_INLINE float square(float x) {
            return x * x;
        }

        FORCE_INLINE float deg2rad(float deg) {
            static const float to_rad = 3.14159265358979323846f / 180.0f;
            return deg * to_rad;
        }

        FORCE_INLINE float rad2deg(float rad) {
            static const float to_deg = 180.0f / 3.14159265358979323846f;
            return rad * to_deg;
        }

        FORCE_INLINE float fast_sin(float angle_deg) {
            float angle_rad = deg2rad(angle_deg);

            if (PI < angle_rad) {
                angle_rad = angle_rad - int((angle_rad + PI) * INV_PI) * HALF_PI;
            } else if (angle_rad < -PI) {
                angle_rad = angle_rad - int((angle_rad - PI) * INV_PI) * HALF_PI;
            }

            return angle_rad *
                   (1 - angle_rad * angle_rad *
                    (0.16666667f - angle_rad * angle_rad *
                     (0.00833333f - angle_rad * angle_rad * (0.0001984f - angle_rad * angle_rad * 0.0000027f))));
        }

        FORCE_INLINE void fast_sincos(float angle_deg, float& sin, float& cos) {
            sin = fast_sin(angle_deg);
            cos = fast_sqrt(1 - sin * sin);
        }

        FORCE_INLINE float float_select(const float a, const float ge_zero, const float lt_zero) {
            return a >= 0.0f ? ge_zero : lt_zero;
        }
    }
}
