using CETech.CEMath;

namespace CETech.World
{
    public partial class Tranform
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

        public static int GetTranform(int world, int entity)
        {
            return GetTranformImpl(world, entity);
        }

        public static int Create(int world, int entity, int parent, Vec3f position, Quatf rotation,
            Vec3f scale)
        {
            return CreateImpl(world, entity, parent, position, rotation, scale);
        }

        public static void Link(int world, int parent_ent, int child_ent)
        {
            LinkImpl(world, parent_ent, child_ent);
        }

        public static void SetPosition(int world, int transform, Vec3f pos)
        {
            SetPositionImpl(world, transform, pos);
        }

        public static void SetRotation(int world, int transform, Quatf rot)
        {
            SetRotationImpl(world, transform, rot);
        }

        public static void SetScale(int world, int transform, Vec3f scale)
        {
            SetScaleImpl(world, transform, scale);
        }

        public static Vec3f GetPosition(int world, int transform)
        {
            return GetPositionImpl(world, transform);
        }

        public static Quatf GetRotation(int world, int transform)
        {
            return GetRotationImpl(world, transform);
        }

        public static Vec3f GetScale(int world, int transform)
        {
            return GetScaleImpl(world, transform);
        }

        public static Mat4f GetWorldMatrix(int world, int transform)
        {
            return GetWorldMatrixImpl(world, transform);
        }
    }
}