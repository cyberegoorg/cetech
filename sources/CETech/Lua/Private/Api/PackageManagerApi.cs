using CETech.Resource;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class PackageManagerApi
    {
        public static void Load(long name)
        {
            PackageManager.Load(name);
        }

        public static void Unload(long name)
        {
            PackageManager.Unload(name);
        }

        public static bool IsLoaded(long name)
        {
            return PackageManager.IsLoaded(name);
        }

        public static void Flush(long name)
        {
            PackageManager.Flush(name);
        }
    }
}