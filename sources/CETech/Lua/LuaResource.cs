using System.IO;
using CETech.Develop;

namespace CETech.Lua
{
    /// <summary>
    ///     Lua script resouce
    /// </summary>
    public class LuaResource
    {
        /// <summary>
        ///     Resource type
        /// </summary>
        public static readonly long Type = StringId.FromString("lua");

#if CETECH_DEVELOP
        /// <summary>
        ///     Resource compiler
        /// </summary>
        /// <param name="capi">Compiler api</param>
        public static void Compile(ResourceCompiler.CompilatorApi capi)
        {
            capi.ResourceFile.CopyTo(capi.BuildFile);

            // TODO: COMPILE!!!!
            //var script = new Script();
            //script.Dump(script.LoadStream(capi.ResourceFile), capi.BuildFile);
        }
#endif

        /// <summary>
        ///     Resource loader
        /// </summary>
        /// <param name="input">Resource data stream</param>
        /// <returns>Resource data</returns>
        public static object ResourceLoader(Stream input)
        {
            using (var ms = new MemoryStream())
            {
                input.CopyTo(ms);
                return ms.ToArray();
            }
        }

        /// <summary>
        ///     Resource offline.
        /// </summary>
        /// <param name="data">Data</param>
        public static void ResourceOffline(object data)
        {
        }

        /// <summary>
        ///     Resource online
        /// </summary>
        /// <param name="data">Data</param>
        public static void ResourceOnline(object data)
        {
        }

        /// <summary>
        ///     Resource unloader
        /// </summary>
        /// <param name="data">data</param>
        public static void ResourceUnloader(object data)
        {
        }
    }
}