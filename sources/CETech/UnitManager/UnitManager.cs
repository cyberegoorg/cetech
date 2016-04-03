using System.Collections.Generic;
using MsgPack;

namespace CETech
{
    public partial class UnitManager
    {
        public static int Spawn(int world, long unit)
        {
            return SpawnImpl(world, unit);
        }

        public static int Spawn(MessagePackObjectDictionary resource, int world)
        {
            return SpawnImpl(resource, world);
        }

        public static void Init()
        {
            InitImpl();
        }

        public static void Shutdown()
        {
            ShutdownImpl();
        }
    }
}