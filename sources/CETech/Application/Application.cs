namespace CETech
{
    /// <summary>
    ///     Application
    /// </summary>
    public static partial class Application
    {
        /// <summary>
        ///     Get application main window
        /// </summary>
        public static Window MainWindow { get; set; }

        /// <summary>
        /// Parse command line args
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
        public static bool Init()
        {
            return InitImpl();
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
    }
}