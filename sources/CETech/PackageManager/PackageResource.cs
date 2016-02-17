using System.IO;
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

            var idx = 0;
            foreach (var type in rootNode.Children)
            {
                var typestr = type.Key as YamlScalarNode;
                var sequence = type.Value as YamlSequenceNode;

                var typeid = StringId.FromString(typestr.Value);

                pack.Type[idx] = typeid;
                pack.Names[idx] = new long[sequence.Children.Count];

                var name_idx = 0;
                foreach (var name in sequence.Children)
                {
                    var nameid = StringId.FromString(((YamlScalarNode) name).Value);

                    pack.Names[idx][name_idx] = nameid;
                    ++name_idx;
                }

                ++idx;
            }

            var serializer = MessagePackSerializer.Get<Resource>();
            serializer.Pack(capi.BuildFile, pack);
        }

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