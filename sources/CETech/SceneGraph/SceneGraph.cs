namespace CETech.World
{
    public partial class SceneGraph
    {
        static public void Init()
        {
            InitImpl();
        }

        static public void Shutdown()
        {
            ShutdownImpl();
        }

        public static void InitWorld(int world)
        {
            InitWorldImpl(world);
        }

        public static Vector3f GetPosition(int world, int entity)
        {
            return GetPositionImpl(world, entity);
        }

        public static Vector3f GetRotation(int world, int entity)
        {
            return GetRotationImpl(world, entity);
        }

        public static Vector3f GetScale(int world, int entity)
        {
            return GetScaleImpl(world, entity);
        }
    }
}