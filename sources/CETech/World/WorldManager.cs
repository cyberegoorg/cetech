namespace CETech.World
{
    public partial class WorldManager
    {
        public static void Init()
        {
            InitImpl();
        }

        public static void Shutdown()
        {
            ShutdownImpl();
        }

        public static int CreateWorld()
        {
            return CreateWorldImpl();
        }

        public static void DestroyWorld(int world)
        {
            DestroyWorldImpl(world);
        }
    }
}