#ifndef CETECH_FMATH_H
#define CETECH_FMATH_H

//==============================================================================
// Includes
//==============================================================================

#include <math.h>
#include <float.h>

#include "cetech/kernel/types.h"

//==============================================================================
// Constants
//==============================================================================

#define CETECH_float_PI     3.1415926535897932f
#define CETECH_float_INVPI  0.31830988618f
#define CETECH_float_HALFPI 1.57079632679f

#define CETECH_float_TORAD (CETECH_float_PI/180.0f)
#define CETECH_float_TODEG (180.0f/CETECH_float_PI)

#define CETECH_float_E 2.71828182845904523536f

#define CETECH_float_EPSILON FLT_EPSILON
#define CETECH_float_MIN FLT_MIN
#define CETECH_float_MAX FLT_MAX


//==============================================================================
// Interface
//==============================================================================

CETECH_FORCE_INLINE float float_floor(float f) {
    return floorf(f);
}

CETECH_FORCE_INLINE float float_ceil(float f) {
    return ceilf(f);
}

CETECH_FORCE_INLINE float float_round(float f) {
    return float_floor(f + 0.5f);
}

CETECH_FORCE_INLINE float float_min(float a,
                                    float b) {
    return a < b ? a : b;
}

CETECH_FORCE_INLINE float float_min3(float a,
                                     float b,
                                     float c) {
    return float_min(a, float_min(b, c));
}

CETECH_FORCE_INLINE float float_max(float a,
                                    float b) {
    return a > b ? a : b;
}

CETECH_FORCE_INLINE float float_max3(float a,
                                     float b,
                                     float c) {
    return float_max(a, float_max(b, c));
}

CETECH_FORCE_INLINE float float_abs(float a) {
    return a < 0.0f ? -a : a;
}

CETECH_FORCE_INLINE float float_clamp(float a,
                                      float min,
                                      float max) {
    return float_min(float_max(a, min), max);
}

CETECH_FORCE_INLINE float float_saturate(float a) {
    return float_clamp(a, 0.0f, 1.0f);
}

CETECH_FORCE_INLINE float float_lerp(float a,
                                     float b,
                                     float t) {
    return a + (b - a) * t;
}

CETECH_FORCE_INLINE float float_sign(float a) {
    return a < 0.0f ? -1.0f : 1.0f;
}

CETECH_FORCE_INLINE float float_step(float edge,
                                     float a) {
    return a < edge ? 0.0f : 1.0f;
}

CETECH_FORCE_INLINE float float_pulse(float a,
                                      float start,
                                      float end) {
    return float_step(a, start) - float_step(a, end);
}

CETECH_FORCE_INLINE int float_equal(float a,
                                    float b,
                                    float epsilon) {
    // http://realtimecollisiondetection.net/blog/?p=89
    const float lhs = float_abs(a - b);
    const float rhs = epsilon * float_max3(1.0f, float_abs(a), float_abs(b));
    return lhs <= rhs;
}

CETECH_FORCE_INLINE int float_equals(const float *__restrict _a,
                                     const float *__restrict _b,
                                     int _num,
                                     float _epsilon) {

    int equal = float_equal(_a[0], _b[0], _epsilon);

    for (int i = 1; equal && i < _num; ++i) {
        equal = float_equal(_a[i], _b[i], _epsilon);
    }
    return equal;
}

CETECH_FORCE_INLINE float float_bias(float time,
                                     float bias) {
    /// http://blog_demofox.org/2012/09/24/bias-and-gain-are-your-friend/
    return time / ((1.0f / bias - 2.0f) * (1.0f - time) + 1.0f);
}


CETECH_FORCE_INLINE float float_gain(float time,
                                     float gain) {
    /// http://blog_demofox.org/2012/09/24/bias-and-gain-are-your-friend/
    if (time < 0.5f)
        return float_bias(time * 2.0f, gain) / 2.0f;

    return float_bias(time * 2.0f - 1.0f, 1.0f - gain) / 2.0f + 0.5f;
}

CETECH_FORCE_INLINE float float_to_rad(float angle) {
    return angle * CETECH_float_TORAD;
}

CETECH_FORCE_INLINE float float_to_deg(float angle) {
    return angle * CETECH_float_TODEG;
}

CETECH_FORCE_INLINE float float_sin(float angle) {
    return sinf(angle);
}

CETECH_FORCE_INLINE float float_cos(float angle) {
    return cosf(angle);
}

CETECH_FORCE_INLINE float float_tan(float angle) {
    return tanf(angle);
}

CETECH_FORCE_INLINE float float_fast_inv_sqrt(float number) {
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

CETECH_FORCE_INLINE float float_fast_sqrt(float number) {
    return float_fast_inv_sqrt(number) * number;
}

CETECH_FORCE_INLINE float float_sqrt(float number) {
    return sqrtf(number);
}

CETECH_FORCE_INLINE float float_sq(float f) {
    return f * f;
}

CETECH_FORCE_INLINE float float_atan2(float y,
                                      float x) {
    return atan2f(y, x);
}

CETECH_FORCE_INLINE float float_asin(float f) {
    if (-1.0f < f) {
        if (f < 1.0f)
            return asinf(f);

        return CETECH_float_HALFPI;
    }

    return -CETECH_float_HALFPI;
}

CETECH_FORCE_INLINE float float_acos(float f) {
    if (-1.0f < f) {
        if (f < 1.0f)
            return acosf(f);

        return 0.0f;
    }

    return CETECH_float_PI;
}

#endif //CETECH_FMATH_H
