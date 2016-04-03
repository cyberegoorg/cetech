using System;
using System.Collections.Generic;
using MsgPack;

namespace CETech.World
{
    public partial class WorldManager
    {
        private static HandlerID _handlers;

        private static Dictionary<long, List<Levelnstance>> _levelnstances = new Dictionary<long,List<Levelnstance>>();

        private class Levelnstance
        {
            public readonly Dictionary<long, int> units = new Dictionary<long, int>();
        }

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
            _levelnstances[id] = new List<Levelnstance>();
            return id;
        }

        private static void DestroyWorldImpl(int world)
        {
            TranformationSystem.RemoveWorld(world);
            PrimitiveMeshRenderer.RemoveWorld(world);
            _handlers.Destroy(world);
        }

        private static void UpdateImpl(int world)
        {
        }

        private static int LoadLevelImpl(int world, long level, Vector3f position, Vector3f rotation, Vector3f scale)
        {
            var level_resource = ResourceManager.Get<MessagePackObjectDictionary>(LevelResource.Type, level);
            var level_instance = new Levelnstance();

            var units = level_resource["units"].AsList();
            var units_name = level_resource["units_name"].AsList();

            var level_ent = EntityManager.Create();
            TranformationSystem.Create(world, level_ent, Int32.MaxValue, position, rotation, scale);

            for (var i = 0; i < units.Count; ++i)
            {
                var spawned_unit = UnitManager.Spawn(units[i].AsDictionary(), world);

                level_instance.units[units_name[i].AsInt64()] = spawned_unit;

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
            throw new NotImplementedException();
        }
    }
}