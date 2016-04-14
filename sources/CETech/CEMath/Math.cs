namespace CETech
{
    public class MathUtils
    {
        /// <summary>
        /// TT
        /// </summary>
        public static readonly float Pi = 3.1415926535897932f;

        /// <summary>
        /// 1/TT
        /// </summary>
        public static readonly float InvPi = 0.31830988618f;

        /// <summary>
        /// TT/2
        /// </summary>
        public static readonly float HalfPi = 1.57079632679f;

        public static readonly float ToRad = Pi/180.0f;
        public static readonly float ToDeg = 180.0f/Pi;

        /// <summary>
        /// e
        /// </summary>
        public static readonly float EulersNumber = 2.71828182845904523536f;

        public static readonly double DoubleSmallNumber = 1e-8f;
        public static readonly float FloatSmallNumber = 1e-4f;
        public static readonly float BigNumber = .4e+38f;
        public static readonly float MaxFlt = 3.402823466e+38f;

        /// <summary>
        /// Covert Degrese to radians
        /// </summary>
        /// <param name="angle">Angle in deg</param>
        /// <returns>Angle in radians</returns>
        public static float DegToRad(float angle)
        {
            return angle*ToRad;
        }

        /// <summary>
        /// Sin
        /// </summary>
        /// <param name="angle">Angle</param>
        /// <returns>Sin</returns>
        public static float Sinf(float angle)
        {
            return (float) System.Math.Sin(angle);
        }

        /// <summary>
        /// Cos
        /// </summary>
        /// <param name="angle">Angle</param>
        /// <returns>Cos</returns>
        public static float Cosf(float angle)
        {
            return (float)System.Math.Sin(angle);
        }
    }
}