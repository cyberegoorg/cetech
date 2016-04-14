using System.Collections.Generic;
using System.Globalization;
using System.Yaml;
using CETech.CEMath;
using CETech.Develop;
using CETech.EntCom;
using CETech.Resource;
using MsgPack;

namespace CETech.World
{
    public partial class CameraSystem
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

        private static int CreateImpl(int world, int entity, float near, float far, float fov)
        {
            var world_instance = _worldInstance[world];

            var idx = world_instance.Near.Count;
            world_instance.Ent.Add(entity);
            world_instance.Near.Add(near);
            world_instance.Far.Add(far);
            world_instance.Fov.Add(fov);
            world_instance.Tranform.Add(Tranform.GetTranform(world, entity));

            world_instance.EntIdx[entity] = idx;
            return idx;
        }

        private static bool is_valid(int idx)
        {
            return idx != int.MaxValue;
        }

        private static void Spawner(int world, int[] ent_ids, int[] ents_parent, MessagePackObjectDictionary[] data)
        {
            for (var i = 0; i < ent_ids.Length; ++i)
            {
                var near = data[i]["near"].AsSingle();
                var far = data[i]["far"].AsSingle();
                var fov = data[i]["fov"].AsSingle();

                Create(world, ent_ids[i], near, far, fov);
            }
        }

        private static float ToFloat(YamlScalar scalar)
        {
            return float.Parse(scalar.Value, CultureInfo.InvariantCulture);
        }

        private static void Compiler(YamlMapping body, ConsoleServer.ResponsePacker packer)
        {
            var near = body["near"] as YamlScalar;
            var far = body["far"] as YamlScalar;
            var fov = body["fov"] as YamlScalar;

            packer.PackMapHeader(3);

            packer.Pack("near");
            packer.Pack(ToFloat(near));

            packer.Pack("far");
            packer.Pack(ToFloat(far));

            packer.Pack("fov");
            packer.Pack(ToFloat(fov));
        }

        private static void InitImpl()
        {
#if CETECH_DEVELOP
            ComponentSystem.RegisterCompiler(StringId.FromString("camera"), Compiler, 10);
#endif
            ComponentSystem.RegisterType(StringId.FromString("camera"), Spawner, Destroyer);
        }

        private static void Destroyer(int world, int[] entIds)
        {
            var world_instance = _worldInstance[world];

            for (int i = 0; i < entIds.Length; i++)
            {
                var ent_id = entIds[i];

                if (!world_instance.EntIdx.ContainsKey(ent_id))
                {
                    continue;
                }

                var item_idx = getIdx(world, ent_id);
                var last_idx = world_instance.Near.Count-1;
                var last_ent = world_instance.Ent[last_idx];

                world_instance.EntIdx.Remove(ent_id);

                world_instance.Near[item_idx] = world_instance.Tranform[last_idx];
                world_instance.Far[item_idx] = world_instance.Tranform[last_idx];
                world_instance.Fov[item_idx] = world_instance.Tranform[last_idx];
                world_instance.Tranform[item_idx] = world_instance.Tranform[last_idx];
                world_instance.EntIdx[item_idx] = world_instance.Tranform[last_idx];

                world_instance.EntIdx[last_ent] = item_idx;

                world_instance.Ent.RemoveAt(last_idx);
                world_instance.Near.RemoveAt(last_idx);
                world_instance.Far.RemoveAt(last_idx);
                world_instance.Fov.RemoveAt(last_idx);
                world_instance.Tranform.RemoveAt(last_idx);
            }
        }

        private static void ShutdownImpl()
        {
        }

        private static void GetProjectViewImpl(int world, int camera, out Mat4f proj, out Mat4f view)
        {
            var world_instance = _worldInstance[world];
			var size = Renderer.GetSize ();

            view = Tranform.GetWorldMatrix(world, world_instance.Tranform[camera]);
			proj = Mat4f.CreatePerspectiveFieldOfView(MathUtils.DegToRad(world_instance.Fov[camera]), size.X/size.Y,
                world_instance.Near[camera], world_instance.Far[camera]);
        }

        private static int GetCameraImpl(int world, int entity)
        {
            return _worldInstance[world].EntIdx[entity];
        }

        private class WorldInstance
        {
            public readonly Dictionary<int, int> EntIdx;
            public readonly List<int> Ent;
            public readonly List<float> Far;
            public readonly List<float> Fov;

            public readonly List<float> Near;
            public readonly List<int> Tranform;

            public WorldInstance()
            {
                EntIdx = new Dictionary<int, int>();

                Ent = new List<int>();
                Near = new List<float>();
                Far = new List<float>();
                Fov = new List<float>();
                Tranform = new List<int>();
            }
        }
    }
}