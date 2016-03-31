using System.Collections.Generic;
using System.IO;
using System.Linq;
using CETech.Develop;
using CETech.Lua;
using CETech.World;
using MsgPack.Serialization;
using YamlDotNet.RepresentationModel;

namespace CETech
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
            var yaml = new YamlStream();
            yaml.Load(input);

            var rootNode = yaml.Documents[0].RootNode as YamlMappingNode;

            var pack = new Resource();
            pack.Type = new long[rootNode.Children.Count];
            pack.Names = new long[rootNode.Children.Count][];

            Dictionary<long, int> prioritDictionary = new Dictionary<long, int>();
            prioritDictionary[PackageResource.Type] = 0;
            prioritDictionary[ShaderResource.Type] = 0;
            prioritDictionary[LuaResource.Type] = 1;
            prioritDictionary[ConfigResource.Type] = 2;
            prioritDictionary[UnitResource.Type] = 3;
            prioritDictionary[ShaderResource.Type] = 4;
            prioritDictionary[MaterialResource.Type] = 5;
            prioritDictionary[StringId.FromString("texture")] = 6;

            Dictionary<long, YamlSequenceNode> types_nodes = new Dictionary<long, YamlSequenceNode>();
            foreach (var type in rootNode.Children)
            {
                var typestr = type.Key as YamlScalarNode;
                var sequence = type.Value as YamlSequenceNode;

                var typeid = StringId.FromString(typestr.Value);
                types_nodes.Add(typeid, sequence);
            }

            var idx = 0;
            var nodes = types_nodes.OrderBy(pair => prioritDictionary[pair.Key]).Select(pair => pair.Key).ToArray();
            foreach (var node in nodes)
            {
                var sequence = types_nodes[node];
                pack.Type[idx] = node;
                pack.Names[idx] = new long[sequence.Children.Count];

                var name_idx = 0;
                foreach (var name in sequence.Children)
                {
                    var nameid = StringId.FromString(((YamlScalarNode)name).Value);

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

        /// <summary>
        ///     Package resouce format
        /// </summary>
        public struct Resource
        {
            /// <summary>
            ///     Types
            /// </summary>
            public long[] Type;

            /// <summary>
            ///     [type][name, name, ...]
            /// </summary>
            public long[][] Names;
        }
    }
}