using System;
using MoonSharp.Interpreter;

namespace CETech.CEMath
{
    [MoonSharpUserData]
    public struct Vec4f
    {
        public static readonly Vec4f Zero = new Vec4f();
        public static readonly Vec4f UnitX = new Vec4f(1.0f, 0.0f, 0.0f, 0.0f);
        public static readonly Vec4f UnitY = new Vec4f(0.0f, 1.0f, 0.0f, 0.0f);
        public static readonly Vec4f UnitZ = new Vec4f(0.0f, 0.0f, 1.0f, 0.0f);
        public static readonly Vec4f UnitW = new Vec4f(0.0f, 0.0f, 0.0f, 1.0f);

        public float X, Y, Z, W;

        public Vec4f(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public static float LengthSquared(Vec4f value)
        {
            return value.X*value.X + value.Y*value.Y + value.Z*value.Z + value.W*value.W;
        }

        public static float Length(Vec4f value)
        {
            return (float) Math.Sqrt(value.X*value.X + value.Y*value.Y + value.Z*value.Z + value.W*value.W);
        }

        public static Vec4f Normalize(Vec4f value)
        {
            var length = 1.0f/(float) Math.Sqrt(value.X*value.X + value.Y*value.Y + value.Z*value.Z + value.W*value.W);
            return new Vec4f(value.X*length, value.Y*length, value.Z*length, value.W*length);
        }

        public static float Dot(Vec4f vector1, Vec4f vector2)
        {
            return vector1.X*vector2.X +
                   vector1.Y*vector2.Y +
                   vector1.Z*vector2.Z +
                   vector1.W*vector2.W;
        }

        public static Vec4f operator -(Vec4f left, Vec4f right)
        {
            return new Vec4f(left.X - right.X, left.Y - right.Y, left.Z - right.Z, left.W - right.W);
        }

        public static Vec4f operator -(Vec4f v)
        {
            return new Vec4f(-v.X, -v.Y, -v.Z, -v.W);
        }

        public static Vec4f operator *(Vec4f left, Vec4f right)
        {
            return new Vec4f(left.X*right.X, left.Y*right.Y, left.Z*right.Z, left.W*right.W);
        }

        public static Vec4f operator *(Vec4f left, float right)
        {
            return new Vec4f(left.X*right, left.Y*right, left.Z*right, left.W*right);
        }
    }
}