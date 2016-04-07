// ReSharper disable once CheckNamespace
namespace CETech.World
{
    public partial class Unit
    {
        public static void Init()
        {
            InitImpl();
        }

        public static void Shutdown()
        {
            ShutdownImpl();
        }

        public static int Spawn(int world, long unit)
        {
            return SpawnImpl(world, unit);
        }

        public static int SpawnFromResource(UnitResource.CompiledResource resource, int world)
        {
            return SpawnFromResourceImpl(resource, world);
        }

    }
}