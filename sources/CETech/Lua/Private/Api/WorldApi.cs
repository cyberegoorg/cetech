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
    }
}