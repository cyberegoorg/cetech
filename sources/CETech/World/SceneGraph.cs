using CETech.CEMath;

namespace CETech.World
{
    public partial class SceneGraph
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

		public static int GetNodeByName(int world, int entity, long name)
		{
			return GetNodeByNameImpl (world, entity, name);
		}

		public static int Create(int world, int entity, long[] names, int[] parents, Mat4f[] pose)
        {
            return CreateImpl(world, entity, names, parents, pose);
        }

        public static void Link(int world, int parent_node, int child_node)
        {
            LinkImpl(world, parent_node, child_node);
        }

        public static void SetPosition(int world, int node, Vec3f pos)
        {
            SetPositionImpl(world, node, pos);
        }

        public static void SetRotation(int world, int node, Quatf rot)
        {
            SetRotationImpl(world, node, rot);
        }

        public static void SetScale(int world, int node, Vec3f scale)
        {
            SetScaleImpl(world, node, scale);
        }

        public static Vec3f GetPosition(int world, int node)
        {
            return GetPositionImpl(world, node);
        }

        public static Quatf GetRotation(int world, int node)
        {
            return GetRotationImpl(world, node);
        }

        public static Vec3f GetScale(int world, int node)
        {
            return GetScaleImpl(world, node);
        }

        public static Mat4f GetWorldMatrix(int world, int node)
        {
            return GetWorldMatrixImpl(world, node);
        }
    }
}