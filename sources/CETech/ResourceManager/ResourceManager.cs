using System.Collections.Generic;
using System.IO;
using System.Threading;
using CETech.Utils;

namespace CETech
{
    /// <summary>
    ///     Resource manager
    /// </summary>
    public static class ResourceManager
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

        /// <summary>
        ///     Resource unloader delegate.
        /// </summary>
        /// <param name="data">Resource data</param>
        public delegate void ResourceUnloader(object data);

        private static readonly Dictionary<long, ResourceLoader> LoaderMap =
            new Dictionary<long, ResourceLoader>();

        private static readonly Dictionary<long, ResourceUnloader> UnloaderMap =
            new Dictionary<long, ResourceUnloader>();

        private static readonly Dictionary<long, ResourceOnline> OnlineMap =
            new Dictionary<long, ResourceOnline>();

        private static readonly Dictionary<long, ResourceOffline> OfflineMap =
            new Dictionary<long, ResourceOffline>();

        private static readonly Dictionary<long, int> TypesMap = new Dictionary<long, int>();

        private static readonly List<Dictionary<long, object>> DataMap =
            new List<Dictionary<long, object>>();

        private static readonly List<Dictionary<long, int>> RefMap = new List<Dictionary<long, int>>();

        private static SpinLock _addLock = new SpinLock();

        /// <summary>
        ///     Auto load
        /// </summary>
        public static bool AutoLoad { get; set; }

        /// <summary>
        ///     Register resource type
        /// </summary>
        /// <param name="type">Type name</param>
        /// <param name="loader">Loader delegate</param>
        /// <param name="unloader">Unloader delegate</param>
        /// <param name="online">Online delegate</param>
        /// <param name="offline">Offline delegate</param>
        public static void RegisterType(long type, ResourceLoader loader, ResourceUnloader unloader,
            ResourceOnline online, ResourceOffline offline)
        {
            var idx = DataMap.Count;
            DataMap.Add(new Dictionary<long, object>());
            RefMap.Add(new Dictionary<long, int>());

            TypesMap[type] = idx;

            LoaderMap[type] = loader;
            UnloaderMap[type] = unloader;
            OnlineMap[type] = online;
            OfflineMap[type] = offline;
        }

        /// <summary>
        ///     Load resources.
        /// </summary>
        /// <param name="type">Resource type</param>
        /// <param name="names">Resource names</param>
        /// <returns>Loaded resource data</returns>
        public static object[] Load(long type, long[] names)
        {
            var data = new object[names.Length];

            for (var i = 0; i < names.Length; i++)
            {
                Log.Debug("resource_manager", "Loading resource {0:x}{1:x}", type, names[i]);

                var input = FileSystem.Open("build", string.Format("{0:x}{1:x}", type, names[i]),
                    FileSystem.OpenMode.Read);
                data[i] = LoaderMap[type](input);
            }

            return data;
        }

        /// <summary>
        ///     Add loaded resources
        /// </summary>
        /// <param name="loaded_data">Loaded data</param>
        /// <param name="type">Resource type</param>
        /// <param name="names">Resource names</param>
        public static void AddLoaded(object[] loaded_data, long type, long[] names)
        {
            var online = OnlineMap[type];
            var idx = TypesMap[type];

            var gotLock = false;
            try
            {
                _addLock.Enter(ref gotLock);

                for (var i = 0; i < names.Length; i++)
                {
                    IncRef(idx, names[i]);

                    DataMap[idx][names[i]] = loaded_data[i];

                    online(DataMap[idx][names[i]]);
                }
            }
            finally
            {
                if (gotLock) _addLock.Exit();
            }
        }

        /// <summary>
        ///     Load resource now.
        /// </summary>
        /// <param name="type">Resource type</param>
        /// <param name="names">Resource names</param>
        public static void LoadNow(long type, long[] names)
        {
            var loaded_data = Load(type, names);
            AddLoaded(loaded_data, type, names);
        }


        /// <summary>
        ///     Unload resource
        /// </summary>
        /// <param name="type">Resource type</param>
        /// <param name="names">Resource names</param>
        public static void Unload(long type, long[] names)
        {
            var offline = OfflineMap[type];
            var unloader = UnloaderMap[type];

            var idx = TypesMap[type];

            for (var i = 0; i < names.Length; i++)
            {
                offline(DataMap[idx][names[i]]);
            }

            for (var i = 0; i < names.Length; i++)
            {
                if (DecRef(idx, names[i]))
                {
                    unloader(DataMap[idx][names[i]]);

                    DataMap[idx].Remove(names[i]);
                    RefMap[idx].Remove(names[i]);
                }
            }
        }

        /// <summary>
        ///     Can get resources?
        /// </summary>
        /// <param name="type">Resource type</param>
        /// <param name="names">Resource names</param>
        /// <returns>True if can</returns>
        public static bool CanGet(long type, long[] names)
        {
            var idx = TypesMap[type];
            for (var i = 0; i < names.Length; i++)
            {
                if (!DataMap[idx].ContainsKey(names[i]))
                {
                    return false;
                }
            }

            return true;
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
            long[] names = {name};

            if (AutoLoad && !CanGet(type, names))
            {
                LoadNow(type, names);
            }

            var idx = TypesMap[type];
            return (T) DataMap[idx][name];
        }

        private static void IncRef(int type_idx, long name)
        {
            int counter;

            if (!RefMap[type_idx].TryGetValue(name, out counter))
            {
                counter = 0;
            }

            RefMap[type_idx][name] = counter + 1;
        }

        private static bool DecRef(int type_idx, long name)
        {
            RefMap[type_idx][name] -= 1;

            return RefMap[type_idx][name] == 0;
        }
    }
}