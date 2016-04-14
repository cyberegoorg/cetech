using System;
using MoonSharp.Interpreter;

namespace CETech.CEMath
{
    //TODO: move to lua
    [MoonSharpUserData]
    public struct Vec2f
    {
        public static readonly Vec2f Zero = new Vec2f();
        public static readonly Vec2f UnitX = new Vec2f(1.0f, 0.0f);
        public static readonly Vec2f UnitY = new Vec2f(0.0f, 1.0f);

        public float X, Y;

        public Vec2f(float x, float y)
        {
            X = x;
            Y = y;
        }

        public static float LengthSquared(Vec2f value)
        {
            return value.X* value.X + value.Y * value.Y;
        }

        public static float Length(Vec2f value)
        {
            return (float)Math.Sqrt(value.X * value.X + value.Y * value.Y);
        }

        public static Vec2f Normalize(Vec2f value)
        {       
            var inv_length = 1.0f/(float) Math.Sqrt(value.X*value.X + value.Y*value.Y);
            return new Vec2f(value.X*inv_length, value.Y*inv_length);
        }

        public static float Dot(Vec2f vector1, Vec2f vec2)
        {
            return vector1.X*vec2.X +
                   vector1.Y*vec2.Y;
        }

        public static Vec2f Cross(Vec2f vector1, Vec2f vec2)
        {
            throw new NotImplementedException();
        }

        public static Vec2f operator -(Vec2f left, Vec2f right)
        {
            return new Vec2f(left.X - right.X, left.Y - right.Y);
        }

        public static Vec2f operator -(Vec2f v)
        {
            return new Vec2f(-v.X, -v.Y);
        }

        public static Vec2f operator *(Vec2f left, Vec2f right)
        {
            return new Vec2f(left.X*right.X, left.Y*right.Y);
        }

        public static Vec2f operator *(Vec2f left, float right)
        {
            return new Vec2f(left.X*right, left.Y*right);
        }
    }
}