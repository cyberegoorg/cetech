using System;
using MoonSharp.Interpreter;

namespace CETech.World
{
    [MoonSharpUserData]
    public struct Vector2f
    {
        public static readonly Vector2f Zero = new Vector2f();
        public static readonly Vector2f UnitX = new Vector2f(1.0f, 0.0f);
        public static readonly Vector2f UnitY = new Vector2f(0.0f, 1.0f);

        public float X, Y;

        public Vector2f(float x, float y)
        {
            X = x;
            Y = y;
        }

        public float LengthSquared()
        {
            return X * X + Y * Y;
        }

        public static Vector2f Normalize(Vector2f value)
        {
            var inv_length = 1.0f / (float)Math.Sqrt(value.X * value.X + value.Y * value.Y);
            return new Vector2f(value.X * inv_length, value.Y / inv_length);
        }

        public static float Dot(Vector2f vector1, Vector2f vector2)
        {
            return vector1.X * vector2.X +
                   vector1.Y * vector2.Y;
        }

        public static Vector2f Cross(Vector2f vector1, Vector2f vector2)
        {
            throw new NotImplementedException();
        }

        public static Vector2f operator -(Vector2f left, Vector2f right)
        {
            return new Vector2f(left.X - right.X, left.Y - right.Y);
        }

        public static Vector2f operator -(Vector2f v)
        {
            return new Vector2f(-v.X, -v.Y);
        }

        public static Vector2f operator *(Vector2f left, Vector2f right)
        {
            return new Vector2f(left.X * right.X, left.Y * right.Y);
        }

        public static Vector2f operator *(Vector2f left, float right)
        {
            return new Vector2f(left.X * right, left.Y * right);
        }
    }
}