using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class UnitManagerApi
    {
        public static int Spawn(int world, string unit)
        {
            return UnitManager.Spawn(world, StringId.FromString(unit));
        }

        public static void Destroy(int world, int unit)
        {
            UnitManager.Destroy(world, unit);
        }
    }
}