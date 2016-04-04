using System;

namespace CETech.World
{
    public struct Matrix4f
    {
        public static readonly Matrix4f Identity = new Matrix4f(
            1f, 0f, 0f, 0f,
            0f, 1f, 0f, 0f,
            0f, 0f, 1f, 0f,
            0f, 0f, 0f, 1f
            );

        public float M11, M12, M13, M14;
        public float M21, M22, M23, M24;
        public float M31, M32, M33, M34;
        public float M41, M42, M43, M44;

        public Matrix4f(
            float m11, float m12, float m13, float m14,
            float m21, float m22, float m23, float m24,
            float m31, float m32, float m33, float m34,
            float m41, float m42, float m43, float m44
            )
        {
            M11 = m11;
            M12 = m12;
            M13 = m13;
            M14 = m14;

            M21 = m21;
            M22 = m22;
            M23 = m23;
            M24 = m24;

            M31 = m31;
            M32 = m32;
            M33 = m33;
            M34 = m34;

            M41 = m41;
            M42 = m42;
            M43 = m43;
            M44 = m44;
        }

        public static Matrix4f CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio,
            float nearPlaneDistance, float farPlaneDistance)
        {
            var yScale = 1.0f/(float) Math.Tan(fieldOfView*0.5f);
            var xScale = yScale/aspectRatio;

            Matrix4f result;
            result.M11 = xScale;
            result.M12 = result.M13 = result.M14 = 0.0f;
            result.M22 = yScale;
            result.M21 = result.M23 = result.M24 = 0.0f;
            result.M31 = result.M32 = 0.0f;
            result.M33 = farPlaneDistance/(nearPlaneDistance - farPlaneDistance);
            result.M34 = -1.0f;
            result.M41 = result.M42 = result.M44 = 0.0f;
            result.M43 = nearPlaneDistance*farPlaneDistance/(nearPlaneDistance - farPlaneDistance);

            return result;
        }

        public static Matrix4f CreateLookAt(Vector3f cameraPosition, Vector3f cameraTarget, Vector3f cameraUpVector)
        {
            var zaxis = Vector3f.Normalize(cameraPosition - cameraTarget);
            var xaxis = Vector3f.Normalize(Vector3f.Cross(cameraUpVector, zaxis));
            var yaxis = Vector3f.Cross(zaxis, xaxis);

            var result = Identity;
            result.M11 = xaxis.X;
            result.M12 = yaxis.X;
            result.M13 = zaxis.X;
            result.M21 = xaxis.Y;
            result.M22 = yaxis.Y;
            result.M23 = zaxis.Y;
            result.M31 = xaxis.Z;
            result.M32 = yaxis.Z;
            result.M33 = zaxis.Z;
            result.M41 = -Vector3f.Dot(xaxis, cameraPosition);
            result.M42 = -Vector3f.Dot(yaxis, cameraPosition);
            result.M43 = -Vector3f.Dot(zaxis, cameraPosition);

            return result;
        }

        public static Matrix4f CreateScale(float scale)
        {
            return CreateScale(scale, scale, scale);
        }

        public static Matrix4f CreateScale(float xScale, float yScale, float zScale)
        {
            var result = Identity;
            result.M11 = xScale;
            result.M22 = yScale;
            result.M33 = zScale;

            return result;
        }

        public static Matrix4f CreateTranslation(float xPosition, float yPosition, float zPosition)
        {
            var result = Identity;
            result.M41 = xPosition;
            result.M42 = yPosition;
            result.M43 = zPosition;

            return result;
        }

        public static Matrix4f CreateRotationY(float radians)
        {
            var c = (float) Math.Cos(radians);
            var s = (float) Math.Sin(radians);

            var result = Identity;
            result.M11 = c;
            result.M13 = -s;
            result.M31 = s;
            result.M33 = c;

            return result;
        }

        public static Matrix4f CreateBillboard(Vector3f objectPosition, Vector3f cameraPosition, Vector3f cameraUpVector,
            Vector3f cameraForwardVector)
        {
            const float epsilon = 1e-4f;

            var zaxis = new Vector3f(
                objectPosition.X - cameraPosition.X,
                objectPosition.Y - cameraPosition.Y,
                objectPosition.Z - cameraPosition.Z);

            var norm = zaxis.LengthSquared();
            if (norm < epsilon)
                zaxis = -cameraForwardVector;
            else
                zaxis = zaxis*(1.0f/(float) Math.Sqrt(norm));

            var xaxis = Vector3f.Normalize(Vector3f.Cross(cameraUpVector, zaxis));
            var yaxis = Vector3f.Cross(zaxis, xaxis);

            var result = Identity;
            result.M11 = xaxis.X;
            result.M12 = xaxis.Y;
            result.M13 = xaxis.Z;
            result.M21 = yaxis.X;
            result.M22 = yaxis.Y;
            result.M23 = yaxis.Z;
            result.M31 = zaxis.X;
            result.M32 = zaxis.Y;
            result.M33 = zaxis.Z;
            result.M41 = objectPosition.X;
            result.M42 = objectPosition.Y;
            result.M43 = objectPosition.Z;

            return result;
        }

        public static Matrix4f CreateReflection(Vector3f normal, float d)
        {
            var scale = 1.0f/(float) Math.Sqrt(normal.LengthSquared());
            var a = normal.X*scale;
            var b = normal.Y*scale;
            var c = normal.Z*scale;
            d *= scale;

            var fa = -2.0f*a;
            var fb = -2.0f*b;
            var fc = -2.0f*c;

            Matrix4f result;

            result.M11 = fa*a + 1.0f;
            result.M12 = fb*a;
            result.M13 = fc*a;
            result.M14 = 0.0f;

            result.M21 = fa*b;
            result.M22 = fb*b + 1.0f;
            result.M23 = fc*b;
            result.M24 = 0.0f;

            result.M31 = fa*c;
            result.M32 = fb*c;
            result.M33 = fc*c + 1.0f;
            result.M34 = 0.0f;

            result.M41 = fa*d;
            result.M42 = fb*d;
            result.M43 = fc*d;
            result.M44 = 1.0f;

            return result;
        }

        public static Matrix4f CreateFromYawPitchRoll(float yaw, float pitch, float roll)
        {
            //  Roll first, about axis the object is facing, then
            //  pitch upward, then yaw to face into the new heading
            var halfRoll = roll*0.5f;
            var sr = (float) Math.Sin(halfRoll);
            var cr = (float) Math.Cos(halfRoll);

            var halfPitch = pitch*0.5f;
            var sp = (float) Math.Sin(halfPitch);
            var cp = (float) Math.Cos(halfPitch);

            var halfYaw = yaw*0.5f;
            var sy = (float) Math.Sin(halfYaw);
            var cy = (float) Math.Cos(halfYaw);

            var x = cy*sp*cr + sy*cp*sr;
            var y = sy*cp*cr - cy*sp*sr;
            var z = cy*cp*sr - sy*sp*cr;
            var w = cy*cp*cr + sy*sp*sr;

            var xx = x*x;
            var yy = y*y;
            var zz = z*z;

            var xy = x*y;
            var wz = z*w;
            var xz = z*x;
            var wy = y*w;
            var yz = y*z;
            var wx = x*w;

            var result = Identity;
            result.M11 = 1.0f - 2.0f*(yy + zz);
            result.M12 = 2.0f*(xy + wz);
            result.M13 = 2.0f*(xz - wy);
            result.M21 = 2.0f*(xy - wz);
            result.M22 = 1.0f - 2.0f*(zz + xx);
            result.M23 = 2.0f*(yz + wx);
            result.M31 = 2.0f*(xz + wy);
            result.M32 = 2.0f*(yz - wx);
            result.M33 = 1.0f - 2.0f*(yy + xx);

            return result;
        }

        public static Matrix4f operator *(Matrix4f value1, Matrix4f value2)
        {
            Matrix4f m;

            // First row
            m.M11 = value1.M11*value2.M11 + value1.M12*value2.M21 + value1.M13*value2.M31 + value1.M14*value2.M41;
            m.M12 = value1.M11*value2.M12 + value1.M12*value2.M22 + value1.M13*value2.M32 + value1.M14*value2.M42;
            m.M13 = value1.M11*value2.M13 + value1.M12*value2.M23 + value1.M13*value2.M33 + value1.M14*value2.M43;
            m.M14 = value1.M11*value2.M14 + value1.M12*value2.M24 + value1.M13*value2.M34 + value1.M14*value2.M44;

            // Second row
            m.M21 = value1.M21*value2.M11 + value1.M22*value2.M21 + value1.M23*value2.M31 + value1.M24*value2.M41;
            m.M22 = value1.M21*value2.M12 + value1.M22*value2.M22 + value1.M23*value2.M32 + value1.M24*value2.M42;
            m.M23 = value1.M21*value2.M13 + value1.M22*value2.M23 + value1.M23*value2.M33 + value1.M24*value2.M43;
            m.M24 = value1.M21*value2.M14 + value1.M22*value2.M24 + value1.M23*value2.M34 + value1.M24*value2.M44;

            // Third row
            m.M31 = value1.M31*value2.M11 + value1.M32*value2.M21 + value1.M33*value2.M31 + value1.M34*value2.M41;
            m.M32 = value1.M31*value2.M12 + value1.M32*value2.M22 + value1.M33*value2.M32 + value1.M34*value2.M42;
            m.M33 = value1.M31*value2.M13 + value1.M32*value2.M23 + value1.M33*value2.M33 + value1.M34*value2.M43;
            m.M34 = value1.M31*value2.M14 + value1.M32*value2.M24 + value1.M33*value2.M34 + value1.M34*value2.M44;

            // Fourth row
            m.M41 = value1.M41*value2.M11 + value1.M42*value2.M21 + value1.M43*value2.M31 + value1.M44*value2.M41;
            m.M42 = value1.M41*value2.M12 + value1.M42*value2.M22 + value1.M43*value2.M32 + value1.M44*value2.M42;
            m.M43 = value1.M41*value2.M13 + value1.M42*value2.M23 + value1.M43*value2.M33 + value1.M44*value2.M43;
            m.M44 = value1.M41*value2.M14 + value1.M42*value2.M24 + value1.M43*value2.M34 + value1.M44*value2.M44;

            return m;
        }
    }
}