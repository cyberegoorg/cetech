using System;
using System.IO;
using MoonSharp.Interpreter;
using NNanomsg;
using NNanomsg.Protocols;

namespace CETech.Utils
{
    /// <summary>
    ///     Log handlers
    /// </summary>
    public class LogHandler
    {
        private static readonly string LogFormat = "---" + Environment.NewLine +
                                                   "level: {0}" + Environment.NewLine +
                                                   "where: {1}" + Environment.NewLine +
                                                   "time: {2}" + Environment.NewLine +
                                                   "worker: {3}" + Environment.NewLine +
                                                   "msg: |" + Environment.NewLine +
                                                   "  {4}" + Environment.NewLine;

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
        /// <param name="level">Level</param>
        /// <param name="time">Time</param>
        /// <param name="workerId">Worker id</param>
        /// <param name="where">Where</param>
        /// <param name="msg">Msg</param>
        public static void ConsoleLog(Log.Level level, DateTime time, int workerId, string where, string msg)
        {
            Console.ForegroundColor = LevelToColor(level);
            Console.WriteLine(LogFormat, level, where, time, workerId, msg);
            Console.ResetColor();
        }


        /// <summary>
        ///     File log handler
        /// </summary>
        public class FileLog 
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
            /// <param name="level">Level</param>
            /// <param name="time">Time</param>
            /// <param name="workerId">Worker id</param>
            /// <param name="where">Where</param>
            /// <param name="msg">Msg</param>
            public void Log(Log.Level level, DateTime time, int workerId, string where, string msg)
            {
                _write.Write(LogFormat, level, where, time, workerId, msg);
                _write.Flush();
            }
        }

        /// <summary>
        ///     Nanomsg log handler
        /// </summary>
        public class NanoLog
        {
            private PublishSocket _socket;

            /// <summary>
            ///     Create file log handler
            /// </summary>
            /// <param name="url">url</param>
            public NanoLog(string url)
            {
                _socket = new PublishSocket();
                _socket.Bind(url);
            }

            /// <summary>
            ///     Log handler
            /// </summary>
            /// <param name="level">Level</param>
            /// <param name="time">Time</param>
            /// <param name="workerId">Worker id</param>
            /// <param name="where">Where</param>
            /// <param name="msg">Msg</param>
            public void Log(Log.Level level, DateTime time, int workerId, string where, string msg)
            {
                _socket.Send(System.Text.Encoding.UTF8.GetBytes(string.Format(LogFormat, level, where, time, workerId, msg).ToCharArray()));
            }
        }
    }
}