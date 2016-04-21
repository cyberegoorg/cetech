using System;
using System.Runtime.InteropServices;

namespace CETech
{
    public class CMath
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
        public static readonly float EulersNumber = 2.71828182845904523536f;

        public static readonly double DoubleSmallNumber = 1e-8f;
        public static readonly float FloatSmallNumber = 1e-4f;
        public static readonly float BigNumber = .4e+38f;
        public static readonly float MaxFlt = 3.402823466e+38f;

        /// <summary>
        ///     Covert Degrese to radians
        /// </summary>
        /// <param name="angle">Angle in deg</param>
        /// <returns>Angle in radians</returns>
        public static float DegToRad(float angle)
        {
            return angle*ToRad;
        }

        /// <summary>
        ///     Sin
        /// </summary>
        /// <param name="angle">Angle</param>
        /// <returns>Sin</returns>
        public static float Sinf(float angle)
        {
            return (float) Math.Sin(angle);
        }

        /// <summary>
        ///     Cos
        /// </summary>
        /// <param name="angle">Angle</param>
        /// <returns>Cos</returns>
        public static float Cosf(float angle)
        {
            return (float) Math.Cos(angle);
        }

        public static float Abs(float a)
        {
            return a < 0.0f ? -a : a;
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

        [StructLayout(LayoutKind.Explicit)]
        private struct FloatLongUnion
        {
            [FieldOffset(0)] public float f;

            [FieldOffset(0)] public long l;
        }

        public static float Square(float f)
        {
            return f*f;
        }

        public static float Atan2f(float y, float x)
        {
            return (float) Math.Atan2(y, x);
        }

        public static float Asin(float f)
        {
            if (-1.0f < f)
            {
                if (f < 1.0f) 
                    return (float)Math.Asin(f);
                return HalfPi;
            }

            return -HalfPi;
        }

        public static float Acos(float f)
        {
            if (-1.0f < f)
            {
                if (f < 1.0f)
                    return (float)Math.Acos(f);

                return 0.0f;
            }

            return Pi;
        }
    }
}