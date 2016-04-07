using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class RenderSystemApi
    {
        public static void RenderWorld(int world, int camera)
        {
            Renderer.RenderWorld(world, camera);
        }

        public static void SetDebug(bool enabled)
        {
            Renderer.SetDebug(enabled);
        }

        public static void SaveScreenShot(string filename)
        {
            Renderer.SaveScreenShot(filename);
        }

        public static void BeginCapture()
        {
            Renderer.BeginCapture();
        }


        public static void EndCapture()
        {
            Renderer.EndCapture();
        }
    }
}