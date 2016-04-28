using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Yaml;
using CETech.Develop;
using CETech.Lua;
using CETech.World;
using MsgPack.Serialization;

namespace CETech.Resource
{
    /// <summary>
    ///     Package resource
    /// </summary>
    public class PackageResource
    {
        /// <summary>
        ///     Resource type
        /// </summary>
        public static readonly long Type = StringId.FromString("package");

#if CETECH_DEVELOP

        /// <summary>
        ///     Resource compiler
        /// </summary>
        /// <param name="capi">Compiler api</param>
        public static void Compile(ResourceCompiler.CompilatorApi capi)
        {
            TextReader input = new StreamReader(capi.ResourceFile);
            var yaml = YamlNode.FromYaml(input, new YamlConfig());

            var rootNode = yaml[0] as YamlMapping;

            var pack = new Resource();
            pack.Type = new long[rootNode.Count];
            pack.Names = new long[rootNode.Count][];

            // TODO: generic
            var prioritDictionary = new Dictionary<long, int>();
            prioritDictionary[Type] = 0;
            prioritDictionary[RenderConfig.Type] = 0;
            prioritDictionary[LuaResource.Type] = 2;
            prioritDictionary[ConfigResource.Type] = 3;
            prioritDictionary[TextureResource.Type] = 4;
            prioritDictionary[ShaderResource.Type] = 5;
            prioritDictionary[MaterialResource.Type] = 6;
            prioritDictionary[UnitResource.Type] = 7;
            prioritDictionary[SceneResource.Type] = 8;
            prioritDictionary[LevelResource.Type] = 9;

            var types_nodes = new Dictionary<long, YamlSequence>();
            foreach (var type in rootNode)
            {
                var typestr = type.Key as YamlScalar;
                var sequence = type.Value as YamlSequence;

                var typeid = StringId.FromString(typestr.Value);
                types_nodes.Add(typeid, sequence);
            }

            var idx = 0;
            var nodes = types_nodes.OrderBy(pair => prioritDictionary[pair.Key]).Select(pair => pair.Key).ToArray();
            foreach (var node in nodes)
            {
                var sequence = types_nodes[node];
                pack.Type[idx] = node;
                pack.Names[idx] = new long[sequence.Count];

                var name_idx = 0;
                foreach (var name in sequence)
                {
                    var nameid = StringId.FromString(((YamlScalar) name).Value);

                    pack.Names[idx][name_idx] = nameid;
                    ++name_idx;
                }

                ++idx;
            }

            var serializer = MessagePackSerializer.Get<Resource>();
            serializer.Pack(capi.BuildFile, pack);
        }
#endif

        /// <summary>
        ///     Resource loader
        /// </summary>
        /// <param name="input">Resource data stream</param>
        /// <returns>Resource data</returns>
        public static object ResourceLoader(Stream input)
        {
            var serializer = MessagePackSerializer.Get<Resource>();
            return serializer.Unpack(input);
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

        public static object Reloader(long name, object new_data)
        {
            return new_data;
        }

        /// <summary>
        ///     Package resouce format
        /// </summary>
        public class Resource
        {
            /// <summary>
            ///     [type][name, name, ...]
            /// </summary>
            public long[][] Names;

            /// <summary>
            ///     Types
            /// </summary>
            public long[] Type;
        }
    }
}