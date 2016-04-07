using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using CETech.Develop;
using CETech.Utils;

namespace CETech.Resource
{
    /// <summary>
    ///     Resource manager
    /// </summary>
    public static partial class Resource { 

        private static readonly Dictionary<long, ResourceLoader> LoaderMap =
            new Dictionary<long, ResourceLoader>();

        private static readonly Dictionary<long, ResourceUnloader> UnloaderMap =
            new Dictionary<long, ResourceUnloader>();

        private static readonly Dictionary<long, ResourceOnline> OnlineMap =
            new Dictionary<long, ResourceOnline>();

        private static readonly Dictionary<long, ResourceOffline> OfflineMap =
            new Dictionary<long, ResourceOffline>();

        private static readonly Dictionary<long, ResourceReloader> ReloaderMap =
            new Dictionary<long, ResourceReloader>();

        private static readonly Dictionary<long, int> TypesMap = new Dictionary<long, int>();

        private static readonly List<Dictionary<long, object>> DataMap =
            new List<Dictionary<long, object>>();

        private static readonly List<Dictionary<long, int>> RefMap = new List<Dictionary<long, int>>();

        private static SpinLock _addLock = new SpinLock();

        private static void RegisterTypeImpl(long type, ResourceLoader loader, ResourceUnloader unloader,
            ResourceOnline online, ResourceOffline offline, ResourceReloader reloader)
        {
            var idx = DataMap.Count;
            DataMap.Add(new Dictionary<long, object>());
            RefMap.Add(new Dictionary<long, int>());

            TypesMap[type] = idx;

            LoaderMap[type] = loader;
            UnloaderMap[type] = unloader;
            OnlineMap[type] = online;
            OfflineMap[type] = offline;
            ReloaderMap[type] = reloader;
        }

        private static object[] LoadImpl(long type, long[] names)
        {
            var data = new object[names.Length];

            for (var i = 0; i < names.Length; i++)
            {
                Log.Debug("resource_manager", "Loading resource {0:X}{1:X}", type, names[i]);

                using (var input = FileSystem.Open("build", string.Format("{0:X}{1:X}", type, names[i]),
                    FileSystem.OpenMode.Read))
                {
                    data[i] = LoaderMap[type](input);
                }
            }

            return data;
        }

        private static void AddLoadedImpl(object[] loaded_data, long type, long[] names)
        {
            var online = OnlineMap[type];
            var idx = TypesMap[type];

/*            var gotLock = false;
            try
            {
                _addLock.Enter(ref gotLock);*/

            for (var i = 0; i < names.Length; i++)
            {
                IncRef(idx, names[i]);

                DataMap[idx][names[i]] = loaded_data[i];

                online(DataMap[idx][names[i]]);
            }
/*            }
            finally
            {
                if (gotLock) _addLock.Exit();
            }*/
        }

        private static void LoadNowImpl(long type, long[] names)
        {
            var loaded_data = Load(type, names);

            var taskid = TaskManager.AddBegin("addloaded", data => AddLoadedImpl(loaded_data, type, names), null,
                TaskManager.TaskPriority.High, affinity: TaskManager.TaskAffinity.MainThead);
            TaskManager.AddEnd(new[] {taskid});
            TaskManager.Wait(taskid);
        }

        private static void UnloadImpl(long type, long[] names)
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

        private static bool CanGetImpl(long type, long[] names)
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

        private static T GetImpl<T>(long type, long name)
        {
            long[] names = {name};

            if (AutoLoad && !CanGet(type, names))
            {
                LoadNow(type, names);
                Log.Warning("resource_manager", "Autoloading {0:X}", name);
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

        private static void InitConfigImpl()
        {
            ConfigSystem.CreateValue("resource_manager.build", "Path to build dir", Path.Combine("data", "build"));
            AutoLoad = true;
        }

        private static void ReloadImpl(long type, long[] names)
        {
            var data = new object[names.Length];

            var new_data = Load(type, names);

            for (var i = 0; i < names.Length; i++)
            {
                Log.Debug("resource_manager", "Reloading resource {0:X}{1:X}", type, names[i]);
                data[i] = ReloaderMap[type](names[i], new_data[i]);
            }
        }

        private static void ReloadAllImpl(long type)
        {
            var idx = TypesMap[type];

            Reload(type, DataMap[idx].Keys.ToArray());
        }

        private static void InitImpl()
        {
            ConsoleServer.RegisterCommand("resource_manager.reload_all", (args, response) =>
            {
                var types = args["types"].AsList();

                for (var i = 0; i < types.Count; i++)
                {
                    ReloadAll(StringId.FromString(types[i].AsString()));
                }
            });
        }

        private static void ShutdownImpl()
        {
        }
    }
}