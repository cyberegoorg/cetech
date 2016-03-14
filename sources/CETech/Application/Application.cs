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
        ///     Init application
        /// </summary>
        public static void Init()
        {
            InitImpl();
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