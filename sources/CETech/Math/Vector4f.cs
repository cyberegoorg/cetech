using System;
using MoonSharp.Interpreter;

namespace CETech.World
{
    [MoonSharpUserData]
    public struct Vector4f
    {
        public static readonly Vector4f Zero = new Vector4f();
        public static readonly Vector4f UnitX = new Vector4f(1.0f, 0.0f, 0.0f, 0.0f);
        public static readonly Vector4f UnitY = new Vector4f(0.0f, 1.0f, 0.0f, 0.0f);
        public static readonly Vector4f UnitZ = new Vector4f(0.0f, 0.0f, 1.0f, 0.0f);

        public float X, Y, Z, W;

        public Vector4f(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public float LengthSquared()
        {
            return X*X + Y*Y + Z*Z + W*W;
        }

        public static Vector4f Normalize(Vector4f value)
        {
            var length = 1.0f/(float) Math.Sqrt(value.X*value.X + value.Y*value.Y + value.Z*value.Z + value.W*value.W);
            return new Vector4f(value.X*length, value.Y*length, value.Z*length, value.W*length);
        }

        public static float Dot(Vector4f vector1, Vector4f vector2)
        {
            return vector1.X*vector2.X +
                   vector1.Y*vector2.Y +
                   vector1.Z*vector2.Z +
                   vector1.W*vector2.W;
        }

        public static Vector4f operator -(Vector4f left, Vector4f right)
        {
            return new Vector4f(left.X - right.X, left.Y - right.Y, left.Z - right.Z, left.W - right.W);
        }

        public static Vector4f operator -(Vector4f v)
        {
            return new Vector4f(-v.X, -v.Y, -v.Z, -v.W);
        }

        public static Vector4f operator *(Vector4f left, Vector4f right)
        {
            return new Vector4f(left.X*right.X, left.Y*right.Y, left.Z*right.Z, left.W*right.W);
        }

        public static Vector4f operator *(Vector4f left, float right)
        {
            return new Vector4f(left.X*right, left.Y*right, left.Z*right, left.W*right);
        }
    }
}