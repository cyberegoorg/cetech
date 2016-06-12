using System.IO;
using MsgPack;

namespace CETech.Develop
{
    /// <summary>
    ///     Console server.
    /// </summary>
    public static partial class ConsoleServer
    {
        /// <summary>
        ///     Command handler.
        /// </summary>
        public delegate void CommandHandler(MessagePackObjectDictionary args, ResponsePacker response);


        /// <summary>
        ///     Registers the command.
        /// </summary>
        /// <param name="name">Name.</param>
        /// <param name="handler">Handler.</param>
        public static void RegisterCommand(string name, CommandHandler handler)
        {
            RegisterCommandImpl(name, handler);
        }

        public static void InitConfig()
        {
            InitConfigImpl();
        }

        /// <summary>
        ///     Init console server.
        /// </summary>
        public static void Init()
        {
            InitImpl();
        }

        /// <summary>
        ///     Shutdown console server.
        /// </summary>
        public static void Shutdown()
        {
            ShutdownImpl();
        }

        /// <summary>
        ///     Console server tick.
        /// </summary>
        public static void Tick()
        {
            TickImpl();
        }

        /// <summary>
        ///     Response packer.
        /// </summary>
        public partial class ResponsePacker
        {
            /// <summary>
            ///     Initializes a new instance.
            /// </summary>
            public ResponsePacker()
            {
                CtorImpl();
            }

            /// <summary>
            ///     Reset packer for new usage.
            /// </summary>
            public void Reset()
            {
                ResetImpl();
            }

            /// <summary>
            ///     Gets the memory stream.
            /// </summary>
            /// <returns>The memory stream.</returns>
            public MemoryStream GetMemoryStream()
            {
                return GetMemoryStreamImpl();
            }

            /// <summary>
            ///     Packs the null.
            /// </summary>
            public void PackNull()
            {
                PackNullImpl();
            }

            /// <summary>
            ///     Pack the specified boolean.
            /// </summary>
            /// <param name="boolean">boolean value.</param>
            public void Pack(bool boolean)
            {
                PackImpl(boolean);
            }

            /// <summary>
            ///     Pack the specified value.
            /// </summary>
            /// <param name="value">Value.</param>
            public void Pack(int value)
            {
                PackImpl(value);
            }

            /// <summary>
            ///     Pack the specified value.
            /// </summary>
            /// <param name="value">Value.</param>
            public void Pack(long value)
            {
                PackImpl(value);
            }

            /// <summary>
            ///     Pack the specified number.
            /// </summary>
            /// <param name="number">Number.</param>
            public void Pack(double number)
            {
                PackImpl(number);
            }

            /// <summary>
            ///     Pack the specified string.
            /// </summary>
            /// <param name="string">String.</param>
            public void Pack(string @string)
            {
                PackImpl(@string);
            }

            /// <summary>
            ///     Packs the map header.
            ///     Must folow 2*count items.
            /// </summary>
            /// <param name="count">Item count.</param>
            public void PackMapHeader(int count)
            {
                PackMapHeaderImpl(count);
            }

            /// <summary>
            ///     Packs the array header.
            /// </summary>
            /// <param name="length">Length.</param>
            public void PackArrayHeader(int length)
            {
                PackArrayHeaderImpl(length);
            }
        }
    }
}