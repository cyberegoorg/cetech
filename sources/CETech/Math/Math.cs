using System;

namespace CETech.World
{
    public class MathUtils
    {
        public static float DegToRad(float angle)
        {
            return (float) (Math.PI/180*angle);
        }
    }
}