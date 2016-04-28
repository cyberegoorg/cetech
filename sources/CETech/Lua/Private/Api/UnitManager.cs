using CETech.Resource;
using CETech.World;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class UnitManagerApi
    {
        public static int Spawn(int world, string unit)
        {
            return Unit.Spawn(world, StringId64.FromString(unit));
        }

        public static void Destroy(int world, int unit)
        {
            Unit.Destroy(world, unit);
        }
    }
}