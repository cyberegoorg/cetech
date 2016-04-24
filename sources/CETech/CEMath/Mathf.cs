using System;
using System.Runtime.InteropServices;
using MoonSharp.Interpreter;

namespace CETech
{
    [MoonSharpUserData]
    public static class Mathf
    {
        /// <summary>
        ///     TT
        /// </summary>
        public static readonly float Pi = 3.1415926535897932f;

        /// <summary>
        ///     1/TT
        /// </summary>
        public static readonly float InvPi = 0.31830988618f;

        /// <summary>
        ///     TT/2
        /// </summary>
        public static readonly float HalfPi = 1.57079632679f;

        public static readonly float ToRad = Pi/180.0f;
        public static readonly float ToDeg = 180.0f/Pi;

        /// <summary>
        ///     e
        /// </summary>
        public static readonly float E = 2.71828182845904523536f;

        public static readonly float FEpsilon = float.Epsilon;
        public static readonly float FMax = float.MaxValue;
        public static readonly float FMin = float.MinValue;

        public static float Floor(float f)
        {
            return (float) Math.Floor(f);
        }

        public static float Ceil(float f)
        {
            return (float) Math.Ceiling(f);
        }

        public static float Round(float f)
        {
            return Floor(f + 0.5f);
        }

        public static float Min(float a, float b)
        {
            return a < b ? a : b;
        }

        public static float Max(float a, float b)
        {
            return a > b ? a : b;
        }

        public static float Abs(float a)
        {
            return a < 0.0f ? -a : a;
        }

        /// <summary>
        ///     http://blog.demofox.org/2012/09/24/bias-and-gain-are-your-friend/
        /// </summary>
        /// <param name="time"></param>
        /// <param name="bias"></param>
        /// <returns></returns>
        public static float Bias(float time, float bias)
        {
            return time/((1.0f/bias - 2.0f)*(1.0f - time) + 1.0f);
        }

        /// <summary>
        ///     http://blog.demofox.org/2012/09/24/bias-and-gain-are-your-friend/
        /// </summary>
        /// <param name="time"></param>
        /// <param name="gain"></param>
        /// <returns></returns>
        public static float Gain(float time, float gain)
        {
            if (time < 0.5f)
                return Bias(time*2.0f, gain)/2.0f;

            return Bias(time*2.0f - 1.0f, 1.0f - gain)/2.0f + 0.5f;
        }

        /// <summary>
        ///     Covert Degrese to radians
        /// </summary>
        /// <param name="angle">Angle in degrese</param>
        /// <returns>Angle in radians</returns>
        public static float DegToRad(float angle)
        {
            return angle*ToRad;
        }

        /// <summary>
        ///     Covert radians to degrese
        /// </summary>
        /// <param name="angle">Angle in radians</param>
        /// <returns>Angle in degrese</returns>
        public static float RadToDeg(float angle)
        {
            return angle*ToDeg;
        }

        /// <summary>
        ///     Sin
        /// </summary>
        /// <param name="angle">Angle</param>
        /// <returns>Sin</returns>
        public static float Sin(float angle)
        {
            return (float) Math.Sin(angle);
        }

        /// <summary>
        ///     Cos
        /// </summary>
        /// <param name="angle">Angle</param>
        /// <returns>Cos</returns>
        public static float Cos(float angle)
        {
            return (float) Math.Cos(angle);
        }

        public static float FastSqrtf(float number)
        {
            return FastInvSqrtf(number)*number;
        }

        public static float FastInvSqrtf(float number)
        {
            //return (float) (1.0f/Math.Sqrt(number));
            float fuconstant = 0x5f375a86;
            var three_halfs = 1.5f;
            var number_half = number*0.5f;

            FloatLongUnion fl;
            fl.l = 0;
            fl.f = number;

            fl.l = (long) (fuconstant - (fl.l >> 1)); // what the fuck?

            fl.f = fl.f*(three_halfs - number_half*fl.f*fl.f); // 1st iteration
            fl.f = fl.f*(three_halfs - number_half*fl.f*fl.f); // 2nd iteration

            return fl.f;
        }

        public static float Square(float f)
        {
            return f*f;
        }

        public static float Atan2(float y, float x)
        {
            return (float) Math.Atan2(y, x);
        }

        public static float Asin(float f)
        {
            if (-1.0f < f)
            {
                if (f < 1.0f)
                    return (float) Math.Asin(f);
                return HalfPi;
            }

            return -HalfPi;
        }

        public static float Acos(float f)
        {
            if (-1.0f < f)
            {
                if (f < 1.0f)
                    return (float) Math.Acos(f);

                return 0.0f;
            }

            return Pi;
        }

        [StructLayout(LayoutKind.Explicit)]
        private struct FloatLongUnion
        {
            [FieldOffset(0)] public float f;

            [FieldOffset(0)] public long l;
        }
    }
}