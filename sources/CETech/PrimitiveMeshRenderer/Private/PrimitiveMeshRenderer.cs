using System.Collections.Generic;
using System.Globalization;
using CETech.Develop;
using MsgPack;
using SharpBgfx;
using YamlDotNet.RepresentationModel;

namespace CETech.World
{
    public partial class PrimitiveMeshRenderer

    {
        private static readonly Dictionary<int, Dictionary<int, float>> _world_ent_idx = new Dictionary<int, Dictionary<int, float>>();
        private static readonly Dictionary<float, VertexBuffer> _size_vb = new Dictionary<float, VertexBuffer>();
        private static IndexBuffer _size_ib;

        private static void InitWorldImpl(int world)
        {
            _size_ib = Cube.CreateIndexBuffer();

            _world_ent_idx[world] = new Dictionary<int, float>();
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
            }
        }

        private static void Compiler(YamlMappingNode body, ConsoleServer.ResponsePacker packer)
        {
            var size = body.Children[new YamlScalarNode("size")] as YamlScalarNode;

            packer.PackMapHeader(1);

            packer.Pack("size");
            packer.Pack(float.Parse(size.Value, CultureInfo.InvariantCulture));
        }

        private static void InitImpl()
        {

#if CETECH_DEVELOP
            ComponentSystem.RegisterCompiler(StringId.FromString("primitive_mesh"), Compiler);
#endif
            ComponentSystem.RegisterSpawnerCompiler(StringId.FromString("primitive_mesh"), Spawner);
        }

        private static void ShutdownImpl()
        {
        }

        public struct PosColorVertex
        {
            private float x;
            private float y;
            private float z;
            private uint abgr;

            public PosColorVertex(float x, float y, float z, uint abgr)
            {
                this.x = x;
                this.y = y;
                this.z = z;
                this.abgr = abgr;
            }

            public static readonly VertexLayout Layout = new VertexLayout().Begin()
                .Add(VertexAttributeUsage.Position, 3, VertexAttributeType.Float)
                .Add(VertexAttributeUsage.Color0, 4, VertexAttributeType.UInt8, true)
                .End();
        }

        public static void RenderWorldImpl(int world)


        {
            foreach (var f in _world_ent_idx[world])
            {
                var world_matrix = TranformationSystem.GetWorldMatrix(world, f.Key);
                unsafe
                {
                    Bgfx.SetTransform(&world_matrix.M11);
                }

                // set pipeline states
                Bgfx.SetVertexBuffer(_size_vb[f.Value]);
                Bgfx.SetIndexBuffer(_size_ib);
                Bgfx.SetRenderState(RenderState.Default);

                // submit primitives
                Bgfx.Submit(0, ResourceManager.Get<MaterialResource.MaterialInstance>(MaterialResource.Type, StringId.FromString("material1")).instance.program);

            }

        }

        public static class Cube
        {
            private static readonly ushort[] indices =
            {
                0, 1, 2, // 0
                1, 3, 2,
                4, 6, 5, // 2
                5, 6, 7,
                0, 2, 4, // 4
                4, 2, 6,
                1, 5, 3, // 6
                5, 7, 3,
                0, 4, 1, // 8
                4, 5, 1,
                2, 3, 6, // 10
                6, 3, 7
            };

            public static VertexBuffer CreateVertexBuffer(float size)
            {
                PosColorVertex[] vertices =
                {
                    new PosColorVertex(-size, size, size, 0xff000000),
                    new PosColorVertex(size, size, size, 0xff0000ff),
                    new PosColorVertex(-size, -size, size, 0xff00ff00),
                    new PosColorVertex(size, -size, size, 0xff00ffff),
                    new PosColorVertex(-size, size, -size, 0xffff0000),
                    new PosColorVertex(size, size, -size, 0xffff00ff),
                    new PosColorVertex(-size, -size, -size, 0xffffff00),
                    new PosColorVertex(size, -size, -size, 0xffffffff)
                };

                return new VertexBuffer(MemoryBlock.FromArray(vertices), PosColorVertex.Layout);
            }

            public static IndexBuffer CreateIndexBuffer()
            {
                return new IndexBuffer(MemoryBlock.FromArray(indices));
            }
        }
    }
}