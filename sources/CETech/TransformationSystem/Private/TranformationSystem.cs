using System;
using System.Collections.Generic;
using System.Globalization;
using CETech.Develop;
using CETech.Utils;
using MsgPack;
using YamlDotNet.RepresentationModel;

namespace CETech.World
{
    public partial class TranformationSystem
    {
        private static readonly Dictionary<int, Dictionary<int, int>> _world_ent_idx =
            new Dictionary<int, Dictionary<int, int>>();

        private static readonly Dictionary<int, List<Vector3f>> _world_position = new Dictionary<int, List<Vector3f>>();
        private static readonly Dictionary<int, List<Vector3f>> _world_rotaiton = new Dictionary<int, List<Vector3f>>();
        private static readonly Dictionary<int, List<Vector3f>> _world_scale = new Dictionary<int, List<Vector3f>>();

        private static readonly Dictionary<int, List<int>> _world_parent = new Dictionary<int, List<int>>();
        private static readonly Dictionary<int, List<int>> _world_first_child = new Dictionary<int, List<int>>();
        private static readonly Dictionary<int, List<int>> _world_next_sibling = new Dictionary<int, List<int>>();

        private static readonly Dictionary<int, List<Matrix4f>> _world_world = new Dictionary<int, List<Matrix4f>>();

        private static int getIdx(int world, int entity)
        {
            return _world_ent_idx[world][entity];
        }

        private static void InitWorldImpl(int world)
        {
            _world_ent_idx[world] = new Dictionary<int, int>();
            _world_position[world] = new List<Vector3f>();
            _world_rotaiton[world] = new List<Vector3f>();
            _world_scale[world] = new List<Vector3f>();

            _world_parent[world] = new List<int>();
            _world_first_child[world] = new List<int>();
            _world_next_sibling[world] = new List<int>();

            _world_world[world] = new List<Matrix4f>();
        }

        private static void RemoveWorldImpl(int world)
        {
            _world_ent_idx.Remove(world);
            _world_position.Remove(world);
            _world_rotaiton.Remove(world);
            _world_scale.Remove(world);
        }

        private static int CreateInstance(int world, int entity, int parent, Vector3f position, Vector3f rotation, Vector3f scale)
        {
            var idx = _world_position[world].Count;
            _world_position[world].Add(position);
            _world_rotaiton[world].Add(rotation);
            _world_scale[world].Add(scale);

            _world_parent[world].Add(Int32.MaxValue);
            _world_first_child[world].Add(Int32.MaxValue);
            _world_next_sibling[world].Add(Int32.MaxValue);

            _world_world[world].Add(new Matrix4f());

            _world_ent_idx[world][entity] = idx;

            
            Log.Debug("scene_graph", "Create instance [{0} {1} {2}], [{3} {4} {5}], [{6} {7} {8}],",
                position.X, position.Y, position.Z,
                rotation.X, rotation.Y, rotation.Z,
                scale.X, scale.Y, scale.Z);
            
            if (parent != Int32.MaxValue)
            {
                var parent_idx = _world_ent_idx[world][(int) parent];

                _world_parent[world][idx] = parent_idx;

                if (!is_valid(_world_first_child[world][parent_idx]))
                {
                    _world_first_child[world][parent_idx] = idx;
                }
                else
                {
                    var first_child_idx = _world_first_child[world][parent_idx];

                    _world_first_child[world][parent_idx] = idx;
                    _world_next_sibling[world][idx] = first_child_idx;
                }

                _world_parent[world][idx] = parent_idx;
            }

            return idx;
        }

        private static bool is_valid(int idx)
        {
            return idx != int.MaxValue;
        }

        public static void Transform(int world, int idx, Matrix4f parent)
        {
            var pos = _world_position[world][idx];
            var rot = _world_rotaiton[world][idx];
            var sca = _world_scale[world][idx];

            var local = Matrix4f.CreateFromYawPitchRoll(rot.X, rot.Y, rot.Z);
            local.M41 = pos.X; local.M42 = pos.Y; local.M43 = pos.Z;

            _world_world[world][idx] = local * parent;

            var child = _world_first_child[world][idx];

            while (is_valid(child))
            {
                Transform(world, child, _world_world[world][idx]);
                child = _world_next_sibling[world][child];
            }
        }

        private static void Spawner(int world, int[] ent_ids, int[] ents_parent, MessagePackObjectDictionary[] data)
        {
            for (var i = 0; i < ent_ids.Length; ++i)
            {
                var pos = data[i]["position"].AsList();
                var rot = data[i]["rotation"].AsList();
                var sca = data[i]["scale"].AsList();

                var position = new Vector3f {X = pos[0].AsSingle(), Y = pos[1].AsSingle(), Z = pos[2].AsSingle()};
                var rotation = new Vector3f {X = rot[0].AsSingle(), Y = rot[1].AsSingle(), Z = rot[2].AsSingle()};
                var scale = new Vector3f {X = sca[0].AsSingle(), Y = sca[1].AsSingle(), Z = sca[2].AsSingle()};

                CreateInstance(world, ent_ids[i], (ents_parent[i] != Int32.MaxValue) ? ent_ids[ents_parent[i]]: Int32.MaxValue, position, rotation, scale);
            }

            Transform(world, _world_ent_idx[world][ent_ids[0]], Matrix4f.Identity);
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
            return _world_position[world][getIdx(world, entity)];
        }

        private static Vector3f GetRotationImpl(int world, int entity)
        {
            return _world_rotaiton[world][getIdx(world, entity)];
        }

        private static Vector3f GetScaleImpl(int world, int entity)
        {
            return _world_scale[world][getIdx(world, entity)];
        }

        private static Matrix4f GetWorldMatrixImpl(int world, int entity)
        {
            return _world_world[world][getIdx(world, entity)];
        }

        private static void SetPositionImpl(int world, int entity, Vector3f pos)
        {
            var parent_idx = _world_parent[world][getIdx(world, entity)];
            Matrix4f parent =  parent_idx != Int32.MaxValue ? _world_world[world][parent_idx] : Matrix4f.Identity;

            _world_position[world][getIdx(world, entity)] = pos;
            Transform(world, getIdx(world,entity), GetWorldMatrix(world, entity));
        }

        private static void SetRotationImpl(int world, int entity, Vector3f rot)
        {
            var parent_idx = _world_parent[world][getIdx(world, entity)];
            Matrix4f parent = parent_idx != Int32.MaxValue ? _world_world[world][parent_idx] : Matrix4f.Identity;

            _world_rotaiton[world][getIdx(world, entity)] = rot;
            Transform(world, getIdx(world, entity), parent);
        }

        private static void SetScaleImpl(int world, int entity, Vector3f scale)
        {
            var parent_idx = _world_parent[world][getIdx(world, entity)];
            Matrix4f parent = parent_idx != Int32.MaxValue ? _world_world[world][parent_idx] : Matrix4f.Identity;

            _world_scale[world][getIdx(world, entity)] = scale;
            Transform(world, getIdx(world, entity), parent);
        }
    }
}