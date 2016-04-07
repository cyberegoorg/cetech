namespace CETech.World
{
    public partial class UnitManager
    {
        public static int Spawn(int world, long unit)
        {
            return SpawnImpl(world, unit);
        }

        public static int SpawnFromResource(UnitResource.CompiledResource resource, int world)
        {
            return SpawnFromResourceImpl(resource, world);
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