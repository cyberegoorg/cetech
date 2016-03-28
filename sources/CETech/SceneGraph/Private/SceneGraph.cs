using System.Collections.Generic;
using CETech.Develop;
using CETech.Utils;
using MsgPack;
using YamlDotNet.RepresentationModel;

namespace CETech.World
{
    public partial class SceneGraph
    {
        private static Dictionary<int, Dictionary<int, int>> _world_ent_idx = new Dictionary<int, Dictionary<int, int>>();
        private static Dictionary<int, List<Vector3f>> _world_position = new Dictionary<int, List<Vector3f>>();
        private static Dictionary<int, List<Vector3f>> _world_rotaiton = new Dictionary<int, List<Vector3f>>();
        private static Dictionary<int, List<Vector3f>> _world_scale = new Dictionary<int, List<Vector3f>>();

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
        }

        private static int createInstance(int world, int entity, Vector3f position, Vector3f rotation, Vector3f scale)
        {
            var idx = _world_position[world].Count; 
            _world_position[world].Add(position);
            _world_rotaiton[world].Add(rotation);
            _world_scale[world].Add(scale);

            _world_ent_idx[world][entity] = idx;

            Log.Debug("scene_graph", "Create instance [{0} {1} {2}], [{3} {4} {5}], [{6} {7} {8}],",
                position.x, position.y, position.z,
                rotation.x, rotation.y, rotation.z,
                scale.x, scale.y, scale.z);

            return idx;
        }

        private static void Spawner(int world, int[] ent_ids, MessagePackObjectDictionary[] data)
        {

            for (var i = 0; i < ent_ids.Length; ++i)
            {
                var pos = data[i]["position"].AsList();
                var rot = data[i]["rotation"].AsList();
                var sca = data[i]["scale"].AsList();

                Vector3f position = new Vector3f {x = pos[0].AsSingle(), y = pos[1].AsSingle(), z = pos[2].AsSingle()};
                Vector3f rotation = new Vector3f {x = rot[0].AsSingle(), y = rot[1].AsSingle(), z = rot[2].AsSingle()};
                Vector3f scale = new Vector3f {x = sca[0].AsSingle(), y = sca[1].AsSingle(), z = sca[2].AsSingle()};

                createInstance(world, ent_ids[i], position, rotation, scale);
            }
        }

        public static void Compiler(YamlMappingNode body, ConsoleServer.ResponsePacker packer)
        {
            var position = body.Children[new YamlScalarNode("position")] as YamlSequenceNode;
            var rotation = body.Children[new YamlScalarNode("rotation")] as YamlSequenceNode;
            var scale = body.Children[new YamlScalarNode("scale")] as YamlSequenceNode;

            packer.PackMapHeader(3);

            packer.Pack("position");
            packer.PackArrayHeader(3);
            packer.Pack(float.Parse(((YamlScalarNode)position.Children[0]).Value));
            packer.Pack(float.Parse(((YamlScalarNode)position.Children[1]).Value));
            packer.Pack(float.Parse(((YamlScalarNode)position.Children[2]).Value));

            packer.Pack("rotation");
            packer.PackArrayHeader(3);
            packer.Pack(float.Parse(((YamlScalarNode)rotation.Children[0]).Value));
            packer.Pack(float.Parse(((YamlScalarNode)rotation.Children[1]).Value));
            packer.Pack(float.Parse(((YamlScalarNode)rotation.Children[2]).Value));

            packer.Pack("scale");
            packer.PackArrayHeader(3);
            packer.Pack(float.Parse(((YamlScalarNode)scale.Children[0]).Value));
            packer.Pack(float.Parse(((YamlScalarNode)scale.Children[1]).Value));
            packer.Pack(float.Parse(((YamlScalarNode)scale.Children[2]).Value));
        }

        static private void InitImpl()
        {

#if CETECH_DEVELOP
            ComponentSystem.RegisterCompiler(StringId.FromString("transform"), Compiler);
#endif
            ComponentSystem.RegisterSpawnerCompiler(StringId.FromString("transform"), Spawner);
        }

        static private void ShutdownImpl()
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
    }
}