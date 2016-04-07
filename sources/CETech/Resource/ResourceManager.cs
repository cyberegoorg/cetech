using System.IO;

namespace CETech.Resource
{
    /// <summary>
    ///     Resource manager
    /// </summary>
    public static partial class ResourceManager
    {
        /// <summary>
        ///     Resource loader delegate
        /// </summary>
        /// <param name="input">Resource input</param>
        public delegate object ResourceLoader(Stream input);

        /// <summary>
        ///     Resource offline delegate.
        /// </summary>
        /// <param name="data">Resource data</param>
        public delegate void ResourceOffline(object data);

        /// <summary>
        ///     Resource online delegate.
        /// </summary>
        /// <param name="data">Resource data</param>
        public delegate void ResourceOnline(object data);

        public delegate object ResourceReloader(long name, object new_data);

        /// <summary>
        ///     Resource unloader delegate.
        /// </summary>
        /// <param name="data">Resource data</param>
        public delegate void ResourceUnloader(object data);

        /// <summary>
        ///     Auto load
        /// </summary>
        public static bool AutoLoad { get; set; }

        public static void InitConfig()
        {
            //AutoLoad = true;
            InitConfigImpl();
        }

        public static void Init()
        {
            InitImpl();
        }

        public static void Shutdown()
        {
            ShutdownImpl();
        }

        /// <summary>
        ///     Register resource type
        /// </summary>
        /// <param name="type">Type name</param>
        /// <param name="loader">Loader delegate</param>
        /// <param name="unloader">Unloader delegate</param>
        /// <param name="online">Online delegate</param>
        /// <param name="offline">Offline delegate</param>
        public static void RegisterType(long type, ResourceLoader loader, ResourceUnloader unloader,
            ResourceOnline online, ResourceOffline offline, ResourceReloader reloader)
        {
            RegisterTypeImpl(type, loader, unloader, online, offline, reloader);
        }

        /// <summary>
        ///     Load resources.
        /// </summary>
        /// <param name="type">Resource type</param>
        /// <param name="names">Resource names</param>
        /// <returns>Loaded resource data</returns>
        public static object[] Load(long type, long[] names)
        {
            return LoadImpl(type, names);
        }

        /// <summary>
        ///     Add loaded resources
        /// </summary>
        /// <param name="loaded_data">Loaded data</param>
        /// <param name="type">Resource type</param>
        /// <param name="names">Resource names</param>
        /// <summary>
        ///     Load resource now.
        /// </summary>
        /// <param name="type">Resource type</param>
        /// <param name="names">Resource names</param>
        public static void LoadNow(long type, long[] names)
        {
            LoadNowImpl(type, names);
        }

        public static void Reload(long type, long[] names)
        {
            ReloadImpl(type, names);
        }

        public static void ReloadAll(long type)
        {
            ReloadAllImpl(type);
        }

        /// <summary>
        ///     Unload resource
        /// </summary>
        /// <param name="type">Resource type</param>
        /// <param name="names">Resource names</param>
        public static void Unload(long type, long[] names)
        {
            UnloadImpl(type, names);
        }

        /// <summary>
        ///     Can get resources?
        /// </summary>
        /// <param name="type">Resource type</param>
        /// <param name="names">Resource names</param>
        /// <returns>True if can</returns>
        public static bool CanGet(long type, long[] names)
        {
            return CanGetImpl(type, names);
        }

        /// <summary>
        ///     Get resource data.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="name"></param>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public static T Get<T>(long type, long name)
        {
            return GetImpl<T>(type, name);
        }
    }
}