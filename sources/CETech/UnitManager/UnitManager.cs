namespace CETech
{
    public partial class UnitManager
    {
        public static int Spawn(int world, long unit)
        {
            return SpawnImpl(world, unit);
        }
    }
}