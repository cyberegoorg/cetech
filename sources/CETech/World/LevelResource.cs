using System.Collections.Generic;
using System.IO;
using CETech.Develop;
using MsgPack;
using MsgPack.Serialization;
using YamlDotNet.RepresentationModel;

namespace CETech.World
{
    public class LevelResource
    {
        /// <summary>
        ///     Resource type
        /// </summary>
        public static readonly long Type = StringId.FromString("level");

        /// <summary>
        ///     Resource loader
        /// </summary>
        /// <param name="input">Resource data stream</param>
        /// <returns>Resource data</returns>
        public static object ResourceLoader(Stream input)
        {
            return MessagePackSerializer.Get<MessagePackObjectDictionary>().Unpack(input);
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
            var unitsNode = rootNode.Children[new YamlScalarNode("units")] as YamlMappingNode;

            var packer = new ConsoleServer.ResponsePacker();

            var units_names = new List<long>(unitsNode.Children.Count);

            packer.PackMapHeader(2);
            packer.Pack("units");

            packer.PackArrayHeader(unitsNode.Children.Count);
            foreach (var unit in unitsNode.Children)
            {
                var unit_def = unit.Value as YamlMappingNode;
                var name = unit_def.Children[new YamlScalarNode("name")] as YamlScalarNode;

                units_names.Add(StringId.FromString(name.Value));

                UnitResource.Compile(unit.Value as YamlMappingNode, packer);
            }

            packer.Pack("units_name");
            packer.PackArrayHeader(units_names.Count);
            for (int i = 0; i < units_names.Count; i++)
            {
                packer.Pack(units_names[i]);
            }

            packer.GetMemoryStream().WriteTo(capi.BuildFile);
        }
#endif

        public static object ResourceReloader(long name, object new_data)
        {
            return new_data;
        }
    }
}