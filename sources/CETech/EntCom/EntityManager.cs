// ReSharper disable once CheckNamespace

namespace CETech.EntCom
{
    /// <summary>
    ///     Entity manager
    /// </summary>
    public partial class EntityManager
    {

        public static readonly int NullEntity = 0;

        /// <summary>
        ///     Init entity manager
        /// </summary>
        public static void Init()
        {
            InitImpl();
        }

        /// <summary>
        ///     Shutdown entity manager.
        /// </summary>
        public static void Shutdown()
        {
            ShutdownImpl();
        }

        /// <summary>
        ///     Create new entity
        /// </summary>
        /// <returns>entity handler</returns>
        public static int Create()
        {
            return CreateImpl();
        }

        /// <summary>
        ///     Destroy entity.
        /// </summary>
        /// <param name="entity">entity handler</param>
        public static void Destroy(int entity)
        {
            DestroyImpl(entity);
        }

        /// <summary>
        ///     Is entity alive?
        /// </summary>
        /// <param name="entity">Entity handler</param>
        /// <returns>True if entity is live else return false</returns>
        public static bool Alive(int entity)
        {
            return AliveImpl(entity);
        }
    }
}