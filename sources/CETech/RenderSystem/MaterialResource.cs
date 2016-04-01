using System.IO;
using CETech.Develop;
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
            var shader_name = ((YamlScalarNode) rootNode.Children[new YamlScalarNode("shader")]).Value;
            var uniforms = (YamlMappingNode)rootNode.Children[new YamlScalarNode("uniforms")];


            var resource = new Resource {shader_name = StringId.FromString(shader_name)};
            resource.unforms_name = new string[uniforms.Children.Count];
            resource.unforms_value = new string[uniforms.Children.Count];
            resource.unforms_type = new int[uniforms.Children.Count];

            var idx = 0;
            foreach (var child in uniforms.Children)
            {
                resource.unforms_type[idx] = 1; // TEXTURE
                resource.unforms_name[idx] = ((YamlScalarNode)child.Key).Value;
                resource.unforms_value[idx] = ((YamlScalarNode)((YamlMappingNode)child.Value).Children[new YamlScalarNode("texture")]).Value;

                ++idx;
            }

            var serializer = MessagePackSerializer.Get<Resource>();
            serializer.Pack(capi.BuildFile, resource);

            capi.add_dependency(shader_name + ".shader");
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
            var resource = serializer.Unpack(input);

            var mat_inst = new MaterialInstance { resource = resource, texture_uniform = new Uniform[resource.unforms_value.Length], texture_resource = new TextureResource.Resource[resource.unforms_value.Length] };

            return mat_inst;
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
            var resource = (MaterialInstance) data;
            resource.instance = ResourceManager.Get<ShaderResource.ShaderInstance>(ShaderResource.Type,
                resource.resource.shader_name);

            var idx = 0;
            foreach (var uniform_name in resource.resource.unforms_value)
            {
                resource.texture_uniform[idx] = new Uniform(uniform_name, UniformType.Int1);
                resource.texture_resource[idx] = ResourceManager.Get<TextureResource.Resource>(TextureResource.Type, StringId.FromString(uniform_name));
                ++idx;
            }
        }

        /// <summary>
        ///     Resource unloader
        /// </summary>
        /// <param name="data">data</param>
        public static void ResourceUnloader(object data)
        {
        }

        public struct Resource
        {
            public long shader_name;
            public string[] unforms_name;
            public int[] unforms_type;
            public string[] unforms_value;
        }

        public class MaterialInstance
        {
            public ShaderResource.ShaderInstance instance;
            public Resource resource;
            public Uniform[] texture_uniform;
            public TextureResource.Resource[] texture_resource;
        }

        public static object Reloader(long name, object new_data)
        {
            var old = ResourceManager.Get<MaterialInstance>(Type, name);
            var resource = (MaterialInstance)new_data;

            old.instance = ResourceManager.Get<ShaderResource.ShaderInstance>(ShaderResource.Type,
                resource.resource.shader_name);

            var idx = 0;
            foreach (var uniform_name in resource.resource.unforms_value)
            {
                old.texture_uniform[idx] = new Uniform(uniform_name, UniformType.Int1);
                old.texture_resource[idx] = ResourceManager.Get<TextureResource.Resource>(TextureResource.Type, StringId.FromString(uniform_name));
                ++idx;
            }

            return old;
        }
    }
}