using System.Diagnostics;

namespace CETech.EntCom
{
    public partial class EntityManager
    {
        private static HandlerID _hadlers;

        private static void InitImpl()
        {
            _hadlers = new HandlerID();
            Debug.Assert(_hadlers.Create() == NullEntity);
        }

        private static void ShutdownImpl()
        {
        }

        public static int CreateImpl()
        {
            return _hadlers.Create();
        }

        public static void DestroyImpl(int entity)
        {
            _hadlers.Destroy(entity);
        }

        public static bool AliveImpl(int entity)
        {
            return _hadlers.Alive(entity);
        }
    }
}