using CETech.Utils;

namespace CETech.Develop
{
    public static partial class ConsoleServer
    {
        private static LogHandler.NanoLog _nanoLog;

        private static void InitImpl()
        {
            _nanoLog = new LogHandler.NanoLog("ws://*:5556");
            Log.LogEvent += _nanoLog.Log;

        }

        private static void ShutdownImpl()
        {
            Log.LogEvent -= _nanoLog.Log;
            _nanoLog = null;
        }
    }
}