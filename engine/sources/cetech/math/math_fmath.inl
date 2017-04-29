#ifndef CELIB_FMATH_H
#define CELIB_FMATH_H

//==============================================================================
// Includes
//==============================================================================

#include <math.h>
#include <float.h>

#include "cetech/types.h"

//==============================================================================
// Constants
//==============================================================================

#define CEL_float_PI     3.1415926535897932f
#define CEL_float_INVPI  0.31830988618f
#define CEL_float_HALFPI 1.57079632679f

#define CEL_float_TORAD (CEL_float_PI/180.0f)
#define CEL_float_TODEG (180.0f/CEL_float_PI)

#define CEL_float_E 2.71828182845904523536f

#define CEL_float_EPSILON FLT_EPSILON
#define CEL_float_MIN FLT_MIN
#define CEL_float_MAX FLT_MAX


//==============================================================================
// Interface
//==============================================================================

CEL_FORCE_INLINE float cel_float_floor(float f) {
    return floorf(f);
}

CEL_FORCE_INLINE float cel_float_ceil(float f) {
    return ceilf(f);
}

CEL_FORCE_INLINE float cel_float_round(float f) {
    return cel_float_floor(f + 0.5f);
}

CEL_FORCE_INLINE float cel_float_min(float a,
                                   float b) {
    return a < b ? a : b;
}

CEL_FORCE_INLINE float cel_float_min3(float a,
                                    float b,
                                    float c) {
    return cel_float_min(a, cel_float_min(b, c));
}

CEL_FORCE_INLINE float cel_float_max(float a,
                                   float b) {
    return a > b ? a : b;
}

CEL_FORCE_INLINE float cel_float_max3(float a,
                                    float b,
                                    float c) {
    return cel_float_max(a, cel_float_max(b, c));
}

CEL_FORCE_INLINE float cel_float_abs(float a) {
    return a < 0.0f ? -a : a;
}

CEL_FORCE_INLINE float cel_float_clamp(float a,
                                     float min,
                                     float max) {
    return cel_float_min(cel_float_max(a, min), max);
}

CEL_FORCE_INLINE float cel_float_saturate(float a) {
    return cel_float_clamp(a, 0.0f, 1.0f);
}

CEL_FORCE_INLINE float cel_float_lerp(float a,
                                    float b,
                                    float t) {
    return a + (b - a) * t;
}

CEL_FORCE_INLINE float cel_float_sign(float a) {
    return a < 0.0f ? -1.0f : 1.0f;
}

CEL_FORCE_INLINE float cel_float_step(float edge,
                                    float a) {
    return a < edge ? 0.0f : 1.0f;
}

CEL_FORCE_INLINE float cel_float_pulse(float a,
                                     float start,
                                     float end) {
    return cel_float_step(a, start) - cel_float_step(a, end);
}

CEL_FORCE_INLINE int cel_float_equal(float a,
                                   float b,
                                   float epsilon) {
    // http://realtimecollisiondetection.net/blog/?p=89
    const float lhs = cel_float_abs(a - b);
    const float rhs = epsilon * cel_float_max3(1.0f, cel_float_abs(a), cel_float_abs(b));
    return lhs <= rhs;
}

CEL_FORCE_INLINE int cel_float_equals(const float *__restrict _a,
                                    const float *__restrict _b,
                                    int _num,
                                    float _epsilon) {

    int equal = cel_float_equal(_a[0], _b[0], _epsilon);

    for (int i = 1; equal && i < _num; ++i) {
        equal = cel_float_equal(_a[i], _b[i], _epsilon);
    }
    return equal;
}

CEL_FORCE_INLINE float cel_float_bias(float time,
                                    float bias) {
    /// http://blog_demofox.org/2012/09/24/bias-and-gain-are-your-friend/
    return time / ((1.0f / bias - 2.0f) * (1.0f - time) + 1.0f);
}


CEL_FORCE_INLINE float cel_float_gain(float time,
                                    float gain) {
    /// http://blog_demofox.org/2012/09/24/bias-and-gain-are-your-friend/
    if (time < 0.5f)
        return cel_float_bias(time * 2.0f, gain) / 2.0f;

    return cel_float_bias(time * 2.0f - 1.0f, 1.0f - gain) / 2.0f + 0.5f;
}

CEL_FORCE_INLINE float cel_float_to_rad(float angle) {
    return angle * CEL_float_TORAD;
}

CEL_FORCE_INLINE float cel_float_to_deg(float angle) {
    return angle * CEL_float_TODEG;
}

CEL_FORCE_INLINE float cel_float_sin(float angle) {
    return sinf(angle);
}

CEL_FORCE_INLINE float cel_float_cos(float angle) {
    return cosf(angle);
}

CEL_FORCE_INLINE float cel_float_tan(float angle) {
    return tanf(angle);
}

CEL_FORCE_INLINE float cel_float_fast_inv_sqrt(float number) {
    // QUAKE3 fast inverse TODO: URL here

    //return (float) (1.0f/Math.Sqrt(number));

    const float fuconstant = 0x5f375a86;
    const float three_halfs = 1.5f;
    const float number_half = number * 0.5f;

    union {
        float f;
        long l;
    } fl = {.f = number};

    fl.l = (long) (fuconstant - (fl.l >> 1)); // what the fuck?

    fl.f = fl.f * (three_halfs - number_half * fl.f * fl.f); // 1st iteration
    fl.f = fl.f * (three_halfs - number_half * fl.f * fl.f); // 2nd iteration

    return fl.f;
}

CEL_FORCE_INLINE float cel_float_fast_sqrt(float number) {
    return cel_float_fast_inv_sqrt(number) * number;
}

CEL_FORCE_INLINE float cel_float_sqrt(float number) {
    return sqrtf(number);
}

CEL_FORCE_INLINE float cel_float_sq(float f) {
    return f * f;
}

CEL_FORCE_INLINE float cel_float_atan2(float y,
                                     float x) {
    return atan2f(y, x);
}

CEL_FORCE_INLINE float cel_float_asin(float f) {
    if (-1.0f < f) {
        if (f < 1.0f)
            return asinf(f);

        return CEL_float_HALFPI;
    }

    return -CEL_float_HALFPI;
}

CEL_FORCE_INLINE float cel_float_acos(float f) {
    if (-1.0f < f) {
        if (f < 1.0f)
            return acosf(f);

        return 0.0f;
    }

    return CEL_float_PI;
}

#endif //CELIB_FMATH_H
