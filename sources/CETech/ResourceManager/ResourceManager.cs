using System.Collections.Generic;
using System.IO;
using System.Threading;

namespace CETech
{
    public static class ResourceManager
    {
        public delegate byte[] ResourceLoader(Stream input);

        public delegate void ResourceOffline(byte[] data);

        public delegate void ResourceOnline(byte[] data);

        public delegate void ResourceUnloader(byte[] data);

        public static bool _autoReload;

        private static readonly Dictionary<StringId64, ResourceLoader> _loader_map =
            new Dictionary<StringId64, ResourceLoader>();

        private static readonly Dictionary<StringId64, ResourceUnloader> _unloader_map =
            new Dictionary<StringId64, ResourceUnloader>();

        private static readonly Dictionary<StringId64, ResourceOnline> _online_map =
            new Dictionary<StringId64, ResourceOnline>();

        private static readonly Dictionary<StringId64, ResourceOffline> _offline_map =
            new Dictionary<StringId64, ResourceOffline>();

        private static readonly Dictionary<StringId64, int> _types_map = new Dictionary<StringId64, int>();

        private static readonly List<Dictionary<StringId64, byte[]>> _data_map =
            new List<Dictionary<StringId64, byte[]>>();

        private static readonly List<Dictionary<StringId64, int>> _ref_map = new List<Dictionary<StringId64, int>>();

        private static SpinLock _add_lock = new SpinLock();

        public static bool AutoReload
        {
            get { return _autoReload; }
            set { _autoReload = value; }
        }

        public static void RegisterType(StringId64 type, ResourceLoader loader, ResourceUnloader unloader,
            ResourceOnline online, ResourceOffline offline)
        {
            var idx = _data_map.Count;
            _data_map.Add(new Dictionary<StringId64, byte[]>());
            _ref_map.Add(new Dictionary<StringId64, int>());

            _types_map[type] = idx;

            _loader_map[type] = loader;
            _unloader_map[type] = unloader;
            _online_map[type] = online;
            _offline_map[type] = offline;
        }

        public static byte[][] Load(StringId64 type, StringId64[] names)
        {
            var data = new byte[names.Length][];

            for (var i = 0; i < names.Length; i++)
            {
                var input = FileSystem.Open("build", string.Format("{0}{1}", type, names[i]), FileSystem.OpenMode.Read);

                data[i] = _loader_map[type](input);
            }

            return data;
        }

        public static void AddLoaded(byte[][] loaded_data, StringId64 type, StringId64[] names)
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

        public static void LoadNow(StringId64 type, StringId64[] names)
        {
            var loaded_data = Load(type, names);
            AddLoaded(loaded_data, type, names);
        }

        public static bool CanGet(StringId64 type, StringId64[] names)
        {
            var idx = _types_map[type];
            for (var i = 0; i < names.Length; i++)
            {
                if (!_data_map[idx].ContainsKey(names[i]))
                {
                    return false;
                }
                ;
            }

            return true;
        }

        public static void Unload(StringId64 type, StringId64[] names)
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

        private static void incRef(int type_idx, StringId64 name)
        {
            int counter;

            if (!_ref_map[type_idx].TryGetValue(name, out counter))
            {
                counter = 0;
            }

            _ref_map[type_idx][name] = counter + 1;
        }

        private static bool decRef(int type_idx, StringId64 name)
        {
            _ref_map[type_idx][name] -= 1;

            return _ref_map[type_idx][name] == 0;
        }

        public static void Get(StringId64 type, StringId64 name, out byte[] data)
        {
            StringId64[] names = {name};

            if (_autoReload && !CanGet(type, names))
            {
                LoadNow(type, names);
            }

            var idx = _types_map[type];
            data = _data_map[idx][name];
        }
    }
}