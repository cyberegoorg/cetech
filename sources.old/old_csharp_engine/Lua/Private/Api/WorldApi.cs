using CETech.CEMath;
using CETech.Resource;
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

        public static int LoadLevel(int world, string level, Vec3f? position, Quatf? rotation, Vec3f? scale)
        {
            return World.World.LoadLevel(
                world,
                StringId64.FromString(level),
                position ?? Vec3f.Zero,
                rotation ?? Quatf.Identity,
                scale ?? Vec3f.Unit);
        }

        public static int UnitByName(int world, int level, string name)
        {
            return World.World.UnitByName(world, level, StringId64.FromString(name));
        }

        public static int LevelUnit(int world, int level)
        {
            return World.World.LevelUnit(world, level);
        }
    }
}