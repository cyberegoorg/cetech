using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CELib
{
    /// <summary>
    /// Log system.
    /// </summary>
    public static class Log
    {
        /// <summary>
        /// Log level enum.
        /// </summary>
        public enum Level
        {
            Info,
            Warning,
            Error,
            Debug
        }

        public delegate void LogHandler(Level level, DateTime time, int workerId, string where, string msg, params object[] args);
        public static event LogHandler LogEvent;

        public static void Info(string where, string msg, params object[] args)
        {
            OnLog(Level.Info, DateTime.Now, 0, where, msg, args);
        }

        public static void Warning(string where, string msg, params object[] args)
        {
            OnLog(Level.Warning, DateTime.Now, 0, where, msg, args);
        }


        public static void Error(string where, string msg, params object[] args)
        {
            OnLog(Level.Error, DateTime.Now, 0, where, msg, args);
        }

        public static void Debug(string where, string msg, params object[] args)
        {
#if DEBUG
            OnLog(Level.Debug, DateTime.Now, 0, where, msg, args);
#endif
        }


        private static void OnLog(Level level, DateTime time, int workerid, string @where, string msg, params object[] args)
        {
            var handler = LogEvent;
            if (handler != null) handler(level, time, workerid, @where, msg, args);
        }

    }
}
