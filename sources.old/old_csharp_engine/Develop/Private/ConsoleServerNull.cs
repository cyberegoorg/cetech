using System.IO;

namespace CETech.Develop
{
    public static partial class ConsoleServer
    {

        private static void InitConfigImpl()
        {
            
        }

        private static void InitImpl()
        {
        }


        private static void ShutdownImpl()
        {
        }


        private static void TickImpl()
        {
        }

        private static void RegisterCommandImpl(string name, CommandHandler handler)
        {
        }

        public partial class ResponsePacker
        {
            private void CtorImpl()
            {

            }

            private void PackNullImpl()
            {
            }

            private void PackImpl(bool boolean)
            {
            }

            private void PackImpl(double number)
            {
            }

            private void PackImpl(int value)
            {
            }

            private void PackImpl(string @string)
            {
            }

            private void PackMapHeaderImpl(int count)
            {
            }

            private void PackArrayHeaderImpl(int length)
            {
            }

            private MemoryStream GetMemoryStreamImpl()
            {
                return new MemoryStream();
            }

            private void ResetImpl()
            {
            }
        }
    }
}