#ifndef CELIB_FMATH_H
#define CELIB_FMATH_H

//==============================================================================
// Includes
//==============================================================================

#include <math.h>
#include <float.h>

#include "../types.h"

//==============================================================================
// Constants
//==============================================================================

#define f32_PI     3.1415926535897932f
#define f32_InvPI  0.31830988618f
#define f32_HalfPI 1.57079632679f

#define f32_ToRad (f32_PI/180.0f)
#define f32_ToDeg (180.0f/f32_PI)

#define f32_E 2.71828182845904523536f

#define f32_Epsilon FLT_EPSILON
#define f32_Min FLT_MIN
#define f32_Max FLT_MAX


//==============================================================================
// Interface
//==============================================================================

CEL_FORCE_INLINE float f32_floor(float f) {
    return floorf(f);
}

CEL_FORCE_INLINE float f32_ceil(float f) {
    return ceilf(f);
}

CEL_FORCE_INLINE float f32_round(float f) {
    return f32_floor(f + 0.5f);
}

CEL_FORCE_INLINE float f32_min(float a,
                              float b) {
    return a < b ? a : b;
}

CEL_FORCE_INLINE float f32_min3(float a,
                               float b,
                               float c) {
    return f32_min(a, f32_min(b, c));
}

CEL_FORCE_INLINE float f32_max(float a,
                              float b) {
    return a > b ? a : b;
}

CEL_FORCE_INLINE float f32_max3(float a,
                               float b,
                               float c) {
    return f32_max(a, f32_max(b, c));
}

CEL_FORCE_INLINE float f32_abs(float a) {
    return a < 0.0f ? -a : a;
}

CEL_FORCE_INLINE float f32_clamp(float a,
                                float min,
                                float max) {
    return f32_min(f32_max(a, min), max);
}

CEL_FORCE_INLINE float f32_saturate(float a) {
    return f32_clamp(a, 0.0f, 1.0f);
}

CEL_FORCE_INLINE float f32_lerp(float a,
                               float b,
                               float t) {
    return a + (b - a) * t;
}

CEL_FORCE_INLINE float f32_sign(float a) {
    return a < 0.0f ? -1.0f : 1.0f;
}

CEL_FORCE_INLINE float f32_step(float edge,
                               float a) {
    return a < edge ? 0.0f : 1.0f;
}

CEL_FORCE_INLINE float f32_pulse(float a,
                                float start,
                                float end) {
    return f32_step(a, start) - f32_step(a, end);
}

CEL_FORCE_INLINE int f32_equal(float a,
                              float b,
                              float epsilon) {
    // http://realtimecollisiondetection.net/blog/?p=89
    const float lhs = f32_abs(a - b);
    const float rhs = epsilon * f32_max3(1.0f, f32_abs(a), f32_abs(b));
    return lhs <= rhs;
}

CEL_FORCE_INLINE int f32_equals(const float *__restrict _a,
                               const float *__restrict _b,
                               int _num,
                               float _epsilon) {

    int equal = f32_equal(_a[0], _b[0], _epsilon);

    for (int i = 1; equal && i < _num; ++i) {
        equal = f32_equal(_a[i], _b[i], _epsilon);
    }
    return equal;
}

CEL_FORCE_INLINE float f32_bias(float time,
                               float bias) {
    /// http://blog_demofox.org/2012/09/24/bias-and-gain-are-your-friend/
    return time / ((1.0f / bias - 2.0f) * (1.0f - time) + 1.0f);
}


CEL_FORCE_INLINE float f32_gain(float time,
                               float gain) {
    /// http://blog_demofox.org/2012/09/24/bias-and-gain-are-your-friend/
    if (time < 0.5f)
        return f32_bias(time * 2.0f, gain) / 2.0f;

    return f32_bias(time * 2.0f - 1.0f, 1.0f - gain) / 2.0f + 0.5f;
}

CEL_FORCE_INLINE float f32_to_rad(float angle) {
    return angle * f32_ToRad;
}

CEL_FORCE_INLINE float f32_to_deg(float angle) {
    return angle * f32_ToDeg;
}

CEL_FORCE_INLINE float f32_sin(float angle) {
    return sinf(angle);
}

CEL_FORCE_INLINE float f32_cos(float angle) {
    return cosf(angle);
}

CEL_FORCE_INLINE float f32_tan(float angle) {
    return tanf(angle);
}

CEL_FORCE_INLINE float f32_fast_inv_sqrt(float number) {
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

CEL_FORCE_INLINE float f32_fast_sqrt(float number) {
    return f32_fast_inv_sqrt(number) * number;
}

CEL_FORCE_INLINE float f32_sqrt(float number) {
    return sqrtf(number);
}

CEL_FORCE_INLINE float f32_sq(float f) {
    return f * f;
}

CEL_FORCE_INLINE float f32_atan2(float y,
                                float x) {
    return atan2f(y, x);
}

CEL_FORCE_INLINE float f32_asin(float f) {
    if (-1.0f < f) {
        if (f < 1.0f)
            return asinf(f);

        return f32_HalfPI;
    }

    return -f32_HalfPI;
}

CEL_FORCE_INLINE float f32_acos(float f) {
    if (-1.0f < f) {
        if (f < 1.0f)
            return acosf(f);

        return 0.0f;
    }

    return f32_PI;
}

#endif //CELIB_FMATH_H
