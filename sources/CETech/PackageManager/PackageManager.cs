using System.Collections.Generic;
using CETech.Utils;

namespace CETech
{
    public class PackageManager
    {
        private static void package_task(object data)
        {
            var task = (PackageTask) data;
            Log.Debug("package_task", "Load package {0}{1}", task.type, task.name);

            ResourceManager.LoadNow(task.type, task.names);
        }


        public static unsafe void Load(StringId64 name)
        {
            byte[] data;
            ResourceManager.Get(PackageResource.Type, name, out data);

            var header = ByteUtils.FromBytes<PackageResource.Header>(data);

            var headerSizeof = sizeof (PackageResource.Header);
            var typeHeaderSizeof = sizeof (PackageResource.TypeHeader);
            var stringIdSizeof = sizeof (StringId64);

            var typeHeaderOffset = headerSizeof;

            int[] tasks = {0};
            var name_list = new List<StringId64>();
            for (ulong i = 0; i < header.count; i++, typeHeaderOffset += typeHeaderSizeof)
            {
                name_list.Clear();

                var type_header = ByteUtils.FromBytes<PackageResource.TypeHeader>(data, typeHeaderOffset);
                var namesOffset = type_header.offset;
                for (ulong j = 0; j < type_header.count; ++j, namesOffset += (ulong) stringIdSizeof)
                {
                    var res_name = ByteUtils.FromBytes<StringId64>(data, (int) namesOffset);
                    name_list.Add(res_name);
                }

                var task = new PackageTask
                {
                    type = new StringId64(type_header.type),
                    names = name_list.ToArray(),
                    name = name
                };

                tasks[0] = TaskManager.AddBegin("compile_task", package_task, task);
                TaskManager.AddEnd(tasks);
            }
        }

        public static unsafe void Unload(StringId64 name)
        {
            byte[] data;
            ResourceManager.Get(PackageResource.Type, name, out data);

            var header = ByteUtils.FromBytes<PackageResource.Header>(data);

            var headerSizeof = sizeof (PackageResource.Header);
            var typeHeaderSizeof = sizeof (PackageResource.TypeHeader);
            var stringIdSizeof = sizeof (StringId64);

            var typeHeaderOffset = headerSizeof;

            int[] tasks = {0};
            var name_list = new List<StringId64>();
            for (ulong i = 0; i < header.count; i++, typeHeaderOffset += typeHeaderSizeof)
            {
                name_list.Clear();

                var type_header = ByteUtils.FromBytes<PackageResource.TypeHeader>(data, typeHeaderOffset);
                var namesOffset = type_header.offset;
                for (ulong j = 0; j < type_header.count; ++j, namesOffset += (ulong) stringIdSizeof)
                {
                    var res_name = ByteUtils.FromBytes<StringId64>(data, (int) namesOffset);
                    name_list.Add(res_name);
                }

                ResourceManager.Unload(new StringId64(type_header.type), name_list.ToArray());
            }
        }

        public static unsafe bool IsLoaded(StringId64 name)
        {
            byte[] data;
            ResourceManager.Get(PackageResource.Type, name, out data);

            var header = ByteUtils.FromBytes<PackageResource.Header>(data);

            var headerSizeof = sizeof (PackageResource.Header);
            var typeHeaderSizeof = sizeof (PackageResource.TypeHeader);
            var stringIdSizeof = sizeof (StringId64);

            var typeHeaderOffset = headerSizeof;

            int[] tasks = {0};
            var name_list = new List<StringId64>();
            for (ulong i = 0; i < header.count; i++, typeHeaderOffset += typeHeaderSizeof)
            {
                name_list.Clear();

                var type_header = ByteUtils.FromBytes<PackageResource.TypeHeader>(data, typeHeaderOffset);
                var namesOffset = type_header.offset;
                for (ulong j = 0; j < type_header.count; ++j, namesOffset += (ulong) stringIdSizeof)
                {
                    var res_name = ByteUtils.FromBytes<StringId64>(data, (int) namesOffset);
                    name_list.Add(res_name);
                }

                if (!ResourceManager.CanGet(new StringId64(type_header.type), name_list.ToArray()))
                {
                    return false;
                }
            }

            return true;
        }

        public static void Flush(StringId64 name)
        {
            while (!IsLoaded(name))
            {
                TaskManager.DoWork();
            }
        }

        internal struct PackageTask
        {
            public StringId64 name;
            public StringId64[] names;
            public StringId64 type;
        }
    }
}