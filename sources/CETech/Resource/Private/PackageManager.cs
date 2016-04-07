using CETech.Utils;

namespace CETech.Resource
{
    public partial class PackageManager
    {
        private static void package_task(object data)
        {
            var task = (PackageTask) data;
            Log.Debug("package_task", "Load package {0:X}{1:X}", task.type, task.name);

            ResourceManager.LoadNow(task.type, task.names);
        }

        private static void LoadImpl(long name)
        {
            var pack = ResourceManager.Get<PackageResource.Resource>(PackageResource.Type, name);

            int[] tasks = {0};
            for (var i = 0; i < pack.Type.Length; ++i)
            {
                var task = new PackageTask
                {
                    type = pack.Type[i],
                    names = pack.Names[i],
                    name = name
                };

                tasks[0] = TaskManager.AddBegin("compile_task", package_task, task);
                TaskManager.AddEnd(tasks);
            }
        }

        private static void UnloadImpl(long name)
        {
            var pack = ResourceManager.Get<PackageResource.Resource>(PackageResource.Type, name);

            for (var i = 0; i < pack.Type.Length; ++i)
            {
                ResourceManager.Unload(pack.Type[i], pack.Names[i]);
            }
        }

        private static bool IsLoadedImpl(long name)
        {
            var pack = ResourceManager.Get<PackageResource.Resource>(PackageResource.Type, name);

            for (var i = 0; i < pack.Type.Length; ++i)
            {
                if (!ResourceManager.CanGet(pack.Type[i], pack.Names[i]))
                {
                    return false;
                }
            }

            return true;
        }

        private static void FlushImpl(long name)
        {
            while (!IsLoaded(name))
            {
                TaskManager.DoWork();
            }
        }

        internal struct PackageTask
        {
            public long name;
            public long[] names;
            public long type;
        }
    }
}