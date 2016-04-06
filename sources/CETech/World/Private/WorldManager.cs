using System.Collections.Generic;

namespace CETech.World
{
    public partial class WorldManager
    {
        private static HandlerID _handlers;

        private static readonly Dictionary<long, List<Levelnstance>> _levelnstances =
            new Dictionary<long, List<Levelnstance>>();

        public static void InitImpl()
        {
            _handlers = new HandlerID();
        }

        public static void ShutdownImpl()
        {
        }

        public static int CreateWorldImpl()
        {
            var id = _handlers.Create();
            TranformationSystem.InitWorld(id);
            PrimitiveMeshRenderer.InitWorld(id);
            CameraSystem.InitWorld(id);
            _levelnstances[id] = new List<Levelnstance>();
            return id;
        }

        private static void DestroyWorldImpl(int world)
        {
            TranformationSystem.RemoveWorld(world);
            PrimitiveMeshRenderer.RemoveWorld(world);
            CameraSystem.RemoveWorld(world);
            _handlers.Destroy(world);
        }

        private static void UpdateImpl(int world, float dt)
        {
            // TODO: ! ! !! !!! !!!!!
        }

        private static int LoadLevelImpl(int world, long level, Vector3f position, Vector3f rotation, Vector3f scale)
        {
            var level_resource = ResourceManager.Get<LevelResource.CompiledResource>(LevelResource.Type, level);
            var level_instance = new Levelnstance();

            var units = level_resource.units;
            var units_name = level_resource.units_name;

            var level_ent = EntityManager.Create();
            level_instance.unit = level_ent;

            TranformationSystem.Create(world, level_ent, int.MaxValue, position, rotation, scale);

            for (var i = 0; i < units.Length; ++i)
            {
                var spawned_unit = UnitManager.Spawn(units[i], world);

                level_instance.units[units_name[i]] = spawned_unit;

                TranformationSystem.Link(world, level_ent, spawned_unit);
            }

            _levelnstances[world].Add(level_instance);

            return _levelnstances[world].Count - 1; // TODO
        }

        private static int UnitByNameImpl(int world, int level, long name)
        {
            var level_instance = _levelnstances[world][level];

            int unit;
            return level_instance.units.TryGetValue(name, out unit) ? unit : 0;
        }

        private static int LevelUnitImpl(int world, int level)
        {
            return _levelnstances[world][level].unit;
        }

        private class Levelnstance
        {
            public readonly Dictionary<long, int> units = new Dictionary<long, int>();
            public int unit;
        }
    }
}