using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class RenderSystemApi
    {
        public static void RenderWorld(int world)
        {
            RenderSystem.RenderWorld(world);
        }
    }
}