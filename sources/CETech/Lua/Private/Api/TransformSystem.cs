using CETech.World;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class TransformSystemApi
    {
        public static int GetTransform(int world, int entity)
        {
            return TranformationSystem.GetTranform(world, entity);
        }

        public static void SetPosition(int world, int transform, Vector3f pos)
        {
            TranformationSystem.SetPosition(world, transform, pos);
        }

        public static void SetRotation(int world, int transform, Vector3f rot)
        {
            TranformationSystem.SetRotation(world, transform, rot);
        }

        public static void SetScale(int world, int transform, Vector3f scale)
        {
            TranformationSystem.SetScale(world, transform, scale);
        }

        public static Vector3f GetPosition(int world, int transform)
        {
            return TranformationSystem.GetPosition(world, transform);
        }

        public static Vector3f GetRotation(int world, int transform)
        {
            return TranformationSystem.GetRotation(world, transform);
        }

        public static Vector3f GetScale(int world, int transform)
        {
            return TranformationSystem.GetScale(world, transform);
        }

        public static Matrix4f GetWorldMatrix(int world, int transform)
        {
            return TranformationSystem.GetWorldMatrix(world, transform);
        }
    }
}