using System.IO;
using MsgPack.Serialization;
using YamlDotNet.RepresentationModel;

namespace CETech
{
    public class PackageResource
    {
        public static readonly long Type = StringId.FromString("package");

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
                    var nameid = StringId.FromString(((YamlScalarNode)name).Value);

                    pack.Names[idx][name_idx] = nameid;
                    ++name_idx;
                }

                ++idx;
            }

            var serializer = MessagePackSerializer.Get<Resource>();
            serializer.Pack(capi.BuildFile, pack);
        }

        public static object ResourceLoader(Stream input)
        {
            var serializer = MessagePackSerializer.Get<Resource>();
            return serializer.Unpack(input);
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

        public struct Resource
        {
            public long[] Type;
            public long[][] Names;
        }
    }
}