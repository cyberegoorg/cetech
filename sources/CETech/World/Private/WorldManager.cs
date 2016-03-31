namespace CETech.World
{
    public partial class WorldManager
    {
        private static HandlerID _handlers;

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
    }
}