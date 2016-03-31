namespace CETech.World
{
    public partial class TranformationSystem
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

        public static void SetPosition(int world, int entity, Vector3f pos)
        {
            SetPositionImpl(world, entity, pos);
        }

        public static void SetRotation(int world, int entity, Vector3f rot)
        {
            SetRotationImpl(world, entity, rot);
        }

        public static void SetScale(int world, int entity, Vector3f scale)
        {
            SetScaleImpl(world, entity, scale);
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

        public static Matrix4f GetWorldMatrix(int world, int entity)
        {
            return GetWorldMatrixImpl(world, entity);
        }
    }
}