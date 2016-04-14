using System.Collections.Generic;
using System.Diagnostics;
using CETech.CEMath;
using CETech.EntCom;
using CETech.Resource;

namespace CETech.World
{
    public partial class World
    {
        private static HandlerID _handlers;

        private static Dictionary<int, int> _WorldIdx;

        private static List<int> _WordlId;
        private static List<List<int>> _SpawnedUnits;
        private static List<Levelnstance> _levelnstances;


        public static void InitImpl()
        {
            _handlers = new HandlerID();

            _WorldIdx = new Dictionary<int, int>();
            _SpawnedUnits = new List<List<int>>();
            _WordlId = new List<int>();
            _levelnstances = new List<Levelnstance>();

            Debug.Assert(_handlers.Create() == NullWorld);
        }

        public static void ShutdownImpl()
        {
        }

        public static int CreateWorldImpl()
        {
            var id = _handlers.Create();

            var idx = _WorldIdx.Count;

            _WorldIdx[id] = idx;
            _SpawnedUnits.Add(new List<int>());
            _WordlId.Add(id);
            _levelnstances.Add(new Levelnstance());

            Tranform.InitWorld(id);
            PrimitiveMeshRenderer.InitWorld(id);
            CameraSystem.InitWorld(id);

            return id;
        }

        private static int getIdx(int world)
        {
            return _WorldIdx[world];
        }

        private static void DestroyWorldImpl(int world)
        {
            Tranform.RemoveWorld(world);
            PrimitiveMeshRenderer.RemoveWorld(world);
            CameraSystem.RemoveWorld(world);

            var item_idx = getIdx(world);
            var last_idx = _WorldIdx.Count - 1;
            var last_id = _WordlId[last_idx];

            _WorldIdx.Remove(world);

            // Move value from last to actual
            _SpawnedUnits[item_idx] = _SpawnedUnits[last_idx];
            _WordlId[item_idx] = _WordlId[last_idx];

            // Remove last
            _SpawnedUnits.RemoveAt(last_idx);
            _WordlId.RemoveAt(last_idx);

            _WorldIdx[last_id] = item_idx;

            _handlers.Destroy(world);
        }

        private static void UpdateImpl(int world, float dt)
        {
            // TODO: ! ! !! !!! !!!!!
        }

        private static int LoadLevelImpl(int world, long level, Vec3f position, Vec3f rotation, Vec3f scale)
        {
            var world_idx = getIdx(world);

            var level_instance = new Levelnstance();

            var level_resource = Resource.Resource.Get<LevelResource.CompiledResource>(LevelResource.Type, level);


            var units = level_resource.units;
            var units_name = level_resource.units_name;

            var level_ent = EntityManager.Create();
            level_instance.unit = level_ent;

            Tranform.Create(world, level_ent, int.MaxValue, position, rotation, scale);

            for (var i = 0; i < units.Length; ++i)
            {
                var spawned_unit = Unit.SpawnFromResource(units[i], world);

                level_instance.units[units_name[i]] = spawned_unit;

                Tranform.Link(world, level_ent, spawned_unit);
            }

            _levelnstances[world_idx] = level_instance;

            return _levelnstances.Count - 1; // TODO
        }

        private static int UnitByNameImpl(int world, int level, long name)
        {
            var idx = getIdx(world);

            var level_instance = _levelnstances[idx];

            int unit;
            return level_instance.units.TryGetValue(name, out unit) ? unit : EntityManager.NullEntity;
        }

        private static int LevelUnitImpl(int world, int level)
        {
            var idx = getIdx(world);
            return _levelnstances[idx].unit;
        }

        private class Levelnstance
        {
            public readonly Dictionary<long, int> units = new Dictionary<long, int>();
            public int unit;
            public int[] spawned;
        }
    }
}