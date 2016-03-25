using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class ApplicationApi
    {
        public static void Quit()
        {
            Application.Quit();
        }

        public static string GetPlatform()
        {
            return Application.GetPlatform();
        }
    }
}