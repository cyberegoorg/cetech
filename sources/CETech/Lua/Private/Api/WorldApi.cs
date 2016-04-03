using CETech.World;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class WorldApi
    {
        public static int Create()
        {
            return WorldManager.CreateWorld();
        }

        public static void Destroy(int world)
        {
            WorldManager.DestroyWorld(world);
        }

        public static void Update(int world)
        {
            WorldManager.Update(world);
        }

        public static int LoadLevel(int world, string level, Vector3f? position, Vector3f? rotation, Vector3f? scale)
        {
            return WorldManager.LoadLevel(world, StringId.FromString(level),
                position != null? position.Value : Vector3f.Zero,
                rotation != null ? rotation.Value : Vector3f.Zero,
                scale != null ? scale.Value : Vector3f.Zero);
        }

        public static int UnitByName(int world, int level, string name)
        {
            return WorldManager.UnitByName(world, level, StringId.FromString(name));
        }
    }
}