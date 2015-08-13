#pragma once

#include <cmath>

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
        inline float fabs(float f) ;
        inline bool fcmp(const float f1, const float f2, float epsilon = FLOAT_SMALL_NUMBER);

        inline float fast_sqrt(const float number);
        inline float inv_sqrt(const float number);
        
        inline float deg2rad(float deg);
        inline float rad2deg(float rad);
        
        inline float fast_sin(float angle);
        inline void fast_sincos(float angle, float& sin, float& cos);
    }

    namespace math {
        inline float fabs(float f) {
            const int y = (int&)f & 0x7FFFFFFF;
            return (float&)y;
        }

        inline bool fcmp(const float f1, const float f2, float epsilon) {
            return fabs(f1 - f2) < epsilon;
        }


        inline float fast_sqrt(const float number) {
            unsigned int i = *(unsigned int*) &number; 
            // adjust bias
            i  += 127 << 23;
            // approximation of square root
            i >>= 1; 
            return *(float*) &i;
        }

        inline float inv_sqrt(const float number) {
          long i;
          float x2, y;
          const float threehalfs = 1.5F;

          x2 = number * 0.5F;
          y  = number;
          i  = * ( long * ) &y;  // evil floating point bit level hacking
          i  = 0x5f3759df - ( i >> 1 ); // what the fuck?
          y  = * ( float * ) &i;
          y  = y * ( threehalfs - ( x2 * y * y ) ); // 1st iteration
          // y  = y * ( threehalfs - ( x2 * y * y ) ); // 2nd iteration, this can be removed

          return y;
        }
        
        inline float deg2rad(float deg) {
            static const float to_rad = 3.14159265358979323846f / 180.0f;
            return deg * to_rad;
        }
        
        inline float rad2deg(float rad) {
            static const float to_deg = 180.0f / 3.14159265358979323846f;
            return rad * to_deg;
        }
        
        inline float fast_sin(float angle) {
            angle = deg2rad(angle);
            
            if(PI < angle) {
                angle = angle-int((angle+PI)*INV_PI)*HALF_PI;
            }
            else if(angle < -PI) {
                angle = angle-int((angle-PI)*INV_PI)*HALF_PI;
            }
            
            return angle*(1 - angle*angle*(0.16666667f - angle*angle*(0.00833333f - angle*angle*(0.0001984f - angle*angle*0.0000027f))));
        }
        
        inline void fast_sincos(float angle, float& sin, float& cos) {
            sin = fast_sin(angle);
            cos = fast_sqrt(1-sin*sin);
        }
    }
}
