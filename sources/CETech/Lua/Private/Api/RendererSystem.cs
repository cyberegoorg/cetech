using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class RenderSystemApi
    {
        public static void RenderWorld(int world, int camera)
        {
            RenderSystem.RenderWorld(world, camera);
        }

        public static void SetDebug(bool enabled)
        {
            RenderSystem.SetDebug(enabled);
        }

        public static void SaveScreenShot(string filename)
        {
            RenderSystem.SaveScreenShot(filename);
        }

        public static void BeginCapture()
        {
            RenderSystem.BeginCapture();
        }


        public static void EndCapture()
        {
            RenderSystem.EndCapture();
        }
    }
}