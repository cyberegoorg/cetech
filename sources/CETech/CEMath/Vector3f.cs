using System;
using MoonSharp.Interpreter;

namespace CETech.CEMath
{
    [MoonSharpUserData]
    public struct Vector3f
    {
        public static readonly Vector3f Zero = new Vector3f();
        public static readonly Vector3f UnitX = new Vector3f(1.0f, 0.0f, 0.0f);
        public static readonly Vector3f UnitY = new Vector3f(0.0f, 1.0f, 0.0f);
        public static readonly Vector3f UnitZ = new Vector3f(0.0f, 0.0f, 1.0f);
        public static readonly Vector3f Unit = new Vector3f(1.0f, 1.0f, 1.0f);

        public float X, Y, Z;


        public Vector3f(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public float LengthSquared()
        {
            return X*X + Y*Y + Z*Z;
        }

        public static Vector3f Normalize(Vector3f value)
        {
            var length = 1.0f/(float) Math.Sqrt(value.X*value.X + value.Y*value.Y + value.Z*value.Z);
            return new Vector3f(value.X*length, value.Y*length, value.Z*length);
        }

        public static float Dot(Vector3f vector1, Vector3f vector2)
        {
            return vector1.X*vector2.X +
                   vector1.Y*vector2.Y +
                   vector1.Z*vector2.Z;
        }

        public static Vector3f Cross(Vector3f vector1, Vector3f vector2)
        {
            return new Vector3f(
                vector1.Y*vector2.Z - vector1.Z*vector2.Y,
                vector1.Z*vector2.X - vector1.X*vector2.Z,
                vector1.X*vector2.Y - vector1.Y*vector2.X);
        }

        public static Vector3f Transform(Vector3f position, Matrix4f matrix)
        {
            return new Vector3f(
                position.X*matrix.M11 + position.Y*matrix.M21 + position.Z*matrix.M31 + matrix.M41,
                position.X*matrix.M12 + position.Y*matrix.M22 + position.Z*matrix.M32 + matrix.M42,
                position.X*matrix.M13 + position.Y*matrix.M23 + position.Z*matrix.M33 + matrix.M43);
        }

        public static Vector3f operator -(Vector3f left, Vector3f right)
        {
            return new Vector3f(left.X - right.X, left.Y - right.Y, left.Z - right.Z);
        }

        public static Vector3f operator -(Vector3f v)
        {
            return new Vector3f(-v.X, -v.Y, -v.Z);
        }

        public static Vector3f operator *(Vector3f left, Vector3f right)
        {
            return new Vector3f(left.X*right.X, left.Y*right.Y, left.Z*right.Z);
        }

        public static Vector3f operator *(Vector3f left, float right)
        {
            return new Vector3f(left.X*right, left.Y*right, left.Z*right);
        }
    }
}