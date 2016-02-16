using System.IO;

namespace CETech.Lua
{
    public class LuaResource
    {
        public static readonly long Type = StringId.FromString("lua");

        public static void Compile(ResourceCompiler.CompilatorApi capi)
        {
            capi.ResourceFile.CopyTo(capi.BuildFile);

            // TODO: COMPILE!!!!
            //var script = new Script();
            //script.Dump(script.LoadStream(capi.ResourceFile), capi.BuildFile);
        }

        public static object ResourceLoader(Stream input)
        {
            using (var ms = new MemoryStream())
            {
                input.CopyTo(ms);
                return ms.ToArray();
            }
        }

        public static void ResourceOffline(object data)
        {
        }

        public static void ResourceOnline(object data)
        {
        }

        public static void ResourceUnloader(object data)
        {
        }
    }
}