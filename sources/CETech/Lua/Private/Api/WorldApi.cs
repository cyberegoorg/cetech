using CETech.CEMath;
using CETech.Resource;
using CETech.World;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class WorldApi
    {
        public static int Create()
        {
            return World.World.CreateWorld();
        }

        public static void Destroy(int world)
        {
            World.World.DestroyWorld(world);
        }

        public static void Update(int world, float dt)
        {
            World.World.Update(world, dt);
        }

        public static int LoadLevel(int world, string level, Vec3f? position, Vec3f? rotation, Vec3f? scale)
        {
            return World.World.LoadLevel(world, StringId.FromString(level),
                position != null ? position.Value : Vec3f.Zero,
                rotation != null ? rotation.Value : Vec3f.Zero,
                scale != null ? scale.Value : Vec3f.Unit);
        }

        public static int UnitByName(int world, int level, string name)
        {
            return World.World.UnitByName(world, level, StringId.FromString(name));
        }

        public static int LevelUnit(int world, int level)
        {
            return World.World.LevelUnit(world, level);
        }
    }
}