using System.Collections.Generic;
using System.Globalization;
using CETech.Develop;
using MsgPack;
using YamlDotNet.RepresentationModel;

namespace CETech.World
{
    public partial class TranformationSystem
    {
        private static readonly Dictionary<int, WorldInstance> _worldInstance = new Dictionary<int, WorldInstance>();

        private static int getIdx(int world, int entity)
        {
            return _worldInstance[world].EntIdx[entity];
        }

        private static void InitWorldImpl(int world)
        {
            _worldInstance[world] = new WorldInstance();
        }

        private static void RemoveWorldImpl(int world)
        {
            _worldInstance.Remove(world);
        }

        private static int CreateInstance(int world, int entity, int parent, Vector3f position, Vector3f rotation,
            Vector3f scale)
        {
            var world_instance = _worldInstance[world];

            var idx = world_instance.Position.Count;
            world_instance.Position.Add(position);
            world_instance.Rotation.Add(rotation);
            world_instance.Scale.Add(scale);

            world_instance.Parent.Add(int.MaxValue);
            world_instance.FirstChild.Add(int.MaxValue);
            world_instance.NextSibling.Add(int.MaxValue);

            world_instance.World.Add(new Matrix4f());

            world_instance.EntIdx[entity] = idx;

            if (parent != int.MaxValue)
            {
                var parentIdx = world_instance.EntIdx[parent];

                world_instance.Parent[idx] = parentIdx;

                if (!is_valid(world_instance.FirstChild[parentIdx]))
                {
                    world_instance.FirstChild[parentIdx] = idx;
                }
                else
                {
                    var firstChildIdx = world_instance.FirstChild[parentIdx];

                    world_instance.FirstChild[parentIdx] = idx;
                    world_instance.NextSibling[idx] = firstChildIdx;
                }

                world_instance.Parent[idx] = parentIdx;
            }

            return idx;
        }

        private static bool is_valid(int idx)
        {
            return idx != int.MaxValue;
        }

        private static void Transform(int world, int idx, Matrix4f parent)
        {
            var world_instance = _worldInstance[world];

            var pos = world_instance.Position[idx];
            var rot = world_instance.Rotation[idx];
            var sca = world_instance.Scale[idx]; // TODO: !!!

            var local = Matrix4f.CreateFromYawPitchRoll(rot.X, rot.Y, rot.Z);
            local.M41 = pos.X;
            local.M42 = pos.Y;
            local.M43 = pos.Z;

            world_instance.World[idx] = local*parent;

            var child = world_instance.FirstChild[idx];

            while (is_valid(child))
            {
                Transform(world, child, world_instance.World[idx]);
                child = world_instance.NextSibling[child];
            }
        }

        private static void Spawner(int world, int[] ent_ids, int[] ents_parent, MessagePackObjectDictionary[] data)
        {
            var world_instance = _worldInstance[world];

            for (var i = 0; i < ent_ids.Length; ++i)
            {
                var pos = data[i]["position"].AsList();
                var rot = data[i]["rotation"].AsList();
                var sca = data[i]["scale"].AsList();

                var position = new Vector3f {X = pos[0].AsSingle(), Y = pos[1].AsSingle(), Z = pos[2].AsSingle()};
                var rotation = new Vector3f {X = rot[0].AsSingle(), Y = rot[1].AsSingle(), Z = rot[2].AsSingle()};
                var scale = new Vector3f {X = sca[0].AsSingle(), Y = sca[1].AsSingle(), Z = sca[2].AsSingle()};

                CreateInstance(world, ent_ids[i],
                    ents_parent[i] != int.MaxValue ? ent_ids[ents_parent[i]] : int.MaxValue, position, rotation, scale);
            }

            Transform(world, world_instance.EntIdx[ent_ids[0]], Matrix4f.Identity);
        }

        private static void Compiler(YamlMappingNode body, ConsoleServer.ResponsePacker packer)
        {
            var position = body.Children[new YamlScalarNode("position")] as YamlSequenceNode;
            var rotation = body.Children[new YamlScalarNode("rotation")] as YamlSequenceNode;
            var scale = body.Children[new YamlScalarNode("scale")] as YamlSequenceNode;

            packer.PackMapHeader(3);

            packer.Pack("position");
            packer.PackArrayHeader(3);
            packer.Pack(float.Parse(((YamlScalarNode) position.Children[0]).Value, CultureInfo.InvariantCulture));
            packer.Pack(float.Parse(((YamlScalarNode) position.Children[1]).Value, CultureInfo.InvariantCulture));
            packer.Pack(float.Parse(((YamlScalarNode) position.Children[2]).Value, CultureInfo.InvariantCulture));

            packer.Pack("rotation");
            packer.PackArrayHeader(3);
            packer.Pack(float.Parse(((YamlScalarNode) rotation.Children[0]).Value, CultureInfo.InvariantCulture));
            packer.Pack(float.Parse(((YamlScalarNode) rotation.Children[1]).Value, CultureInfo.InvariantCulture));
            packer.Pack(float.Parse(((YamlScalarNode) rotation.Children[2]).Value, CultureInfo.InvariantCulture));

            packer.Pack("scale");
            packer.PackArrayHeader(3);
            packer.Pack(float.Parse(((YamlScalarNode) scale.Children[0]).Value, CultureInfo.InvariantCulture));
            packer.Pack(float.Parse(((YamlScalarNode) scale.Children[1]).Value, CultureInfo.InvariantCulture));
            packer.Pack(float.Parse(((YamlScalarNode) scale.Children[2]).Value, CultureInfo.InvariantCulture));
        }

        private static void InitImpl()
        {
#if CETECH_DEVELOP
            ComponentSystem.RegisterCompiler(StringId.FromString("transform"), Compiler);
#endif
            ComponentSystem.RegisterSpawnerCompiler(StringId.FromString("transform"), Spawner);
        }

        private static void ShutdownImpl()
        {
        }

        private static Vector3f GetPositionImpl(int world, int entity)
        {
            var world_instance = _worldInstance[world];
            return world_instance.Position[getIdx(world, entity)];
        }

        private static Vector3f GetRotationImpl(int world, int entity)
        {
            var world_instance = _worldInstance[world];
            return world_instance.Rotation[getIdx(world, entity)];
        }

        private static Vector3f GetScaleImpl(int world, int entity)
        {
            var world_instance = _worldInstance[world];
            return world_instance.Scale[getIdx(world, entity)];
        }

        private static Matrix4f GetWorldMatrixImpl(int world, int entity)
        {
            var world_instance = _worldInstance[world];
            return world_instance.World[getIdx(world, entity)];
        }

        private static void SetPositionImpl(int world, int entity, Vector3f pos)
        {
            var world_instance = _worldInstance[world];
            var parent_idx = world_instance.Parent[getIdx(world, entity)];
            var parent = parent_idx != int.MaxValue ? world_instance.World[parent_idx] : Matrix4f.Identity;

            world_instance.Position[getIdx(world, entity)] = pos;
            Transform(world, getIdx(world, entity), GetWorldMatrix(world, entity));
        }

        private static void SetRotationImpl(int world, int entity, Vector3f rot)
        {
            var world_instance = _worldInstance[world];
            var parent_idx = world_instance.Parent[getIdx(world, entity)];
            var parent = parent_idx != int.MaxValue ? world_instance.World[parent_idx] : Matrix4f.Identity;

            world_instance.Rotation[getIdx(world, entity)] = rot;
            Transform(world, getIdx(world, entity), parent);
        }

        private static void SetScaleImpl(int world, int entity, Vector3f scale)
        {
            var world_instance = _worldInstance[world];
            var parent_idx = world_instance.Parent[getIdx(world, entity)];
            var parent = parent_idx != int.MaxValue ? world_instance.World[parent_idx] : Matrix4f.Identity;

            world_instance.Scale[getIdx(world, entity)] = scale;
            Transform(world, getIdx(world, entity), parent);
        }

        private class WorldInstance
        {
            public readonly Dictionary<int, int> EntIdx;
            public readonly List<int> FirstChild;
            public readonly List<int> NextSibling;

            public readonly List<int> Parent;

            public readonly List<Vector3f> Position;
            public readonly List<Vector3f> Rotation;
            public readonly List<Vector3f> Scale;

            public readonly List<Matrix4f> World;

            public WorldInstance()
            {
                NextSibling = new List<int>();
                EntIdx = new Dictionary<int, int>();
                Position = new List<Vector3f>();
                Rotation = new List<Vector3f>();
                Scale = new List<Vector3f>();
                Parent = new List<int>();
                FirstChild = new List<int>();
                World = new List<Matrix4f>();
            }
        }
    }
}