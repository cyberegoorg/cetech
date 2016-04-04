namespace CETech
{
    /// <summary>
    ///     Package manager
    /// </summary>
    public partial class PackageManager
    {
        /// <summary>
        ///     Load package
        /// </summary>
        /// <param name="name">Package name</param>
        public static void Load(long name)
        {
            LoadImpl(name);
        }

        /// <summary>
        ///     Unload package
        /// </summary>
        /// <param name="name">Package name</param>
        public static void Unload(long name)
        {
            UnloadImpl(name);
        }

        /// <summary>
        ///     Is package is loaded?
        /// </summary>
        /// <param name="name">Package name</param>
        /// <returns></returns>
        public static bool IsLoaded(long name)
        {
            return IsLoadedImpl(name);
        }

        /// <summary>
        ///     Wait for package load done.
        /// </summary>
        /// <param name="name">Package name</param>
        public static void Flush(long name)
        {
            FlushImpl(name);
        }
    }
}