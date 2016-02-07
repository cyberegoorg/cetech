using System;
using System.IO;

namespace CELib
{
    /// <summary>
    ///     Log handlers
    /// </summary>
    public class LogHandler
    {
        public static readonly string LogFormat = "---" + Environment.NewLine +
                                                  "level: {0}" + Environment.NewLine +
                                                  "where: {1}" + Environment.NewLine +
                                                  "time: {2}" + Environment.NewLine +
                                                  "worker: {3}" + Environment.NewLine +
                                                  "msg: |" + Environment.NewLine +
                                                  "  {4}\n" + Environment.NewLine;

        internal static ConsoleColor LevelToColor(Log.Level level)
        {
            switch (level)
            {
                case Log.Level.Info:
                    return ConsoleColor.Blue;
                case Log.Level.Warning:
                    return ConsoleColor.Yellow;
                case Log.Level.Error:
                    return ConsoleColor.Red;
                case Log.Level.Debug:
                    return ConsoleColor.Green;
                default:
                    throw new ArgumentOutOfRangeException("level", level, null);
            }
        }

        /// <summary>
        ///     Console log handler
        /// </summary>
        /// <param name="level"></param>
        /// <param name="time"></param>
        /// <param name="workerId"></param>
        /// <param name="where"></param>
        /// <param name="msg"></param>
        /// <param name="args"></param>
        public static void console_log(Log.Level level, DateTime time, int workerId, string where, string msg,
            params object[] args)
        {
            Console.ForegroundColor = LevelToColor(level);
            Console.WriteLine(LogFormat, level, where, time, workerId, string.Format(msg, args));
            Console.ResetColor();
        }


        /// <summary>
        ///     File log handler
        /// </summary>
        public class FileLog : LogHandler
        {
            private readonly StreamWriter _write;

            /// <summary>
            ///     Create file log handler
            /// </summary>
            /// <param name="filename">Filename</param>
            public FileLog(string filename)
            {
                _write = new StreamWriter(filename);
            }

            /// <summary>
            ///     Log handler
            /// </summary>
            /// <param name="level"></param>
            /// <param name="time"></param>
            /// <param name="workerId"></param>
            /// <param name="where"></param>
            /// <param name="msg"></param>
            /// <param name="args"></param>
            public void Log(Log.Level level, DateTime time, int workerId, string where, string msg, params object[] args)
            {
                _write.Write(LogFormat, level, where, time, workerId, string.Format(msg, args));
                _write.Flush();
            }
        }
    }
}