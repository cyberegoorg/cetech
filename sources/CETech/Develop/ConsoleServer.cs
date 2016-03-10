namespace CETech.Develop
{
    public static partial class ConsoleServer
    {

        public static void Init()
        {
            InitImpl();
        }


        public static void Shutdown()
        {
            ShutdownImpl();
        }
    }
}