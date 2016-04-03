namespace CETech.World
{
    public partial class PrimitiveMeshRenderer
    {
        public static void Init()
        {
            InitImpl();
        }

        public static void Shutdown()
        {
            ShutdownImpl();
        }

        public static void InitWorld(int world)
        {
            InitWorldImpl(world);
        }

        public static void RemoveWorld(int world)
        {
            RemoveWorldImpl(world);
        }

        public static void RenderWorld(int world)
        {
            RenderWorldImpl(world);
        }
    }
}