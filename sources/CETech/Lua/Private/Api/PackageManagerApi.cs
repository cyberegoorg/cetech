using CETech.Resource;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class PackageManagerApi
    {
        public static void Load(long name)
        {
            Package.Load(name);
        }

        public static void Unload(long name)
        {
            Package.Unload(name);
        }

        public static bool IsLoaded(long name)
        {
            return Package.IsLoaded(name);
        }

        public static void Flush(long name)
        {
            Package.Flush(name);
        }
    }
}