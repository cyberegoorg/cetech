using CETech.CEMath;
using CETech.Resource;
using CETech.World;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class SceneGraphApi
    {
        public static int GetNodeByName(int world, int entity, string name)
        {
            return SceneGraph.GetNodeByName(world, entity, StringId64.FromString(name));
        }

        public static void SetPosition(int world, int node, Vec3f pos)
        {
            SceneGraph.SetPosition(world, node, pos);
        }

        public static void SetRotation(int world, int node, Quatf rot)
        {
            SceneGraph.SetRotation(world, node, rot);
        }

        public static void SetScale(int world, int node, Vec3f scale)
        {
            SceneGraph.SetScale(world, node, scale);
        }

        public static Vec3f GetPosition(int world, int node)
        {
            return SceneGraph.GetPosition(world, node);
        }

        public static Quatf GetRotation(int world, int node)
        {
            return SceneGraph.GetRotation(world, node);
        }

        public static Vec3f GetScale(int world, int node)
        {
            return SceneGraph.GetScale(world, node);
        }

        public static Mat4f GetWorldMatrix(int world, int node)
        {
            return SceneGraph.GetWorldMatrix(world, node);
        }
    }
}