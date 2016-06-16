/***********************************************************************
**** Float math functions
***********************************************************************/

#ifndef CETECH_FMATH_H
#define CETECH_FMATH_H

/***********************************************************************
**** Includes
***********************************************************************/

#include <math.h>
#include <float.h>

#include "../utils/macros.h"


/***********************************************************************
**** Constants
***********************************************************************/

#define f_PI     3.1415926535897932f
#define f_InvPI  0.31830988618f
#define f_HalfPI 1.57079632679f

#define f_ToRad (f_PI/180.0f)
#define f_ToDeg (180.0f/f_PI)

#define f_E 2.71828182845904523536f

#define f_Epsilon FLT_EPSILON
#define f_Min FLT_MIN
#define f_Max FLT_MAX


/***********************************************************************
**** Functions
***********************************************************************/

static CE_FORCE_INLINE float f_floor(float f) {
    return floorf(f);
}

static CE_FORCE_INLINE float f_ceil(float f) {
    return ceilf(f);
}

static CE_FORCE_INLINE float f_round(float f) {
    return f_floor(f + 0.5f);
}

static CE_FORCE_INLINE float f_min(float a, float b) {
    return a < b ? a : b;
}

static CE_FORCE_INLINE float f_min3(float a, float b, float c) {
    return f_min(a, f_min(b, c));
}

static CE_FORCE_INLINE float f_max(float a, float b) {
    return a > b ? a : b;
}

static CE_FORCE_INLINE float f_max3(float a, float b, float c) {
    return f_max(a, f_max(b, c));
}

static CE_FORCE_INLINE float f_abs(float a) {
    return a < 0.0f ? -a : a;
}

static CE_FORCE_INLINE float f_clamp(float a, float min, float max)
{
    return f_min(f_max(a, min), max);
}

static CE_FORCE_INLINE float f_saturate(float a)
{
    return f_clamp(a, 0.0f, 1.0f);
}

static CE_FORCE_INLINE float f_lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

static CE_FORCE_INLINE float f_sign(float a)
{
    return a < 0.0f ? -1.0f : 1.0f;
}

static CE_FORCE_INLINE float f_step(float edge, float a)
{
    return a < edge ? 0.0f : 1.0f;
}

static CE_FORCE_INLINE float f_pulse(float a, float start, float end)
{
    return f_step(a, start) - f_step(a, end);
}

static CE_FORCE_INLINE bool f_equal(float a, float b, float epsilon) {
    // http://realtimecollisiondetection.net/blog/?p=89
    const float lhs = f_abs(a - b);
    const float rhs = epsilon * f_max3(1.0f, f_abs(a), f_abs(b));
    return lhs <= rhs;
}

static CE_FORCE_INLINE bool f_equals(const float *__restrict _a,
                                     const float *__restrict _b,
                                     int _num,
                                     float _epsilon) {

    bool equal = f_equal(_a[0], _b[0], _epsilon);

    for (int i = 1; equal && i < _num; ++i) {
        equal = f_equal(_a[i], _b[i], _epsilon);
    }
    return equal;
}

static CE_FORCE_INLINE float f_bias(float time, float bias) {
    /// http://blog.demofox.org/2012/09/24/bias-and-gain-are-your-friend/
    return time / ((1.0f / bias - 2.0f) * (1.0f - time) + 1.0f);
}


static CE_FORCE_INLINE float f_gain(float time, float gain) {
    /// http://blog.demofox.org/2012/09/24/bias-and-gain-are-your-friend/
    if (time < 0.5f)
        return f_bias(time * 2.0f, gain) / 2.0f;

    return f_bias(time * 2.0f - 1.0f, 1.0f - gain) / 2.0f + 0.5f;
}

static CE_FORCE_INLINE float f_to_rad(float angle) {
    return angle * f_ToRad;
}

static CE_FORCE_INLINE float f_to_deg(float angle) {
    return angle * f_ToDeg;
}

static CE_FORCE_INLINE float f_sin(float angle) {
    return sinf(angle);
}

static CE_FORCE_INLINE float f_cos(float angle) {
    return cosf(angle);
}

static CE_FORCE_INLINE float f_fast_inv_sqrt(float number) {
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

static CE_FORCE_INLINE float f_fast_sqrt(float number) {
    return f_fast_inv_sqrt(number) * number;
}

static CE_FORCE_INLINE float f_sqrt(float number) {
    return sqrtf(number);
}

static CE_FORCE_INLINE float f_sq(float f) {
    return f * f;
}

static CE_FORCE_INLINE float f_atan2(float y, float x) {
    return atan2f(y, x);
}

static CE_FORCE_INLINE float f_asin(float f) {
    if (-1.0f < f) {
        if (f < 1.0f)
            return asinf(f);

        return f_HalfPI;
    }

    return -f_HalfPI;
}

static CE_FORCE_INLINE float f_acos(float f) {
    if (-1.0f < f) {
        if (f < 1.0f)
            return acosf(f);

        return 0.0f;
    }

    return f_PI;
}

#endif //CETECH_FMATH_H
