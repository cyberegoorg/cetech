using CETech.CEMath;
using CETech.World;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class TransformSystemApi
    {
        public static bool HasTransform(int world, int entity)
        {
            return Tranform.HasTransform(world, entity);
        }

        public static int GetTransform(int world, int entity)
        {
            return Tranform.GetTranform(world, entity);
        }

        public static void SetPosition(int world, int transform, Vec3f pos)
        {
            Tranform.SetPosition(world, transform, pos);
        }

        public static void SetRotation(int world, int transform, Quatf rot)
        {
            Tranform.SetRotation(world, transform, rot);
        }

        public static void SetScale(int world, int transform, Vec3f scale)
        {
            Tranform.SetScale(world, transform, scale);
        }

        public static Vec3f GetPosition(int world, int transform)
        {
            return Tranform.GetPosition(world, transform);
        }

        public static Quatf GetRotation(int world, int transform)
        {
            return Tranform.GetRotation(world, transform);
        }

        public static Vec3f GetScale(int world, int transform)
        {
            return Tranform.GetScale(world, transform);
        }

        public static Mat4f GetWorldMatrix(int world, int transform)
        {
            return Tranform.GetWorldMatrix(world, transform);
        }
    }
}