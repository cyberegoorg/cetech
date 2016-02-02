using System;
using System.IO;

namespace CELib
{
    public class LogHandler
    {
        private static readonly string LogFormat = "---" + System.Environment.NewLine +
                                                   "level: {0}" + System.Environment.NewLine +
                                                   "where: {1}" + System.Environment.NewLine +
                                                   "time: {2}" + System.Environment.NewLine +
                                                   "worker: {3}" + System.Environment.NewLine +
                                                   "msg: |" + System.Environment.NewLine +
                                                   "  {4}\n" + System.Environment.NewLine;

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

        public static void console_log(Log.Level level, DateTime time, int workerId, string where, string msg, params Object[] args)
        {
            Console.ForegroundColor = LevelToColor(level);
            Console.WriteLine(LogFormat, level, where, time, workerId, String.Format(msg, args));
            Console.ResetColor();
        }


        public class FileLog : LogHandler
        {
            private readonly StreamWriter _write;

            public FileLog(string filename)
            {
                _write = new StreamWriter(filename);
            }

            public void Log(Log.Level level, DateTime time, int workerId, string where, string msg, params Object[] args)
            {
                _write.Write(LogFormat, level, where, time, workerId, String.Format(msg, args));
                _write.Flush();
            }
        }
    }
}