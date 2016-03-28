using System.Collections.Generic;
using CETech.World;
using MsgPack;

namespace CETech
{
    public partial class UnitManager
    {
        private static int SpawnImpl(int world, long unit)
        {
            var unit_resource = ResourceManager.Get<Dictionary<MessagePackObject, MessagePackObject>>(
                UnitResource.Type, unit);

            var entity = EntityManager.Create();

            var types = unit_resource["type"].AsList();
            var data = unit_resource["data"].AsList();

            for (var i = 0; i < types.Count; ++i)
            {
                ComponentSystem.Spawn(world, types[i].AsInt64(), new[] {entity}, new[] {data[i].AsDictionary()});
            }

            return entity;
        }
    }
}