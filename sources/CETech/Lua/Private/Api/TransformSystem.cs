using CETech.World;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class TransformSystemApi
    {
        public static void SetPosition(int world, int entity, Vector3f pos)
        {
            TranformationSystem.SetPosition(world, entity, pos);
        }

        public static void SetRotation(int world, int entity, Vector3f rot)
        {
            TranformationSystem.SetRotation(world, entity, rot);
        }

        public static void SetScale(int world, int entity, Vector3f scale)
        {
            TranformationSystem.SetScale(world, entity, scale);
        }

        public static Vector3f GetPosition(int world, int entity)
        {
            return TranformationSystem.GetPosition(world, entity);
        }

        public static Vector3f GetRotation(int world, int entity)
        {
            return TranformationSystem.GetRotation(world, entity);
        }

        public static Vector3f GetScale(int world, int entity)
        {
            return TranformationSystem.GetScale(world, entity);
        }

        public static Matrix4f GetWorldMatrix(int world, int entity)
        {
            return TranformationSystem.GetWorldMatrix(world, entity);
        }
    }
}