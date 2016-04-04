using System.IO;
using System.Yaml;
using CETech.Develop;
using CETechDevelop.Utils;
using MsgPack.Serialization;
using SharpBgfx;

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

        private static void preprocess(YamlMapping root)
        {
            if (root.ContainsKey("parent"))
            {
                var prefab_file = ((YamlScalar)root["parent"]).Value + ".material";

                using (var prefab_source = FileSystem.Open("src", prefab_file, FileSystem.OpenMode.Read))
                {
                    TextReader input = new StreamReader(prefab_source);
                    var parent_yaml = YamlNode.FromYaml(input)[0] as YamlMapping;
                    preprocess(parent_yaml);

                    Yaml.merge(root, parent_yaml);
                }
            }
        }

        /// <summary>
        ///     Resource compiler
        /// </summary>
        /// <param name="capi">Compiler api</param>
        public static void Compile(ResourceCompiler.CompilatorApi capi)
        {
            TextReader input = new StreamReader(capi.ResourceFile);
            var yaml = YamlNode.FromYaml(input);

            var rootNode = yaml[0] as YamlMapping;

            preprocess(rootNode);

            var shader_name = ((YamlScalar) rootNode["shader"]).Value;
            var resource = new Resource {shader_name = StringId.FromString(shader_name)};
            if (rootNode.ContainsKey("textures"))
            {
                var textures = (YamlMapping) rootNode["textures"];

                resource.unforms_name = new string[textures.Count];
                resource.texture = new string[textures.Count];

                var idx = 0;
                foreach (var texture in textures)
                {
                    resource.unforms_name[idx] = ((YamlScalar) texture.Key).Value;
                    resource.texture[idx] = ((YamlScalar) texture.Value).Value;

                    ++idx;
                }
            }
            else
            {
                resource.unforms_name = new string[0];
                resource.texture = new string[0];
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

            var mat_inst = new MaterialInstance
            {
                resource = resource,
                texture_uniform = new Uniform[resource.texture.Length],
                texture_resource = new TextureResource.Resource[resource.texture.Length]
            };

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
            foreach (var uniform_name in resource.resource.texture)
            {
                resource.texture_uniform[idx] = new Uniform(uniform_name, UniformType.Int1);
                resource.texture_resource[idx] = ResourceManager.Get<TextureResource.Resource>(TextureResource.Type,
                    StringId.FromString(uniform_name));
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

        public static object ResourceReloader(long name, object new_data)
        {
            // TODO: !!!

            var old = ResourceManager.Get<MaterialInstance>(Type, name);
            var resource = (MaterialInstance) new_data;
            old.resource = resource.resource;
            old.instance = ResourceManager.Get<ShaderResource.ShaderInstance>(ShaderResource.Type,
                resource.resource.shader_name);

            old.texture_uniform = resource.texture_uniform;
            old.texture_resource = resource.texture_resource;

            if (resource.resource.texture.Length > 0)
            {
                var idx = 0;
                foreach (var uniform_name in resource.resource.texture)
                {
                    old.texture_uniform[idx] = new Uniform(uniform_name, UniformType.Int1);
                    old.texture_resource[idx] = ResourceManager.Get<TextureResource.Resource>(TextureResource.Type,
                        StringId.FromString(uniform_name));
                    ++idx;
                }
            }

            return old;
        }

        public struct Resource
        {
            public long shader_name;
            public string[] unforms_name;
            public string[] texture;
        }

        public class MaterialInstance
        {
            public ShaderResource.ShaderInstance instance;
            public Resource resource;
            public TextureResource.Resource[] texture_resource;
            public Uniform[] texture_uniform;
        }
    }
}