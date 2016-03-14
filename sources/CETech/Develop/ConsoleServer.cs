using MsgPack;

namespace CETech.Develop
{
    public static partial class ConsoleServer
    {
        public delegate void CommandHandler(MessagePackObjectDictionary args, Packer response);

        public static void RegisterCommand(string name, CommandHandler handler)
        {
            RegisterCommandImpl(name, handler);
        }


        public static void Init()
        {
            InitImpl();
        }

        public static void Shutdown()
        {
            ShutdownImpl();
        }

        public static void Tick()
        {
            TickImpl();
        }
    }
}