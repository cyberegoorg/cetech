using System;

namespace CETech
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
            /// <summary>
            /// Informatin message
            /// </summary>
            Info,

            /// <summary>
            /// Warning message
            /// </summary>
            Warning,
            
            /// <summary>
            /// Error message
            /// </summary>
            Error,


            /// <summary>
            /// Debug message
            /// </summary>
            Debug
        }

        /// <summary>
        /// Log handler
        /// </summary>
        /// <param name="level">Log level</param>
        /// <param name="time">Time</param>
        /// <param name="workerId">Worker id</param>
        /// <param name="where">Where</param>
        /// <param name="msg">Message</param>
        /// <param name="args">Message format arguments</param>
        public delegate void LogHandler(Level level, DateTime time, int workerId, string where, string msg, params object[] args);
        
        /// <summary>
        /// Main log event
        /// </summary>
        public static event LogHandler LogEvent;

        /// <summary>
        /// Log info message
        /// </summary>
        /// <param name="where">Where</param>
        /// <param name="msg">Message</param>
        /// <param name="args">Message format arguments</param>
        public static void Info(string where, string msg, params object[] args)
        {
            OnLog(Level.Info, DateTime.Now, 0, where, msg, args);
        }


        /// <summary>
        /// Warning info message
        /// </summary>
        /// <param name="where">Where</param>
        /// <param name="msg">Message</param>
        /// <param name="args">Message format arguments</param>
        public static void Warning(string where, string msg, params object[] args)
        {
            OnLog(Level.Warning, DateTime.Now, 0, where, msg, args);
        }

        /// <summary>
        /// Error info message
        /// </summary>
        /// <param name="where">Where</param>
        /// <param name="msg">Message</param>
        /// <param name="args">Message format arguments</param>
        public static void Error(string where, string msg, params object[] args)
        {
            OnLog(Level.Error, DateTime.Now, 0, where, msg, args);
        }

        /// <summary>
        /// Debug info message
        /// </summary>
        /// <param name="where">Where</param>
        /// <param name="msg">Message</param>
        /// <param name="args">Message format arguments</param>
        public static void Debug(string where, string msg, params object[] args)
        {
#if DEBUG
            OnLog(Level.Debug, DateTime.Now, 0, where, msg, args);
#endif
        }

        /// <summary>
        /// On log
        /// </summary>
        /// <param name="level">Log level</param>
        /// <param name="time">Time</param>
        /// <param name="workerId">Worker id</param>
        /// <param name="where">Where</param>
        /// <param name="msg">Message</param>
        /// <param name="args">Message format arguments</param>
        private static void OnLog(Level level, DateTime time, int workerId, string @where, string msg, params object[] args)
        {
            var handler = LogEvent;
            if (handler != null) handler(level, time, workerId, @where, msg, args);
        }

    }
}
