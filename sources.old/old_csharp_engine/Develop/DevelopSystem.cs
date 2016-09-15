using System;

namespace CETech.Develop
{
    /// <summary>
    ///     Develop system.
    /// </summary>
    public partial class DevelopSystem
    {
        /// <summary>
        ///     Init develop system.
        /// </summary>
        public static void Init()
        {
            InitImpl();
        }

        /// <summary>
        ///     Shutdown develop system.
        /// </summary>
        public static void Shutdown()
        {
            ShutdownImpl();
        }

        /// <summary>
        ///     Pushs the record float.
        /// </summary>
        /// <param name="name">Name.</param>
        /// <param name="value">Value.</param>
        public static void PushRecordFloat(string name, float value)
        {
            PushRecordFloatImpl(name, value);
        }

        /// <summary>
        ///     Pushs the record int.
        /// </summary>
        /// <param name="name">Name.</param>
        /// <param name="value">Value.</param>
        public static void PushRecordInt(string name, int value)
        {
            PushRecordIntImpl(name, value);
        }

        /// <summary>
        ///     Enters the scope.
        /// </summary>
        /// <returns>The scope.</returns>
        public static DateTime EnterScope()
        {
            return EnterScopeImpl();
        }

        /// <summary>
        ///     Leaves the scope.
        /// </summary>
        /// <param name="name">Name.</param>
        /// <param name="start_time">Start time.</param>
        public static void LeaveScope(string name, DateTime start_time)
        {
            LeaveScopeImpl(name, start_time);
        }

        /// <summary>
        ///     Frames the begin.
        /// </summary>
        public static void FrameBegin()
        {
            FrameBeginImpl();
        }

        /// <summary>
        ///     Send this instance.
        /// </summary>
        public static void Send()
        {
            FrameEndImpl();
        }
    }
}