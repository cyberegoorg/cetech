using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Yaml;
using CETech.Develop;
using CETech.EntCom;
using CETech.Resource;
using CETech.Utils;
using MsgPack;
using MsgPack.Serialization;

namespace CETech.World
{
    public class UnitResource
    {
        /// <summary>
        ///     ResourceManager type
        /// </summary>
        public static readonly long Type = StringId64.FromString("unit");

        /// <summary>
        ///     ResourceManager loader
        /// </summary>
        /// <param name="input">ResourceManager data stream</param>
        /// <returns>ResourceManager data</returns>
        public static object ResourceLoader(Stream input)
        {
            //return MessagePackSerializer.Get<MessagePackObjectDictionary>().Unpack(input);
            return MessagePackSerializer.Get<CompiledResource>().Unpack(input);
        }

        /// <summary>
        ///     ResourceManager offline.
        /// </summary>
        /// <param name="data">Data</param>
        public static void ResourceOffline(object data)
        {
        }

        /// <summary>
        ///     ResourceManager online
        /// </summary>
        /// <param name="data">Data</param>
        public static void ResourceOnline(object data)
        {
        }

        /// <summary>
        ///     ResourceManager unloader
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
        public class EntityCompileOutput
        {
            public Dictionary<long, List<long>> ComponentEnt = new Dictionary<long, List<long>>();
            public Dictionary<long, List<YamlMapping>> ComponentsBody = new Dictionary<long, List<YamlMapping>>();
            public List<long> ComponentsType = new List<long>();
            public Dictionary<long, long> EntsParent = new Dictionary<long, long>();
        }

        public static void compile_entitity(YamlMapping rootNode, ref int entities_id, int parent,
            EntityCompileOutput output)
        {
            output.EntsParent[entities_id] = parent;

            var componentsNode = (YamlMapping) rootNode["components"];

            foreach (var component in componentsNode)
            {
                //var component_guid = component.Value as YamlScalar;
                //var components_id = component.Key as YamlScalarNode;
                var component_body = component.Value as YamlMapping;
                var component_type = component_body["component_type"] as YamlScalar;

                var cid = StringId64.FromString(component_type.Value);
                if (!output.ComponentsType.Contains(cid))
                {
                    output.ComponentsType.Add(cid);
                    output.ComponentEnt[cid] = new List<long>();
                }

                if (!output.ComponentsBody.ContainsKey(cid))
                {
                    output.ComponentsBody[cid] = new List<YamlMapping>();
                }

                output.ComponentEnt[cid].Add(entities_id);
                output.ComponentsBody[cid].Add(component_body);
            }

            if (rootNode.ContainsKey("children"))
            {
                var parent_ent = entities_id;
                var childrenNode = rootNode["children"] as YamlMapping;

                foreach (var child in childrenNode)
                {
                    entities_id += 1;
                    compile_entitity(child.Value as YamlMapping, ref entities_id, parent_ent, output);
                }
            }
        }

        private static void preprocess(YamlMapping root, ResourceCompiler.CompilatorApi capi)
        {
            if (root.ContainsKey("prefab"))
            {
                var prefab_file = ((YamlScalar) root["prefab"]).Value + ".unit";

                capi.add_dependency(prefab_file);

                using (var prefab_source = FileSystem.Open("src", prefab_file, FileSystem.OpenMode.Read))
                {
                    TextReader input = new StreamReader(prefab_source);
                    var parent_yaml = YamlNode.FromYaml(input)[0] as YamlMapping;

                    preprocess(parent_yaml, capi);
                    Yaml.merge(root, parent_yaml);
                }
            }
        }

        public static void Compile(YamlMapping root, ConsoleServer.ResponsePacker packer,
            ResourceCompiler.CompilatorApi capi)
        {
            var entities_id = 0;

            preprocess(root, capi);

            var compile_output = new EntityCompileOutput();
            compile_entitity(root, ref entities_id, int.MaxValue, compile_output);

            var components_type_sorted =
                compile_output.ComponentsType.OrderBy(pair => ComponentSystem.GetSpawnOrder(pair))
                    .Select(pair => pair)
                    .ToArray();

            packer.PackMapHeader(5);

            var ent_count = entities_id + 1;
            packer.Pack("ent_count");
            packer.Pack(ent_count);

            packer.Pack("ents_parent");
            packer.PackArrayHeader(ent_count);
            for (var i = 0; i < ent_count; ++i)
            {
                packer.Pack(compile_output.EntsParent[i]);
            }


            packer.Pack("type");
            packer.PackArrayHeader(components_type_sorted.Length);
            for (var i = 0; i < components_type_sorted.Length; ++i)
            {
                packer.Pack(components_type_sorted[i]);
            }

            packer.Pack("ent");
            packer.PackArrayHeader(components_type_sorted.Length);
            for (var i = 0; i < components_type_sorted.Length; ++i)
            {
                var comp_type = components_type_sorted[i];
                var ents = compile_output.ComponentEnt[comp_type];

                packer.PackArrayHeader(ents.Count);
                for (var j = 0; j < ents.Count; j++)
                {
                    packer.Pack(ents[j]);
                }
            }

            packer.Pack("data");
            packer.PackArrayHeader(components_type_sorted.Length);
            for (var i = 0; i < components_type_sorted.Length; ++i)
            {
                var comp_type = components_type_sorted[i];
                var comp_body = compile_output.ComponentsBody[comp_type];

                packer.PackArrayHeader(comp_body.Count);
                for (var j = 0; j < comp_body.Count; j++)
                {
                    ComponentSystem.Compile(comp_type, comp_body[j], packer);
                }
            }
        }

        /// <summary>
        ///     ResourceManager compiler
        /// </summary>
        /// <param name="capi">Compiler api</param>
        public static void Compile(ResourceCompiler.CompilatorApi capi)
        {
            TextReader input = new StreamReader(capi.ResourceFile);
            var yaml = YamlNode.FromYaml(input);

            var packer = new ConsoleServer.ResponsePacker();

            Compile((YamlMapping) yaml[0], packer, capi);

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