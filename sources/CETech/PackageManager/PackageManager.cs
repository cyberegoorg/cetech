using System;
using System.Collections.Generic;
using CETech.Utils;
using MsgPack.Serialization;

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


        public static unsafe void Load(StringId name)
        {
            object data;
            ResourceManager.Get(PackageResource.Type, name, out data);
            PackageResource.PackagePack pack = (PackageResource.PackagePack) (data);
            
            int[] tasks = {0};
            for (int i = 0; i < pack.type.Length; ++i)
            {
                var task = new PackageTask
                {
                    type = new StringId(pack.type[i]),
                    names = pack.names[i],
                    name = name
                };

                tasks[0] = TaskManager.AddBegin("compile_task", package_task, task);
                TaskManager.AddEnd(tasks);
            }
        }

        public static unsafe void Unload(StringId name)
        {
            object data;
            ResourceManager.Get(PackageResource.Type, name, out data);
            PackageResource.PackagePack pack = (PackageResource.PackagePack)(data);

            for (int i = 0; i < pack.type.Length; ++i)
            {
                ResourceManager.Unload(new StringId(pack.type[i]), pack.names[i]);
            }
        }

        public static unsafe bool IsLoaded(StringId name)
        {
            object data;
            ResourceManager.Get(PackageResource.Type, name, out data);
            PackageResource.PackagePack pack = (PackageResource.PackagePack)(data);

            for (int i = 0; i < pack.type.Length; ++i)
            {
                if (!ResourceManager.CanGet(new StringId(pack.type[i]), pack.names[i]))
                {
                    return false;
                }

            }

            return true;
        }

        public static void Flush(StringId name)
        {
            while (!IsLoaded(name))
            {
                TaskManager.DoWork();
            }
        }

        internal struct PackageTask
        {
            public StringId name;
            public StringId[] names;
            public StringId type;
        }
    }
}