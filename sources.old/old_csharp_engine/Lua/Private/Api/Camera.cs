using CETech.World;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class CameraApi
    {
        public static int GetCamera(int world, int entity)
        {
            return CameraSystem.GetCamera(world, entity);
        }
    }
}