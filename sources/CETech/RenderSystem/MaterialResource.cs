using System.Diagnostics;
using System.IO;
using CETech.Develop;
using CETech.Utils;
using MsgPack.Serialization;
using SharpBgfx;
using YamlDotNet.RepresentationModel;

namespace CETech
{
    /// <summary>
    ///     Package resource
    /// </summary>
    public class MaterialResource
    {
        /// <summary>
        ///     Resource type
        /// </summary>
        public static readonly long Type = StringId.FromString("material");

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
            var shader_name = ((YamlScalarNode)rootNode.Children[new YamlScalarNode("shader")]).Value;

            var serializer = MessagePackSerializer.Get<Resource>();
            serializer.Pack(capi.BuildFile, new Resource() { shader_name = StringId.FromString(shader_name)});

            capi.add_dependency(shader_name+".shader");
        }
#endif

        public struct Resource
        {
            public long shader_name;
        }

        public class MaterialInstance
        {
            public ShaderResource.ShaderInstance instance;
            public Resource resource;
        }

        /// <summary>
        ///     Resource loader
        /// </summary>
        /// <param name="input">Resource data stream</param>
        /// <returns>Resource data</returns>
        public static object ResourceLoader(Stream input)
        {
            var serializer = MessagePackSerializer.Get<Resource>();
            Resource resource = serializer.Unpack(input);
            return new MaterialInstance() { resource = resource};
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
            var resource = (MaterialInstance)data;
            resource.instance = ResourceManager.Get<ShaderResource.ShaderInstance>(ShaderResource.Type, resource.resource.shader_name);
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