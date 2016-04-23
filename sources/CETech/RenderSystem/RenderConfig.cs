using System.Collections.Generic;
using System.IO;
using System.Yaml;
using CETech.Develop;
using CETech.Resource;
using MsgPack.Serialization;
using SharpBgfx;

namespace CETech
{
    /// <summary>
    ///     Render config resource
    /// </summary>
    public class RenderConfig
    {
        /// <summary>
        ///     Resource type
        /// </summary>
        public static readonly long Type = StringId.FromString("render_config");

        /// <summary>
        ///     Resource loader
        /// </summary>
        /// <param name="input">Resource data stream</param>
        /// <returns>Resource data</returns>
        public static object ResourceLoader(Stream input)
        {
            var serializer = MessagePackSerializer.Get<Resource>();
            var resource = serializer.Unpack(input);
            return new ConfigInstance {resource = resource};
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
            var instance = (ConfigInstance) data;
            var resource = instance.resource;

            for (var i = 0; i < resource.GlobalResourceName.Count; i++)
            {
                var name = resource.GlobalResourceName[i];
                var format = (TextureFormat) resource.GlobalResourceFormat[i];
                var ration = (BackbufferRatio) resource.GlobalResourceRation[i];

                var flags = TextureFlags.RenderTarget |
                            TextureFlags.MinFilterPoint | TextureFlags.MagFilterPoint | TextureFlags.MipFilterPoint |
                            TextureFlags.ClampU | TextureFlags.ClampV;


                var texture = Texture.Create2D(ration, 1, format, flags);

                instance.GlobalResource.Add(name, texture);
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
            var old = CETech.Resource.Resource.Get<Resource>(Type, name);
            var neww = (Resource) new_data;


            return old;
        }

        public class Resource
        {
            public List<ulong> GlobalResourceFormat = new List<ulong>();
            public List<long> GlobalResourceName = new List<long>();
            public List<ulong> GlobalResourceRation = new List<ulong>();
        }

#if CETECH_DEVELOP
        private static ulong ParseRenderState(YamlMapping render_states)
        {
            var bgfx_render_state = RenderState.None;

            foreach (var state in render_states)
            {
                var key = ((YamlScalar) state.Key).Value;
                var value = ((YamlScalar) state.Value).Value;

                bgfx_render_state |= Renderer.RenderState(key, value);
            }

            return (ulong) bgfx_render_state;
        }


        private static void ParseRenderTarget(YamlMapping render_target, Resource resource)
        {
            var name = ((YamlScalar) render_target["name"]).Value;
            var format = ((YamlScalar) render_target["format"]).Value;
            var ration = ((YamlScalar) render_target["ration"]).Value;

            var rt_format = Renderer.TextureFormat(format);
            var rt_ration = Renderer.BackbufferRation(ration);

            resource.GlobalResourceName.Add(StringId.FromString(name));
            resource.GlobalResourceFormat.Add(rt_format);
            resource.GlobalResourceRation.Add(rt_ration);
        }

        private static void ParseRenderStates(YamlMapping render_states)
        {
            foreach (var define_states in render_states)
            {
                var name = ((YamlScalar) define_states.Key).Value;

                if (((YamlMapping) define_states.Value).ContainsKey("states"))
                {
                    var states = (YamlMapping) ((YamlMapping) define_states.Value)["states"];

                    ParseRenderState(states);
                }
            }
        }

        private static void ParseGlobalResource(YamlSequence global_resource, Resource resource)
        {
            for (var i = 0; i < global_resource.Count; i++)
            {
                var item = (YamlMapping) global_resource[i];
                var type = ((YamlScalar) item["type"]).Value;

                switch (type)
                {
                    case "render_target":
                        ParseRenderTarget(item, resource);
                        break;
                }
            }
        }

        /// <summary>
        ///     Resource compiler
        /// </summary>
        /// <param name="capi">Compiler api</param>
        public static void Compile(ResourceCompiler.CompilatorApi capi)
        {
            var build_dir = FileSystem.GetRootDir("build");

            TextReader input = new StreamReader(capi.ResourceFile);
            var yaml = YamlNode.FromYaml(input);
            var rootNode = yaml[0] as YamlMapping;

            var resource = new Resource();

            foreach (var node in rootNode)
            {
                switch (((YamlScalar) node.Key).Value)
                {
                    case "global_resource":
                        ParseGlobalResource((YamlSequence) node.Value, resource);
                        break;
                }
            }

            var serializer = MessagePackSerializer.Get<Resource>();
            serializer.Pack(capi.BuildFile, resource);
        }

#endif
    }

    public class ConfigInstance
    {
        public Dictionary<long, Texture> GlobalResource = new Dictionary<long, Texture>();
        public RenderConfig.Resource resource;
    }
}