using CETech.Utils;
using MoonSharp.Interpreter;

namespace CETech.Lua.Api
{
    [MoonSharpUserData]
    internal class LogApi
    {
        public static void Info(string where, string msg, params object[] args)
        {
            Log.Info(where, msg, args);
        }

        public static void Warning(string where, string msg, params object[] args)
        {
            Log.Warning(where, msg, args);
        }


        public static void Error(string where, string msg, params object[] args)
        {
            Log.Error(where, msg, args);
        }

        public static void Debug(string where, string msg, params object[] args)
        {
#if DEBUG
            Log.Debug(where, msg, args);
#endif
        }
    }
}