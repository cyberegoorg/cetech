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
        public static void compile_entitity(YamlMappingNode rootNode, ref int entities_id, int parent,
            Dictionary<long, long> ents_parent, List<long> components_type,
            Dictionary<long, List<long>> component_ent, Dictionary<long, List<YamlMappingNode>> components_body)
        {
            ents_parent[entities_id] = parent;

            var componentsNode = rootNode.Children[new YamlScalarNode("components")] as YamlMappingNode;
            foreach (var component in componentsNode.Children)
            {
                var components_id = component.Key as YamlScalarNode;
                var component_body = component.Value as YamlMappingNode;
                var component_type = component_body.Children[new YamlScalarNode("component_type")] as YamlScalarNode;

                var cid = StringId.FromString(component_type.Value);
                if (!components_type.Contains(cid))
                {
                    components_type.Add(cid);
                    component_ent[cid] = new List<long>();
                }

                if (!components_body.ContainsKey(cid))
                {
                    components_body[cid] = new List<YamlMappingNode>();
                }

                component_ent[cid].Add(entities_id);
                components_body[cid].Add(component_body);
            }

            if (rootNode.Children.ContainsKey(new YamlScalarNode("children")))
            {
                var parent_ent = entities_id;

                var childrenNode = rootNode.Children[new YamlScalarNode("children")] as YamlMappingNode;
                foreach (var child in childrenNode.Children)
                {
                    entities_id += 1;
                    compile_entitity(child.Value as YamlMappingNode, ref entities_id, parent_ent, ents_parent,
                        components_type, component_ent,
                        components_body);
                }
            }
        }

        public static void Compile(YamlMappingNode root, ConsoleServer.ResponsePacker packer)
        {
            var entities_id = 0;

            var components_type = new List<long>();
            var component_ent = new Dictionary<long, List<long>>();
            var components_body = new Dictionary<long, List<YamlMappingNode>>();
            var ents_parent = new Dictionary<long, long>();

            compile_entitity(root, ref entities_id, int.MaxValue, ents_parent, components_type, component_ent,
                components_body);

            packer.PackMapHeader(5);

            var ent_count = entities_id + 1;
            packer.Pack("ent_count");
            packer.Pack(ent_count);

            packer.Pack("ents_parent");
            packer.PackArrayHeader(ent_count);
            for (var i = 0; i < ent_count; ++i)
            {
                packer.Pack(ents_parent[i]);
            }


            packer.Pack("type");
            packer.PackArrayHeader(components_type.Count);
            for (var i = 0; i < components_type.Count; ++i)
            {
                packer.Pack(components_type[i]);
            }

            packer.Pack("ent");
            packer.PackArrayHeader(components_type.Count);
            for (var i = 0; i < components_type.Count; ++i)
            {
                var comp_type = components_type[i];
                var ents = component_ent[comp_type];

                packer.PackArrayHeader(ents.Count);
                for (var j = 0; j < ents.Count; j++)
                {
                    packer.Pack(ents[j]);
                }
            }

            packer.Pack("data");
            packer.PackArrayHeader(components_type.Count);
            for (var i = 0; i < components_type.Count; ++i)
            {
                var comp_type = components_type[i];
                var comp_body = components_body[comp_type];

                packer.PackArrayHeader(comp_body.Count);
                for (var j = 0; j < comp_body.Count; j++)
                {
                    ComponentSystem.Compile(comp_type, comp_body[j], packer);
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
            var yaml = new YamlStream();
            yaml.Load(input);

            var rootNode = yaml.Documents[0].RootNode as YamlMappingNode;
            var packer = new ConsoleServer.ResponsePacker();

            Compile(rootNode, packer);

            packer.GetMemoryStream().WriteTo(capi.BuildFile);
        }
#endif

        public static object Reloader(long name, object new_data)
        {
            return new_data;
        }
    }
}