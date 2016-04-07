using CETech.CEMath;

namespace CETech.World
{
    public partial class CameraSystem
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

        public static int Create(int world, int entity, float near, float far, float fov)
        {
            return CreateImpl(world, entity, near, far, fov);
        }

        public static void GetProjectView(int world, int camera, out Matrix4f proj, out Matrix4f view)
        {
            GetProjectViewImpl(world, camera, out proj, out view);
        }

        public static int GetCamera(int world, int entity)
        {
            return GetCameraImpl(world, entity);
        }
    }
}