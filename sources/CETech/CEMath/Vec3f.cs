using System;
using MoonSharp.Interpreter;

namespace CETech.CEMath
{
    [MoonSharpUserData]
    public struct Vec3f
    {
        public static readonly Vec3f Zero = new Vec3f();
        public static readonly Vec3f UnitX = new Vec3f(1.0f, 0.0f, 0.0f);
        public static readonly Vec3f UnitY = new Vec3f(0.0f, 1.0f, 0.0f);
        public static readonly Vec3f UnitZ = new Vec3f(0.0f, 0.0f, 1.0f);
        public static readonly Vec3f Unit = new Vec3f(1.0f, 1.0f, 1.0f);

        public float X, Y, Z;


        public Vec3f(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public static float LengthSquared(Vec3f value)
        {
            return value.X * value.X + value.Y * value.Y + value.Z * value.Z;
        }

        public static float Length(Vec3f value)
        {
            return (float)Math.Sqrt(value.X * value.X + value.Y * value.Y + value.Z * value.Z);
        }

        public static Vec3f Normalize(Vec3f value)
        {
            var length = 1.0f/(float) Math.Sqrt(value.X*value.X + value.Y*value.Y + value.Z*value.Z);
            return new Vec3f(value.X*length, value.Y*length, value.Z*length);
        }

        public static float Dot(Vec3f vector1, Vec3f vector2)
        {
            return vector1.X*vector2.X +
                   vector1.Y*vector2.Y +
                   vector1.Z*vector2.Z;
        }

        public static Vec3f Cross(Vec3f vector1, Vec3f vector2)
        {
            return new Vec3f(
                vector1.Y*vector2.Z - vector1.Z*vector2.Y,
                vector1.Z*vector2.X - vector1.X*vector2.Z,
                vector1.X*vector2.Y - vector1.Y*vector2.X);
        }

        public static Vec3f Transform(Vec3f position, Mat4f matrix)
        {
            return new Vec3f(
                position.X*matrix.M11 + position.Y*matrix.M21 + position.Z*matrix.M31 + matrix.M41,
                position.X*matrix.M12 + position.Y*matrix.M22 + position.Z*matrix.M32 + matrix.M42,
                position.X*matrix.M13 + position.Y*matrix.M23 + position.Z*matrix.M33 + matrix.M43);
        }

        public static Vec3f operator -(Vec3f left, Vec3f right)
        {
            return new Vec3f(left.X - right.X, left.Y - right.Y, left.Z - right.Z);
        }

        public static Vec3f operator -(Vec3f v)
        {
            return new Vec3f(-v.X, -v.Y, -v.Z);
        }

        public static Vec3f operator *(Vec3f left, Vec3f right)
        {
            return new Vec3f(left.X*right.X, left.Y*right.Y, left.Z*right.Z);
        }

        public static Vec3f operator *(Vec3f left, float right)
        {
            return new Vec3f(left.X*right, left.Y*right, left.Z*right);
        }
    }
}