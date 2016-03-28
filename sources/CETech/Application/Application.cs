namespace CETech
{
    /// <summary>
    ///     Application
    /// </summary>
    public static partial class Application
    {
        public delegate void SystemInitConfig();

        public delegate void SystemInitDelegate();

        public delegate void SystemShutdownDelegate();

        public static void RegisterSystems(SystemInitConfig[] initConfig, SystemInitDelegate[] init,
            SystemShutdownDelegate[] shutdown)
        {
            RegisterSystemsImpl(initConfig, init, shutdown);
        }

        /// <summary>
        ///     Parse command line args
        /// </summary>
        /// <param name="args">args</param>
        /// <returns></returns>
        public static bool ParseCmdLine(string[] args)
        {
            return ParseCmdLineImpl(args);
        }

        /// <summary>
        ///     Init application
        /// </summary>
        public static bool Init(string[] args)
        {
            return InitImpl(args);
        }

        /// <summary>
        ///     Shutdown application
        /// </summary>
        public static void Shutdown()
        {
            ShutdownImpl();
        }

        /// <summary>
        ///     Run application
        /// </summary>
        public static void Run()
        {
            RunImpl();
        }

        /// <summary>
        ///     Quit application
        /// </summary>
        public static void Quit()
        {
            QuitImpl();
        }

        /// <summary>
        ///     Get native platform name
        /// </summary>
        /// <returns>Platform name</returns>
        public static string GetPlatform()
        {
            return GetPlatformImpl();
        }
    }
}