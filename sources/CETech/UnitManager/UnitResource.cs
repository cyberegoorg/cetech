using System;
using System.Collections.Generic;
using System.IO;
using System.Yaml;
using CETech.Develop;
using MsgPack;
using MsgPack.Serialization;

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
            //return MessagePackSerializer.Get<MessagePackObjectDictionary>().Unpack(input);
            return MessagePackSerializer.Get<CompiledResource>().Unpack(input);
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

        public static object Reloader(long name, object new_data)
        {
            return new_data;
        }

        public class CompiledResource
        {
            public int ent_count { get; set; }
            public int[] ents_parent { get; set; }
            public long[] type { get; set; }
            public long[][] ent { get; set; }

            public MessagePackObjectDictionary[][] data { get; set; }
        }

#if CETECH_DEVELOP

        public static void compile_entitity(YamlMapping rootNode, ref int entities_id, int parent,
            Dictionary<long, long> ents_parent, List<long> components_type,
            Dictionary<long, List<long>> component_ent, Dictionary<long, List<YamlMapping>> components_body)
        {
            ents_parent[entities_id] = parent;

            var componentsNode = (YamlMapping) rootNode["components"];

            foreach (var component in componentsNode)
            {
                //var component_guid = component.Value as YamlScalar;
                //var components_id = component.Key as YamlScalarNode;
                var component_body = component.Value as YamlMapping;
                var component_type = component_body["component_type"] as YamlScalar;

                var cid = StringId.FromString(component_type.Value);
                if (!components_type.Contains(cid))
                {
                    components_type.Add(cid);
                    component_ent[cid] = new List<long>();
                }

                if (!components_body.ContainsKey(cid))
                {
                    components_body[cid] = new List<YamlMapping>();
                }

                component_ent[cid].Add(entities_id);
                components_body[cid].Add(component_body);
            }

            if (rootNode.ContainsKey("children"))
            {
                var parent_ent = entities_id;
                var childrenNode = rootNode["children"] as YamlMapping;

                foreach (var child in childrenNode)
                {
                    entities_id += 1;
                    compile_entitity(child.Value as YamlMapping, ref entities_id, parent_ent,
                        ents_parent,
                        components_type, component_ent,
                        components_body);
                }
            }
        }

        private static void preprocess(YamlMapping root)
        {
            if (root.ContainsKey("prefab"))
            {
                var prefab_file = ((YamlScalar)root["prefab"]).Value + ".unit";

                using (var prefab_source = FileSystem.Open("src", prefab_file, FileSystem.OpenMode.Read))
                {
                    TextReader input = new StreamReader(prefab_source);
                    var yaml = YamlNode.FromYaml(input)[0] as YamlMapping;

                    preprocess(yaml);

                    root["components"] = yaml["components"];

                    if (root.ContainsKey("children"))
                    {
                        var children = root["children"] as YamlMapping;
                        foreach (var child in children)
                        {
                            preprocess(child.Value as YamlMapping);

                            ((YamlMapping) root["children"])[child.Key] = child.Value;
                        }
                    }
                }
            }
        }

        public static void Compile(YamlMapping root, ConsoleServer.ResponsePacker packer)
        {
            var entities_id = 0;

            var components_type = new List<long>();
            var component_ent = new Dictionary<long, List<long>>();
            var components_body = new Dictionary<long, List<YamlMapping>>();
            var ents_parent = new Dictionary<long, long>();

            preprocess(root);

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
            var yaml = YamlNode.FromYaml(input);

            var packer = new ConsoleServer.ResponsePacker();

            Compile((YamlMapping) yaml[0], packer);

            packer.GetMemoryStream().WriteTo(capi.BuildFile);

            /*
                        TextReader input = new StreamReader(capi.ResourceFile);
                        var yaml = new YamlStream();
                        yaml.Load(input);

                        var rootNode = yaml.Documents[0].RootNode as YamlMappingNode;
                        var packer = new ConsoleServer.ResponsePacker();

                        Compile(rootNode, packer);

                        packer.GetMemoryStream().WriteTo(capi.BuildFile);
            */
        }
#endif
    }
}