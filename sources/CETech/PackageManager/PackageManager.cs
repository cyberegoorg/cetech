using CETech.Utils;

namespace CETech
{
    /// <summary>
    ///     Package manager
    /// </summary>
    public class PackageManager
    {
        private static void package_task(object data)
        {
            var task = (PackageTask) data;
            Log.Debug("package_task", "Load package {0:x}{1:x}", task.type, task.name);

            ResourceManager.LoadNow(task.type, task.names);
        }


        /// <summary>
        ///     Load package
        /// </summary>
        /// <param name="name">Package name</param>
        public static void Load(long name)
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

        /// <summary>
        ///     Unload package
        /// </summary>
        /// <param name="name">Package name</param>
        public static void Unload(long name)
        {
            var pack = ResourceManager.Get<PackageResource.Resource>(PackageResource.Type, name);

            for (var i = 0; i < pack.Type.Length; ++i)
            {
                ResourceManager.Unload(pack.Type[i], pack.Names[i]);
            }
        }

        /// <summary>
        ///     Is package is loaded?
        /// </summary>
        /// <param name="name">Package name</param>
        /// <returns></returns>
        public static bool IsLoaded(long name)
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

        /// <summary>
        ///     Wait for package load done.
        /// </summary>
        /// <param name="name">Package name</param>
        public static void Flush(long name)
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