using CETech.Develop;
using CETech.Resource;
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

        public static void RecompileAll()
        {
#if CETECH_DEVELOP
            ResourceCompiler.CompileAll();
#endif
        }

        public static void ReloadAll()
        {
            var types = new[] {"shader", "texture", "material", "lua"};
            for (var i = 0; i < types.Length; i++)
            {
                ResourceManager.ReloadAll(StringId.FromString(types[i]));
            }
        }
    }
}