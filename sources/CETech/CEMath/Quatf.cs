using MoonSharp.Interpreter;

namespace CETech.CEMath
{
    [MoonSharpUserData]
    public struct Quatf
    {
        public static readonly Quatf Zero = new Quatf();

        public float X, Y, Z, W;

        public Quatf(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }

        public static Mat4f ToMat4F(Quatf q)
        {
            var res = Mat4f.Identity;

            var sqw = q.W*q.W;
            var sqx = q.X*q.X;
            var sqy = q.Y*q.Y;
            var sqz = q.Z*q.Z;

            var invs = 1.0f/(sqx + sqy + sqz + sqw);
            res.M11 = (sqx - sqy - sqz + sqw)*invs;
            res.M22 = (-sqx + sqy - sqz + sqw)*invs;
            res.M33 = (-sqx - sqy + sqz + sqw)*invs;

            var tmp1 = q.X*q.Y;
            var tmp2 = q.Z*q.W;
            res.M21 = 2.0f*(tmp1 + tmp2)*invs;
            res.M12 = 2.0f*(tmp1 - tmp2)*invs;

            tmp1 = q.X*q.Z;
            tmp2 = q.Y*q.W;
            res.M31 = 2.0f*(tmp1 - tmp2)*invs;
            res.M13 = 2.0f*(tmp1 + tmp2)*invs;

            tmp1 = q.Y*q.Z;
            tmp2 = q.X*q.W;
            res.M32 = 2.0f*(tmp1 + tmp2)*invs;
            res.M23 = 2.0f*(tmp1 - tmp2)*invs;

            return res;
        }

        public static Quatf FromAxisAngle(Vec3f axis, float angle_deg)
        {
            var angle_deg_half = angle_deg*0.5f;
            var sin = MathUtils.Sinf(angle_deg_half);
            var cos = MathUtils.Cosf(angle_deg_half);

            return new Quatf(
                sin*axis.X,
                sin*axis.Y,
                sin*axis.Z,
                cos
                );
        }

        public static Quatf FromEurelAngle(float heading, float attitude, float bank)
        {
            var c1 = MathUtils.Cosf(heading/2);
            var s1 = MathUtils.Sinf(heading/2);
            var c2 = MathUtils.Cosf(attitude/2);
            var s2 = MathUtils.Sinf(attitude/2);
            var c3 = MathUtils.Cosf(bank/2);
            var s3 = MathUtils.Sinf(bank/2);
            var c1c2 = c1*c2;
            var s1s2 = s1*s2;

            return new Quatf(
                c1c2*s3 + s1s2*c3,
                s1*c2*c3 + c1*s2*s3,
                c1*s2*c3 - s1*c2*s3,
                c1c2*c3 - s1s2*s3
                );
        }

        public static float Length(Quatf q1)
        {
            return MathUtils.FastSqrtf(LengthSquared(q1));
        }

        public static float LengthSquared(Quatf q1)
        {
            return q1.X*q1.X + q1.Y*q1.Y + q1.Z*q1.Z + q1.W*q1.W;
        }

        public static float InvLength(Quatf q1)
        {
            return MathUtils.FastInvSqrtf(LengthSquared(q1));
        }

        public static Quatf Normalize(Quatf q1)
        {
            return q1*InvLength(q1);
        }

        public static bool IsNormalized(Quatf v1)
        {
            return MathUtils.abs(1.0f - LengthSquared(v1)) < 0.001f;
        }

        public static Quatf operator -(Quatf q1)
        {
            //CE_ASSERT("quaternion", quat::IsNormalized(q1));

            return new Quatf(
                -q1.X,
                -q1.Y,
                -q1.Z,
                q1.W
                );
        }

        public static Quatf operator +(Quatf q1, Quatf q2)
        {
            return new Quatf(
                q1.X + q2.X,
                q1.Y + q2.Y,
                q1.Z + q2.Z,
                q1.W + q2.W
                );
        }

        public static Quatf operator -(Quatf q1, Quatf q2)
        {
            return new Quatf(
                q1.X - q2.X,
                q1.Y - q2.Y,
                q1.Z - q2.Z,
                q1.W - q2.W
                );
        }

        public static Quatf operator *(Quatf q1, Quatf q2)
        {
            return new Quatf(
                q1.X*q2.X - q1.Y*q2.Y - q1.Z*q2.Z - q1.W*q2.W,
                q1.X*q2.Y + q1.Y*q2.X + q1.Z*q2.W - q1.W*q2.Z,
                q1.X*q2.Z - q1.Y*q2.W + q1.Z*q2.X + q1.W*q2.Y,
                q1.X*q2.W + q1.Y*q2.Z - q1.Z*q2.Y + q1.W*q2.X
                );
        }

        public static Quatf operator *(Quatf q1, float s)
        {
            return new Quatf(
                q1.X*s,
                q1.Y*s,
                q1.Z*s,
                q1.W*s
                );
        }

        public static Vec3f operator *(Quatf q1, Vec3f v1)
        {
            var qv = new Vec3f(q1.X, q1.Y, q1.Z);

            var res = Vec3f.Cross(qv, v1)*(2.0f*q1.W);
            res = res + v1*(MathUtils.square(q1.W) - Vec3f.Dot(qv, qv));
            res = res + qv*(2.0f*Vec3f.Dot(qv, v1));

            return res;
        }

        public static Quatf operator /(Quatf q1, float s)
        {
            var inv_s = 1.0f/s;

            return new Quatf(
                q1.X*inv_s,
                q1.Y*inv_s,
                q1.Z*inv_s,
                q1.W*inv_s
                );
        }
    }
}