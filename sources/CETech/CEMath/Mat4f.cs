using System;
using MoonSharp.Interpreter;
using System.Diagnostics;

namespace CETech.CEMath
{
    [MoonSharpUserData]
    public struct Mat4f
    {
        public static readonly Mat4f Identity = new Mat4f(
            1f, 0f, 0f, 0f,
            0f, 1f, 0f, 0f,
            0f, 0f, 1f, 0f,
            0f, 0f, 0f, 1f
            );

        public float M11, M12, M13, M14;
        public float M21, M22, M23, M24;
        public float M31, M32, M33, M34;
        public float M41, M42, M43, M44;

        public Mat4f(
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

        public static Mat4f CreatePerspectiveFieldOfView(float fieldOfView, float aspectRatio,
            float nearPlaneDistance, float farPlaneDistance)
        {
            var yScale = 1.0f/(float) Math.Tan(fieldOfView*0.5f);
            var xScale = yScale/aspectRatio;

            Mat4f result;
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

        public static Mat4f CreateLookAt(Vec3f cameraPosition, Vec3f cameraTarget, Vec3f cameraUpVector)
        {
            var zaxis = Vec3f.Normalize(cameraPosition - cameraTarget);
            var xaxis = Vec3f.Normalize(Vec3f.Cross(cameraUpVector, zaxis));
            var yaxis = Vec3f.Cross(zaxis, xaxis);

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

            result.M41 = -Vec3f.Dot(xaxis, cameraPosition);
            result.M42 = -Vec3f.Dot(yaxis, cameraPosition);
            result.M43 = -Vec3f.Dot(zaxis, cameraPosition);

            return result;
        }

        public static Mat4f CreateScale(float scale)
        {
            return CreateScale(scale, scale, scale);
        }

        public static Mat4f CreateScale(float xScale, float yScale, float zScale)
        {
            var result = Identity;
            result.M11 = xScale;
            result.M22 = yScale;
            result.M33 = zScale;

            return result;
        }

        public static Mat4f CreateTranslation(float xPosition, float yPosition, float zPosition)
        {
            var result = Identity;
            result.M41 = xPosition;
            result.M42 = yPosition;
            result.M43 = zPosition;

            return result;
        }

        public static Mat4f CreateRotationY(float radians)
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

        public static Mat4f CreateBillboard(Vec3f objectPosition, Vec3f cameraPosition, Vec3f cameraUpVector,
            Vec3f cameraForwardVector)
        {
            const float epsilon = 1e-4f;

            var zaxis = new Vec3f(
                objectPosition.X - cameraPosition.X,
                objectPosition.Y - cameraPosition.Y,
                objectPosition.Z - cameraPosition.Z);

            var norm = Vec3f.LengthSquared(zaxis);
            if (norm < epsilon)
                zaxis = -cameraForwardVector;
            else
                zaxis = zaxis*(1.0f/(float) Math.Sqrt(norm));

            var xaxis = Vec3f.Normalize(Vec3f.Cross(cameraUpVector, zaxis));
            var yaxis = Vec3f.Cross(zaxis, xaxis);

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

        public static Mat4f CreateReflection(Vec3f normal, float d)
        {
            var scale = 1.0f/(float) Math.Sqrt(Vec3f.LengthSquared(normal));
            var a = normal.X*scale;
            var b = normal.Y*scale;
            var c = normal.Z*scale;
            d *= scale;

            var fa = -2.0f*a;
            var fb = -2.0f*b;
            var fc = -2.0f*c;

            var result = Identity;

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

        public static Mat4f CreateFromYawPitchRoll(float yaw, float pitch, float roll)
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

        public static Vec3f X(Mat4f m)
        {
            return new Vec3f(m.M11, m.M12, m.M13);
        }

        public static Vec3f Y(Mat4f m)
        {
            return new Vec3f(m.M21, m.M22, m.M23);
        }

        public static Vec3f Z(Mat4f m)
        {
            return new Vec3f(m.M31, m.M32, m.M33);
        }

		public static Vec3f Translation(Mat4f m) {
			return new Vec3f (m.M41, m.M42, m.M43);
		}

		public static Quatf ToQuat(Mat4f m) {
			var x = X (m);
			var y = Y (m);
			var z = Z (m);

			float ww = x.X + y.Y + z.Z;
			float xx = x.X - y.Y - z.Z;
			float yy = y.Y - x.X - z.Z;
			float zz = z.Z - x.X - y.Y;

			float max = ww;
			int index = 0;

			if (xx > max) {
				max = xx;
				index = 1;
			}

			if (yy > max) {
				max = yy;
				index = 2;
			}

			if (zz > max) {
				max = zz;
				index = 3;
			}

			float biggest = Mathf.FastSqrtf(max + 1.0f) * 0.5f;
			float mult = 0.25f / biggest;

			Quatf tmp = Quatf.Identity;
			switch (index) {
			case 0:
				{
					tmp.W = biggest;
					tmp.X = (y.Z - z.Y) * mult;
					tmp.Y = (z.X - x.Z) * mult;
					tmp.Z = (x.Y - y.X) * mult;
					break;
				}
			case 1:
				{
					tmp.X = biggest;
					tmp.W = (y.Z - z.Y) * mult;
					tmp.Y = (x.Y + y.X) * mult;
					tmp.Z = (z.X + x.Z) * mult;
					break;
				}
			case 2:
				{
					tmp.Y = biggest;
					tmp.W = (z.X - x.Z) * mult;
					tmp.X = (x.Y + y.X) * mult;
					tmp.Z = (y.Z + z.Y) * mult;
					break;
				}
			case 3:
				{
					tmp.Z = biggest;
					tmp.W = (x.Y - y.X) * mult;
					tmp.X = (z.X + x.Z) * mult;
					tmp.Y = (y.Z + z.Y) * mult;
					break;
				}
			default:
				{
					Debug.Assert (false);
					break;
				}
			}

			return Quatf.Normalize (tmp);
		}

        public static float GetDeterminant(Mat4f m)
        {
            var xx = m.M11;
            var xy = m.M12;
            var xz = m.M13;
            var xw = m.M14;

            var yx = m.M21;
            var yy = m.M22;
            var yz = m.M23;
            var yw = m.M24;

            var zx = m.M31;
            var zy = m.M32;
            var zz = m.M33;
            var zw = m.M34;

            var tx = m.M41;
            var ty = m.M42;
            var tz = m.M43;
            var tw = m.M44;

            var det = 0.0f;
            det += +xx*(yy*(zz*tw - tz*zw) - zy*(yz*tw - tz*yw) + ty*(yz*zw - zz*yw));
            det += -yx*(xy*(zz*tw - tz*zw) - zy*(xz*tw - tz*xw) + ty*(xz*zw - zz*xw));
            det += +zx*(xy*(yz*tw - tz*yw) - yy*(xz*tw - tz*xw) + ty*(xz*yw - yz*xw));
            det += -tx*(xy*(yz*zw - zz*yw) - yy*(xz*zw - zz*xw) + zy*(xz*yw - yz*xw));

            return det;
        }

        public static Mat4f Inverted(Mat4f m)
        {
            var xx = m.M11;
            var xy = m.M12;
            var xz = m.M13;
            var xw = m.M14;

            var yx = m.M21;
            var yy = m.M22;
            var yz = m.M23;
            var yw = m.M24;

            var zx = m.M31;
            var zy = m.M32;
            var zz = m.M33;
            var zw = m.M34;

            var tx = m.M41;
            var ty = m.M42;
            var tz = m.M43;
            var tw = m.M44;

            var det = GetDeterminant(m);
            var inv_det = 1.0f/det;

            m.M11 = +(yy*(zz*tw - tz*zw) - zy*(yz*tw - tz*yw) + ty*(yz*zw - zz*yw))*inv_det;
            m.M12 = -(xy*(zz*tw - tz*zw) - zy*(xz*tw - tz*xw) + ty*(xz*zw - zz*xw))*inv_det;
            m.M13 = +(xy*(yz*tw - tz*yw) - yy*(xz*tw - tz*xw) + ty*(xz*yw - yz*xw))*inv_det;
            m.M14 = -(xy*(yz*zw - zz*yw) - yy*(xz*zw - zz*xw) + zy*(xz*yw - yz*xw))*inv_det;

            m.M21 = -(yx*(zz*tw - tz*zw) - zx*(yz*tw - tz*yw) + tx*(yz*zw - zz*yw))*inv_det;
            m.M22 = +(xx*(zz*tw - tz*zw) - zx*(xz*tw - tz*xw) + tx*(xz*zw - zz*xw))*inv_det;
            m.M23 = -(xx*(yz*tw - tz*yw) - yx*(xz*tw - tz*xw) + tx*(xz*yw - yz*xw))*inv_det;
            m.M24 = +(xx*(yz*zw - zz*yw) - yx*(xz*zw - zz*xw) + zx*(xz*yw - yz*xw))*inv_det;

            m.M31 = +(yx*(zy*tw - ty*zw) - zx*(yy*tw - ty*yw) + tx*(yy*zw - zy*yw))*inv_det;
            m.M32 = -(xx*(zy*tw - ty*zw) - zx*(xy*tw - ty*xw) + tx*(xy*zw - zy*xw))*inv_det;
            m.M33 = +(xx*(yy*tw - ty*yw) - yx*(xy*tw - ty*xw) + tx*(xy*yw - yy*xw))*inv_det;
            m.M34 = -(xx*(yy*zw - zy*yw) - yx*(xy*zw - zy*xw) + zx*(xy*yw - yy*xw))*inv_det;

            m.M41 = -(yx*(zy*tz - ty*zz) - zx*(yy*tz - ty*yz) + tx*(yy*zz - zy*yz))*inv_det;
            m.M42 = +(xx*(zy*tz - ty*zz) - zx*(xy*tz - ty*xz) + tx*(xy*zz - zy*xz))*inv_det;
            m.M43 = -(xx*(yy*tz - ty*yz) - yx*(xy*tz - ty*xz) + tx*(xy*yz - yy*xz))*inv_det;
            m.M44 = +(xx*(yy*zz - zy*yz) - yx*(xy*zz - zy*xz) + zx*(xy*yz - yy*xz))*inv_det;

            return m;
        }

        public static Mat4f operator *(Mat4f value1, Mat4f value2)
        {
            var m = Identity;

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