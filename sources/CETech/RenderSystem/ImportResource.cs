using System;
using System.Collections.Generic;
using System.IO;
using System.Yaml;
using CETech.Develop;
using CETech.Resource;
using Assimp;
using Assimp.Configs;


namespace CETech
{
    /// <summary>
    ///     Fakse resource for asset importing.
    /// </summary>
    public class ImportResource
    {

        /// <summary>
        ///     Type name
        /// </summary>
        public static readonly long Type = StringId64.FromString("import");

        /// <summary>
        ///     ResourceManager compiler
        /// </summary>
        /// <param name="capi">Compiler api</param>
        public static void Compile(ResourceCompiler.CompilatorApi capi)
        {
            TextReader input = new StreamReader(capi.ResourceFile);
            var yaml = YamlNode.FromYaml(input);
            var rootNode = yaml[0] as YamlMapping;

            if (rootNode.ContainsKey("assimp"))
            {
                var assimp_node = (YamlMapping)rootNode["assimp"];
                import_assimp(((YamlScalar)assimp_node["input"]).Value, capi);
                return;
            }
        }

        private static void import_assimp_scene_graph(Node root_node, YamlMapping out_graph, Dictionary<int, string> geometriesMap)
        {
            var node_maping = new YamlMapping();
            var local = root_node.Transform;

            node_maping["local"] = new YamlSequence(
                local.A1, local.A2, local.A3, local.A4,
                local.B1, local.B2, local.B3, local.B4,
                local.C1, local.C2, local.C3, local.C4,
                local.D1, local.D2, local.D3, local.D4);

            if (root_node.HasMeshes)
            {
                var seq = new YamlSequence();

                for (int i = 0; i < root_node.MeshCount; i++)
                {
                    seq.Add(geometriesMap[root_node.MeshIndices[i]]);
                }

                node_maping["geometries"] = seq;
            }

            if (root_node.HasChildren)
            {
                var children_node = new YamlMapping();

                for (int i = 0; i < root_node.ChildCount; i++)
                {
                    var node = root_node.Children[i];
                    
                    import_assimp_scene_graph(node, children_node, geometriesMap);
                }

                node_maping["children"] = children_node;
            }

            out_graph[root_node.Name] = node_maping;
        }

        private static void import_assimp_scene(Scene assimp_scene, ResourceCompiler.CompilatorApi capi)
        {
            var geometries = new YamlMapping();
            var used_names = new HashSet<string>();

            var geometries_map = new Dictionary<int, string>();

            for (var i = 0; i < assimp_scene.MeshCount; i++)
            {
                var mesh = assimp_scene.Meshes[i];
                var mesh_name = mesh.Name;

                if (mesh_name.Length == 0)
                {
                    mesh_name = string.Format("geom_{0}", i);
                }

                if (used_names.Contains(mesh_name))
                {
                    var unique = 1;
                    while (used_names.Contains(mesh_name))
                    {
                        mesh_name = string.Format("{0}{1}", mesh.Name, unique++);
                    }
                }

                used_names.Add(mesh_name);
                geometries_map[i] = mesh_name;

                var mesh_yaml = new YamlMapping();

                geometries[mesh_name] = mesh_yaml;

                var chanels = new YamlMapping();
                var types = new YamlMapping();
                var indices = new YamlMapping();

                var indices_seq = new YamlSequence();
                var mesh_indicies = mesh.GetIndices();
                indices["size"] = mesh_indicies.Length;
                for (var j = 0; j < mesh_indicies.Length; j++)
                {
                    indices_seq.Add(mesh_indicies[j]);
                }

                // position
                var verticies_node = new YamlSequence();
                var nromals_node = new YamlSequence();
                var tc0_node = new YamlSequence();
                for (var j = 0; j < mesh.VertexCount; j++)
                {
                    var vert = mesh.Vertices[j];

                    verticies_node.Add(vert.X);
                    verticies_node.Add(vert.Y);
                    verticies_node.Add(vert.Z);


                    if (mesh.HasNormals)
                    {
                        var norm = mesh.Normals[j];

                        nromals_node.Add(norm.X);
                        nromals_node.Add(norm.Y);
                        nromals_node.Add(norm.Z);
                    }


                    if (mesh.HasTextureCoords(0))
                    {
                        var tc = mesh.TextureCoordinateChannels[0][j];

                        tc0_node.Add(tc.X);
                        tc0_node.Add(tc.Y);
                    }
                }

                chanels["position"] = verticies_node;
                indices["position"] = indices_seq;
                types["position"] = "vec3";

                // normals
                if (mesh.HasNormals)
                {
                    chanels["normal"] = nromals_node;
                    indices["normal"] = indices_seq;
                    types["normal"] = "vec3";
                }

                // normals
                if (mesh.HasTextureCoords(0))
                {
                    chanels["texcoord0"] = tc0_node;
                    indices["texcoord0"] = indices_seq;
                    types["texcoord0"] = "vec2";
                }

                mesh_yaml["chanels"] = chanels;
                mesh_yaml["indices"] = indices;
                mesh_yaml["types"] = types;
            }


            var graph = new YamlMapping();
            import_assimp_scene_graph(assimp_scene.RootNode, graph, geometries_map);

            var rootNode = new YamlMapping(
                "geometries", geometries,
                "graph", graph
                );

            var output_file = capi.Filename.Replace(".import", ".scene");
            rootNode.ToYamlFile(FileSystem.GetFullPath(capi._sourceRoot, output_file));
        }

        private static void import_assimp(string name, ResourceCompiler.CompilatorApi capi)
        {
            var filename = FileSystem.GetFullPath("src", name);

            //TODO: import config
            var importer = new AssimpContext();

            importer.SetConfig(new NormalSmoothingAngleConfig(66.0f));
            var scene = importer.ImportFile(filename,
                PostProcessPreset.TargetRealTimeMaximumQuality | PostProcessSteps.FlipUVs |
                PostProcessSteps.MakeLeftHanded);

            import_assimp_scene(scene, capi);
        }
    }
}