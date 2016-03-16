using System;

namespace CETech.Develop
{
    public partial class DevelopSystem
    {
        public static void Init()
        {
            InitImpl();
        }

        public static void Shutdown()
        {
            ShutdownImpl();
        }

        public static void PushRecordFloat(string name, float value)
        {
            PushRecordFloatImpl(name, value);
        }

        public static void PushRecordInt(string name, int value)
        {
            PushRecordIntImpl(name, value);
        }

        public static DateTime EnterScope()
        {
            return EnterScopeImpl();
        }

        public static void LeaveScope(string name, DateTime start_time)
        {
            LeaveScopeImpl(name, start_time);
        }

        public static void FrameBegin()
        {
            FrameBeginImpl();
        }

        public static void Send()
        {
            FrameEndImpl();
        }
    }
}