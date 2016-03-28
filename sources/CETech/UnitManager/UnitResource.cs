using System.Collections.Generic;
using System.IO;
using CETech.Develop;
using MsgPack;
using MsgPack.Serialization;
using YamlDotNet.RepresentationModel;

namespace CETech.World
{
    public class UnitResource
    {
        /// <summary>
        ///     Resource type
        /// </summary>
        public static readonly long Type = StringId.FromString("unit");

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
            var componentsNode = rootNode.Children[new YamlScalarNode("components")] as YamlMappingNode;

            var components_type = new List<long>();
            var components_body = new List<YamlMappingNode>();

            foreach (var component in componentsNode.Children)
            {
                var components_id = component.Key as YamlScalarNode;

                var component_body = component.Value as YamlMappingNode;
                components_body.Add(component_body);

                var component_type = component_body.Children[new YamlScalarNode("component_type")] as YamlScalarNode;
                components_type.Add(StringId.FromString(component_type.Value));
            }

            var packer = new ConsoleServer.ResponsePacker();

            packer.PackMapHeader(2);

            packer.Pack("type");
            packer.PackArrayHeader(components_type.Count);
            for (var i = 0; i < components_type.Count; ++i)
            {
                packer.Pack(components_type[i]);
            }

            packer.Pack("data");
            packer.PackArrayHeader(components_body.Count);
            for (var i = 0; i < components_body.Count; ++i)
            {
                ComponentSystem.Compile(components_type[i], components_body[i], packer);
            }

            packer.GetMemoryStream().WriteTo(capi.BuildFile);
        }
#endif

        /// <summary>
        ///     Resource loader
        /// </summary>
        /// <param name="input">Resource data stream</param>
        /// <returns>Resource data</returns>
        public static object ResourceLoader(Stream input)
        {
            return MessagePackSerializer.Get<Dictionary<MessagePackObject, MessagePackObject>>().Unpack(input);
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