using System.Collections.Generic;
using System.Linq;
using System.Yaml;
using CETech.Develop;
using CETech.EntCom;
using CETech.Resource;
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

        private static int CreateImpl(int world, int entity, long material, long mesh)
        {
            var world_instance = _worldInstance[world];

            var idx = world_instance.MaterialInstance.Count;

            var material_instance = Resource.Resource.Get<MaterialResource.MaterialInstance>(MaterialResource.Type,
                material);

            var mesh_instance = Resource.Resource.Get<MeshResource.MeshInstance>(MeshResource.Type, mesh);

            world_instance.MaterialInstance.Add(material_instance);
            world_instance.MeshInstance.Add(mesh_instance);

            world_instance.EntIdx[entity] = idx;

            return idx;
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
                CreateImpl(world, ent_ids[i], data[i]["material"].AsInt64(), data[i]["mesh"].AsInt64());
            }
        }

        private static void Compiler(YamlMapping body, ConsoleServer.ResponsePacker packer)
        {
            var mesh = body["mesh"] as YamlScalar;
            var material = body["material"] as YamlScalar;

            packer.PackMapHeader(2);

            packer.Pack("mesh");
            packer.Pack(StringId.FromString(mesh.Value));

            packer.Pack("material");
            packer.Pack(StringId.FromString(material.Value));
        }

        private static void InitImpl()
        {
#if CETECH_DEVELOP
            ComponentSystem.RegisterCompiler(StringId.FromString("mesh_renderer"), Compiler, 10);
#endif
            ComponentSystem.RegisterType(StringId.FromString("mesh_renderer"), Spawner, Destroyer);
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
            // TODO: SHITCODE
            var world_instance = _worldInstance[world];

            for (var i = 0; i < world_instance.EntIdx.Count; i++)
            {
                var item = world_instance.EntIdx.ElementAt(i);
                var idx = item.Value;

                var mesh_instance = world_instance.MeshInstance[idx];
                var material_instance = world_instance.MaterialInstance[idx];

                var tu = material_instance.texture_uniform;
                var t = material_instance.texture_resource;

                for (var j = 0; j < tu.Length; j++)
                {
                    Bgfx.SetTexture(0, tu[j], t[j].texture);
                }


                var world_matrix = Tranform.GetWorldMatrix(world,
                    Tranform.GetTranform(world, item.Key));

                unsafe
                {
                    Bgfx.SetTransform(&world_matrix.M11);
                }


                Bgfx.SetVertexBuffer(mesh_instance.vb[0]);
                Bgfx.SetIndexBuffer(mesh_instance.ib[0]);

                var state = 0 |
                            RenderState.ColorWrite |
                            RenderState.AlphaWrite |
                            RenderState.DepthWrite |
                            RenderState.DepthTestLess |
                            RenderState.CullCounterclockwise |
                            RenderState.Multisampling;

                Bgfx.SetRenderState(state);

                // submit primitives
                Bgfx.Submit(0, material_instance.instance.program);
            }
        }

        private class WorldInstance
        {
            public readonly Dictionary<int, int> EntIdx;
            public readonly List<MaterialResource.MaterialInstance> MaterialInstance;
            public readonly List<MeshResource.MeshInstance> MeshInstance;

            public WorldInstance()
            {
                EntIdx = new Dictionary<int, int>();
                MaterialInstance = new List<MaterialResource.MaterialInstance>();
                MeshInstance = new List<MeshResource.MeshInstance>();
            }
        }
    }
}