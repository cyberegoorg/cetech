using System.IO;
using MsgPack;

namespace CETech.Develop
{
    public static partial class ConsoleServer
    {
        public delegate void CommandHandler(MessagePackObjectDictionary args, ResponsePacker response);

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

        public partial class ResponsePacker
        {
            public ResponsePacker()
            {
                CtorImpl();
            }

            public void Reset()
            {
                ResetImpl();
            }

            public MemoryStream GetMemoryStream()
            {
                return GetMemoryStreamImpl();
            }

            public void PackNull()
            {
                PackNullImpl();
            }

            public void Pack(bool boolean)
            {
                PackImpl(boolean);
            }

            public void Pack(int value)
            {
                PackImpl(value);
            }

            public void Pack(double number)
            {
                PackImpl(number);
            }

            public void Pack(string @string)
            {
                PackImpl(@string);
            }

            public void PackMapHeader(int count)
            {
                PackMapHeaderImpl(count);
            }

            public void PackArrayHeader(int length)
            {
                PackArrayHeaderImpl(length);
            }
        }
    }
}