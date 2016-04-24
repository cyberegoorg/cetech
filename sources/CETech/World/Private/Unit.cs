using CETech.EntCom;
using MsgPack;

namespace CETech.World
{
    public partial class Unit
    {
        private static int SpawnFromResourceImpl(UnitResource.CompiledResource unit_resource, int world)
        {
            var types = unit_resource.type;
            var data = unit_resource.data;
            var ent_list = unit_resource.ent;
            var ent_count = unit_resource.ent_count;

            var ents_parent = unit_resource.ents_parent;
            var entities = new int[ent_count];
            for (var i = 0; i < ent_count; i++)
            {
                entities[i] = EntityManager.Create();
            }

            for (var i = 0; i < types.Length; ++i)
            {
                var ents = ent_list[i];
                var edata = data[i];

                var entities_id = new int[ents.Length];
                var entities_body = new MessagePackObjectDictionary[ents.Length];
                var entities_parent = new int[ents.Length];

                for (var j = 0; j < ents.Length; j++)
                {
                    entities_id[j] = entities[ents[j]];
                    entities_body[j] = edata[j];
                    entities_parent[j] = ents_parent[j];
                }

                ComponentSystem.Spawn(world, types[i], entities_id, entities_parent, entities_body);
            }

            return entities[0];
        }

        private static int SpawnImpl(int world, long unit)
        {
            var unit_resource = Resource.Resource.Get<UnitResource.CompiledResource>(
                UnitResource.Type, unit);

            return SpawnFromResourceImpl(unit_resource, world);
        }

        private static void InitImpl()
        {
        }

        private static void ShutdownImpl()
        {
        }

        public static void Destroy(int world, int unit)
        {
            // TODO: children (track children)
            ComponentSystem.DestroyAllType(world, new[] {unit});
            EntityManager.Destroy(unit);
        }
    }
}