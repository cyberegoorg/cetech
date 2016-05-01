using System.Collections.Generic;
using System.Linq;
using System.Yaml;
using CETech.CEMath;
using CETech.Develop;
using CETech.EntCom;
using CETech.Resource;
using CETech.Utils;
using MsgPack;
using SharpBgfx;

namespace CETech.World
{
    public partial class MeshRenderer
    {
        private static readonly Dictionary<int, WorldInstance> _worldInstance = new Dictionary<int, WorldInstance>();

        private static int getIdx(int world, int entity)
        {
            return _worldInstance[world].EntIdx[entity];
        }

        private static int CreateImpl(int world, int entity, long material, long scene, long mesh)
        {
            var mesh_instance = Resource.Resource.Get<SceneResource.SceneInstance>(SceneResource.Type, scene);

            var names = mesh_instance.resource.geom_name;
            for (var i = 0; i < names.Length; i++)
            {
                if (names[i] != mesh)
                {
                    continue;
                }

                var world_instance = _worldInstance[world];

                var idx = world_instance.MaterialInstance.Count;
                var material_instance = Resource.Resource.Get<MaterialResource.MaterialInstance>(MaterialResource.Type,
                    material);

                world_instance.EntIdx[entity] = idx;
                world_instance.MaterialInstance.Add(material_instance);
                world_instance.SceneInstance.Add(mesh_instance);
                world_instance.MeshIdx.Add(i);

                var local_pose = mesh_instance.resource.node_local[i];
                var pose = new Mat4f[mesh_instance.resource.node_names[i].Length];
                var pose_id = 0;
                for (var j = 0; j < pose.Length; j += 16)
                {
                    var m_idx = j;

                    pose[pose_id++] = new Mat4f(
                        local_pose[m_idx + 0],
                        local_pose[m_idx + 1],
                        local_pose[m_idx + 2],
                        local_pose[m_idx + 3],
                        local_pose[m_idx + 4],
                        local_pose[m_idx + 5],
                        local_pose[m_idx + 6],
                        local_pose[m_idx + 7],
                        local_pose[m_idx + 8],
                        local_pose[m_idx + 9],
                        local_pose[m_idx + 10],
                        local_pose[m_idx + 11],
                        local_pose[m_idx + 12],
                        local_pose[m_idx + 13],
                        local_pose[m_idx + 14],
                        local_pose[m_idx + 15]
                        );
                }

                var node = SceneGraph.Create(world, entity,
                    mesh_instance.resource.node_names[i], mesh_instance.resource.node_parent[i], pose);

                Log.Debug("scene_graph", "n_cube {0}",
                    SceneGraph.GetNodeByName(world, entity, StringId64.FromString("n_cube")));
                Log.Debug("scene_graph", "g_cube {0}",
                    SceneGraph.GetNodeByName(world, entity, StringId64.FromString("g_cube")));
                Log.Debug("scene_graph", "nn_cube {0}",
                    SceneGraph.GetNodeByName(world, entity, StringId64.FromString("nn_cube")));

                world_instance.NodeIdx.Add(node);

                return idx;
            }

            return int.MaxValue;
        }

        private static void InitWorldImpl(int world)
        {
            _worldInstance[world] = new WorldInstance();
        }

        private static void RemoveWorldImpl(int world)
        {
            _worldInstance.Remove(world);
            //TODO clean all
        }

        private static void Spawner(int world, int[] ent_ids, int[] ents_parent, MessagePackObjectDictionary[] data)
        {
            for (var i = 0; i < ent_ids.Length; ++i)
            {
                CreateImpl(world, ent_ids[i], data[i]["material"].AsInt64(), data[i]["scene"].AsInt64(),
                    data[i]["mesh"].AsInt64());
            }
        }

        private static void Compiler(YamlMapping body, ConsoleServer.ResponsePacker packer)
        {
            var scene = body["scene"] as YamlScalar;
            var mesh = body["mesh"] as YamlScalar;
            var node = body["node"] as YamlScalar;
            var material = body["material"] as YamlScalar;

            packer.PackMapHeader(4);

            packer.Pack("scene");
            packer.Pack(StringId64.FromString(scene.Value));

            packer.Pack("node");
            packer.Pack(StringId64.FromString(node.Value));

            packer.Pack("mesh");
            packer.Pack(StringId64.FromString(mesh.Value));

            packer.Pack("material");
            packer.Pack(StringId64.FromString(material.Value));
        }

        private static void InitImpl()
        {
#if CETECH_DEVELOP
            ComponentSystem.RegisterCompiler(StringId64.FromString("mesh_renderer"), Compiler, 10);
#endif
            ComponentSystem.RegisterType(StringId64.FromString("mesh_renderer"), Spawner, Destroyer);
        }

        private static void Destroyer(int world, int[] entIds)
        {
            // TODO
            var world_instance = _worldInstance[world];

            for (var i = 0; i < entIds.Length; i++)
            {
                var ent_id = entIds[i];
                world_instance.EntIdx.Remove(ent_id);
            }
        }

        private static void ShutdownImpl()
        {
        }

        public static void RenderWorldImpl(int world)
        {
            var world_instance = _worldInstance[world];

            // TODO: SHITCODE
            for (var i = 0; i < world_instance.EntIdx.Count; i++)
            {
                // TODO: !!!
                var item = world_instance.EntIdx.ElementAt(i);
                var idx = item.Value;

                var meshInstance = world_instance.SceneInstance[idx];
                var materialInstance = world_instance.MaterialInstance[idx];
                var mesh_idx = world_instance.MeshIdx[idx];

                var textureUniforms = materialInstance.texture_uniform;
                var textureResource = materialInstance.texture_resource;

                for (var j = 0; j < textureUniforms.Length; j++)
                {
                    Bgfx.SetTexture(0, textureUniforms[j], textureResource[j].texture);
                }

                var world_matrix = Tranform.GetWorldMatrix(world, Tranform.GetTranform(world, item.Key));
                var node_world = SceneGraph.GetWorldMatrix(world, world_instance.NodeIdx[idx]);
                var final_matrix = node_world*world_matrix;

                unsafe
                {
                    Bgfx.SetTransform(&final_matrix.M11);
                }

                Bgfx.SetVertexBuffer(meshInstance.vb[mesh_idx]);
                Bgfx.SetIndexBuffer(meshInstance.ib[mesh_idx]);

                var state = 0 |
                            RenderState.ColorWrite |
                            RenderState.AlphaWrite |
                            RenderState.DepthWrite |
                            RenderState.DepthTestLess |
                            RenderState.CullCounterclockwise |
                            RenderState.Multisampling;

                Bgfx.SetRenderState(state);

                // submit primitives
                Bgfx.Submit(0, materialInstance.instance.program);
            }
        }

        private class WorldInstance
        {
            public readonly Dictionary<int, int> EntIdx;
            public readonly List<MaterialResource.MaterialInstance> MaterialInstance;
            public readonly List<int> MeshIdx;
            public readonly List<int> NodeIdx;
            public readonly List<SceneResource.SceneInstance> SceneInstance;

            public WorldInstance()
            {
                EntIdx = new Dictionary<int, int>();
                MaterialInstance = new List<MaterialResource.MaterialInstance>();
                SceneInstance = new List<SceneResource.SceneInstance>();
                MeshIdx = new List<int>();
                NodeIdx = new List<int>();
            }
        }
    }
}