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


        public static void Load(StringId name)
        {
            var pack = ResourceManager.Get<PackageResource.Resource>(PackageResource.Type, name);

            int[] tasks = {0};
            for (var i = 0; i < pack.Type.Length; ++i)
            {
                var task = new PackageTask
                {
                    type = new StringId(pack.Type[i]),
                    names = pack.Names[i],
                    name = name
                };

                tasks[0] = TaskManager.AddBegin("compile_task", package_task, task);
                TaskManager.AddEnd(tasks);
            }
        }

        public static void Unload(StringId name)
        {
            var pack = ResourceManager.Get<PackageResource.Resource>(PackageResource.Type, name);

            for (var i = 0; i < pack.Type.Length; ++i)
            {
                ResourceManager.Unload(new StringId(pack.Type[i]), pack.Names[i]);
            }
        }

        public static bool IsLoaded(StringId name)
        {
            var pack = ResourceManager.Get<PackageResource.Resource>(PackageResource.Type, name);

            for (var i = 0; i < pack.Type.Length; ++i)
            {
                if (!ResourceManager.CanGet(new StringId(pack.Type[i]), pack.Names[i]))
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