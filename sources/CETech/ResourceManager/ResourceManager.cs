using System.Collections.Generic;
using System.IO;
using System.Threading;
using CETech.Utils;

namespace CETech
{
    public static class ResourceManager
    {
        public delegate object ResourceLoader(Stream input);

        public delegate void ResourceOffline(object data);

        public delegate void ResourceOnline(object data);

        public delegate void ResourceUnloader(object data);

        public static bool _autoReload;

        private static readonly Dictionary<StringId, ResourceLoader> _loader_map =
            new Dictionary<StringId, ResourceLoader>();

        private static readonly Dictionary<StringId, ResourceUnloader> _unloader_map =
            new Dictionary<StringId, ResourceUnloader>();

        private static readonly Dictionary<StringId, ResourceOnline> _online_map =
            new Dictionary<StringId, ResourceOnline>();

        private static readonly Dictionary<StringId, ResourceOffline> _offline_map =
            new Dictionary<StringId, ResourceOffline>();

        private static readonly Dictionary<StringId, int> _types_map = new Dictionary<StringId, int>();

        private static readonly List<Dictionary<StringId, object>> _data_map =
            new List<Dictionary<StringId, object>>();

        private static readonly List<Dictionary<StringId, int>> _ref_map = new List<Dictionary<StringId, int>>();

        private static SpinLock _add_lock = new SpinLock();

        public static bool AutoReload
        {
            get { return _autoReload; }
            set { _autoReload = value; }
        }

        public static void RegisterType(StringId type, ResourceLoader loader, ResourceUnloader unloader,
            ResourceOnline online, ResourceOffline offline)
        {
            var idx = _data_map.Count;
            _data_map.Add(new Dictionary<StringId, object>());
            _ref_map.Add(new Dictionary<StringId, int>());

            _types_map[type] = idx;

            _loader_map[type] = loader;
            _unloader_map[type] = unloader;
            _online_map[type] = online;
            _offline_map[type] = offline;
        }

        public static object[] Load(StringId type, StringId[] names)
        {
            var data = new object[names.Length];

            for (var i = 0; i < names.Length; i++)
            {
                Log.Debug("resource_manager", "Loading resource {0}{1}", type, names[i]);

                var input = FileSystem.Open("build", string.Format("{0}{1}", type, names[i]), FileSystem.OpenMode.Read);
                data[i] = _loader_map[type](input);
            }

            return data;
        }

        public static void AddLoaded(object[] loaded_data, StringId type, StringId[] names)
        {
            var online = _online_map[type];
            var idx = _types_map[type];

            var gotLock = false;
            try
            {
                _add_lock.Enter(ref gotLock);

                for (var i = 0; i < names.Length; i++)
                {
                    incRef(idx, names[i]);

                    _data_map[idx][names[i]] = loaded_data[i];

                    online(_data_map[idx][names[i]]);
                }
            }
            finally
            {
                if (gotLock) _add_lock.Exit();
            }
        }

        public static void LoadNow(StringId type, StringId[] names)
        {
            var loaded_data = Load(type, names);
            AddLoaded(loaded_data, type, names);
        }

        public static bool CanGet(StringId type, StringId[] names)
        {
            var idx = _types_map[type];
            for (var i = 0; i < names.Length; i++)
            {
                if (!_data_map[idx].ContainsKey(names[i]))
                {
                    return false;
                }
            }

            return true;
        }

        public static void Unload(StringId type, StringId[] names)
        {
            var offline = _offline_map[type];
            var unloader = _unloader_map[type];

            var idx = _types_map[type];

            for (var i = 0; i < names.Length; i++)
            {
                offline(_data_map[idx][names[i]]);
            }

            for (var i = 0; i < names.Length; i++)
            {
                if (decRef(idx, names[i]))
                {
                    unloader(_data_map[idx][names[i]]);

                    _data_map[idx].Remove(names[i]);
                    _ref_map[idx].Remove(names[i]);
                }
            }
        }

        private static void incRef(int type_idx, StringId name)
        {
            int counter;

            if (!_ref_map[type_idx].TryGetValue(name, out counter))
            {
                counter = 0;
            }

            _ref_map[type_idx][name] = counter + 1;
        }

        private static bool decRef(int type_idx, StringId name)
        {
            _ref_map[type_idx][name] -= 1;

            return _ref_map[type_idx][name] == 0;
        }

        public static T Get<T>(StringId type, StringId name)
        {
            StringId[] names = {name};

            if (_autoReload && !CanGet(type, names))
            {
                LoadNow(type, names);
            }

            var idx = _types_map[type];
            return (T) _data_map[idx][name];
        }
    }
}