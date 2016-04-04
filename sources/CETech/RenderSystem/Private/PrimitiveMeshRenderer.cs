using System;
using System.Collections.Generic;
using System.Globalization;
using System.Yaml;
using CETech.Develop;
using MsgPack;
using SharpBgfx;

namespace CETech.World
{
    public partial class PrimitiveMeshRenderer

    {
        private static readonly Dictionary<int, Dictionary<int, float>> _world_ent_idx =
            new Dictionary<int, Dictionary<int, float>>();

        private static readonly Dictionary<int, Dictionary<int, MaterialResource.MaterialInstance>> _world_ent_material
            = new Dictionary<int, Dictionary<int, MaterialResource.MaterialInstance>>();

        private static readonly Dictionary<float, VertexBuffer> _size_vb = new Dictionary<float, VertexBuffer>();
        private static IndexBuffer _size_ib;

        private static void InitWorldImpl(int world)
        {
            _size_ib = Cube.CreateIndexBuffer();

            _world_ent_idx[world] = new Dictionary<int, float>();
            _world_ent_material[world] = new Dictionary<int, MaterialResource.MaterialInstance>();
        }

        private static void RemoveWorldImpl(int world)
        {
            _world_ent_idx.Remove(world);
        }

        private static void Spawner(int world, int[] ent_ids, int[] ents_parent, MessagePackObjectDictionary[] data)
        {
            for (var i = 0; i < ent_ids.Length; ++i)
            {
                var size = data[i]["size"].AsSingle();


                if (!_size_vb.ContainsKey(size))
                {
                    _size_vb[size] = Cube.CreateVertexBuffer(size);
                }

                _world_ent_idx[world][ent_ids[i]] = size;
                _world_ent_material[world][ent_ids[i]] =
                    ResourceManager.Get<MaterialResource.MaterialInstance>(MaterialResource.Type,
                        data[i]["material"].AsInt64());
            }
        }

        private static void Compiler(YamlMapping body, ConsoleServer.ResponsePacker packer)
        {
            var size = body["size"] as YamlScalar;
            var material = body["material"] as YamlScalar;

            packer.PackMapHeader(2);

            packer.Pack("size");
            packer.Pack(float.Parse(size.Value, CultureInfo.InvariantCulture));

            packer.Pack("material");
            packer.Pack(StringId.FromString(material.Value));
        }

        private static void InitImpl()
        {
#if CETECH_DEVELOP
            ComponentSystem.RegisterCompiler(StringId.FromString("primitive_mesh"), Compiler, 10);
#endif
            ComponentSystem.RegisterSpawner(StringId.FromString("primitive_mesh"), Spawner);
        }

        private static void ShutdownImpl()
        {
        }

        public static void RenderWorldImpl(int world)


        {
            foreach (var f in _world_ent_idx[world])
            {
                var idx = 0;
                var textures = _world_ent_material[world][f.Key].texture_resource;
                foreach (var un in _world_ent_material[world][f.Key].texture_uniform)
                {
                    Bgfx.SetTexture(0, un, textures[idx].texture);
                    ++idx;
                }

                var world_matrix = TranformationSystem.GetWorldMatrix(world, TranformationSystem.GetTranform(world,f.Key));
                unsafe
                {
                    Bgfx.SetTransform(&world_matrix.M11);
                }

                // set pipeline states
                Bgfx.SetVertexBuffer(_size_vb[f.Value]);
                Bgfx.SetIndexBuffer(_size_ib);
                Bgfx.SetRenderState(RenderState.Default);

                // submit primitives
                Bgfx.Submit(0, _world_ent_material[world][f.Key].instance.program);
            }
        }

        public struct PosNormalTexcoordVertex
        {
            private float x;
            private float y;
            private float z;
            private uint normal;
            private float u;
            private float v;

            public PosNormalTexcoordVertex(float x, float y, float z, uint normal, float u, float v)
            {
                this.x = x;
                this.y = y;
                this.z = z;
                this.normal = normal;
                this.u = u;
                this.v = v;
            }

            public static readonly VertexLayout Layout = new VertexLayout().Begin()
                .Add(VertexAttributeUsage.Position, 3, VertexAttributeType.Float)
                .Add(VertexAttributeUsage.Normal, 4, VertexAttributeType.UInt8, true, true)
                .Add(VertexAttributeUsage.TexCoord0, 2, VertexAttributeType.Float)
                .End();
        }


        public static class Cube
        {
            private static readonly ushort[] indices =
            {
                0, 1, 2,
                1, 3, 2,
                4, 6, 5,
                5, 6, 7,
                8, 9, 10,
                9, 11, 10,
                12, 14, 13,
                13, 14, 15,
                16, 17, 18,
                17, 19, 18,
                20, 22, 21,
                21, 22, 23
            };

            private static uint PackF4u(float x, float y, float z)
            {
                var bytes = new byte[]
                {
                    (byte) (x*127.0f + 128.0f),
                    (byte) (y*127.0f + 128.0f),
                    (byte) (z*127.0f + 128.0f),
                    128
                };

                return BitConverter.ToUInt32(bytes, 0);
            }

            public static VertexBuffer CreateVertexBuffer(float size)
            {
                PosNormalTexcoordVertex[] vertices =
                {
                    new PosNormalTexcoordVertex(-size, size, size, PackF4u(0.0f, 1.0f, 0.0f), 1.0f, 1.0f),
                    new PosNormalTexcoordVertex(size, size, size, PackF4u(0.0f, 1.0f, 0.0f), 0.0f, 1.0f),
                    new PosNormalTexcoordVertex(-size, size, -size, PackF4u(0.0f, 1.0f, 0.0f), 1.0f, 0.0f),
                    new PosNormalTexcoordVertex(size, size, -size, PackF4u(0.0f, 1.0f, 0.0f), 0.0f, 0.0f),
                    new PosNormalTexcoordVertex(-size, -size, size, PackF4u(0.0f, -1.0f, 0.0f), 1.0f, 1.0f),
                    new PosNormalTexcoordVertex(size, -size, size, PackF4u(0.0f, -1.0f, 0.0f), 0.0f, 1.0f),
                    new PosNormalTexcoordVertex(-size, -size, -size, PackF4u(0.0f, -1.0f, 0.0f), 1.0f, 0.0f),
                    new PosNormalTexcoordVertex(size, -size, -size, PackF4u(0.0f, -1.0f, 0.0f), 0.0f, 0.0f),
                    new PosNormalTexcoordVertex(size, -size, size, PackF4u(0.0f, 0.0f, 1.0f), 0.0f, 0.0f),
                    new PosNormalTexcoordVertex(size, size, size, PackF4u(0.0f, 0.0f, 1.0f), 0.0f, 1.0f),
                    new PosNormalTexcoordVertex(-size, -size, size, PackF4u(0.0f, 0.0f, 1.0f), 1.0f, 0.0f),
                    new PosNormalTexcoordVertex(-size, size, size, PackF4u(0.0f, 0.0f, 1.0f), 1.0f, 1.0f),
                    new PosNormalTexcoordVertex(size, -size, -size, PackF4u(0.0f, 0.0f, -1.0f), 0.0f, 0.0f),
                    new PosNormalTexcoordVertex(size, size, -size, PackF4u(0.0f, 0.0f, -1.0f), 0.0f, 1.0f),
                    new PosNormalTexcoordVertex(-size, -size, -size, PackF4u(0.0f, 0.0f, -1.0f), 1.0f, 0.0f),
                    new PosNormalTexcoordVertex(-size, size, -size, PackF4u(0.0f, 0.0f, -1.0f), 1.0f, 1.0f),
                    new PosNormalTexcoordVertex(size, size, -size, PackF4u(1.0f, 0.0f, 0.0f), 1.0f, 1.0f),
                    new PosNormalTexcoordVertex(size, size, size, PackF4u(1.0f, 0.0f, 0.0f), 0.0f, 1.0f),
                    new PosNormalTexcoordVertex(size, -size, -size, PackF4u(1.0f, 0.0f, 0.0f), 1.0f, 0.0f),
                    new PosNormalTexcoordVertex(size, -size, size, PackF4u(1.0f, 0.0f, 0.0f), 0.0f, 0.0f),
                    new PosNormalTexcoordVertex(-size, size, -size, PackF4u(-1.0f, 0.0f, 0.0f), 1.0f, 1.0f),
                    new PosNormalTexcoordVertex(-size, size, size, PackF4u(-1.0f, 0.0f, 0.0f), 0.0f, 1.0f),
                    new PosNormalTexcoordVertex(-size, -size, -size, PackF4u(-1.0f, 0.0f, 0.0f), 1.0f, 0.0f),
                    new PosNormalTexcoordVertex(-size, -size, size, PackF4u(-1.0f, 0.0f, 0.0f), 0.0f, 0.0f)
                };

                return new VertexBuffer(MemoryBlock.FromArray(vertices), PosNormalTexcoordVertex.Layout);
            }

            public static IndexBuffer CreateIndexBuffer()
            {
                return new IndexBuffer(MemoryBlock.FromArray(indices));
            }
        }
    }
}