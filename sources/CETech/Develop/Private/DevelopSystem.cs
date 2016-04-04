using System;
using System.Collections.Generic;
using System.Threading;
using NNanomsg.Protocols;

// ReSharper disable once CheckNamespace

namespace CETech.Develop
{
    public partial class DevelopSystem
    {
        private static SpinLock _addRecordFloatLock = new SpinLock();
        private static SpinLock _addRecordIntLock = new SpinLock();
        private static SpinLock _addScopesLock = new SpinLock();

        private static readonly List<RecordFloat> RecordFloats = new List<RecordFloat>();
        private static readonly List<RecordInt> RecordInts = new List<RecordInt>();
        private static readonly List<Scope> RecordScopes = new List<Scope>();

        [ThreadStatic] private static int _scopeDepth;

        private static PublishSocket _socket;

        private static void InitImpl()
        {
            _socket = new PublishSocket();
            _socket.Bind($"ws://*:{ConfigSystem.GetValueInt("console_server.base_port") + 2}");
        }

        private static void ShutdownImpl()
        {
        }

        private static void PushRecordFloatImpl(string name, float value)
        {
            var gotLock = false;
            try
            {
                _addRecordFloatLock.Enter(ref gotLock);

                RecordFloats.Add(new RecordFloat {name = name, value = value});
            }
            finally
            {
                if (gotLock) _addRecordFloatLock.Exit();
            }
        }

        private static void PushRecordIntImpl(string name, int value)
        {
            var gotLock = false;
            try
            {
                _addRecordIntLock.Enter(ref gotLock);

                RecordInts.Add(new RecordInt {name = name, value = value});
            }
            finally
            {
                if (gotLock) _addRecordIntLock.Exit();
            }
        }

        private static DateTime EnterScopeImpl()
        {
            ++_scopeDepth;
            return DateTime.Now;
        }

        private static void LeaveScopeImpl(string name, DateTime startTime)
        {
            var gotLock = false;
            try
            {
                _addScopesLock.Enter(ref gotLock);

                var start_time_ms = (startTime - new DateTime(1970, 1, 1)).TotalMilliseconds;
                var end_time_ms = (DateTime.Now - new DateTime(1970, 1, 1)).TotalMilliseconds;

                RecordScopes.Add(new Scope
                {
                    name = name,
                    start = start_time_ms,
                    end = end_time_ms,
                    depth = _scopeDepth,
                    workerid = TaskManager.WorkerId
                });
                --_scopeDepth;
            }
            finally
            {
                if (gotLock) _addScopesLock.Exit();
            }
        }

        private static void FrameBeginImpl()
        {
            RecordFloats.Clear();
            RecordInts.Clear();
            RecordScopes.Clear();
        }

        private static void FrameEndImpl()
        {
            var count = RecordFloats.Count + RecordInts.Count + RecordScopes.Count;

            if (count == 0)
            {
                return;
            }

            var packer = new ConsoleServer.ResponsePacker();

            packer.PackMapHeader(3);

            packRecordFloats(packer);
            packRecordInts(packer);
            packScopes(packer);

            var stream = packer.GetMemoryStream();
            if (stream.Length == 0)
            {
                return;
            }

            _socket.Send(stream.ToArray());
        }

        private static void packRecordFloats(ConsoleServer.ResponsePacker packer)
        {
            //
            packer.Pack("EVENT_RECORD_FLOAT");

            packer.PackMapHeader(RecordFloats.Count);
            for (var i = 0; i < RecordFloats.Count; ++i)
            {
                packer.Pack(RecordFloats[i].name);
                packer.Pack(RecordFloats[i].value);
            }
        }

        private static void packRecordInts(ConsoleServer.ResponsePacker packer)
        {
            packer.Pack("EVENT_RECORD_INT");

            packer.PackMapHeader(RecordInts.Count);
            for (var i = 0; i < RecordInts.Count; ++i)
            {
                packer.Pack(RecordInts[i].name);
                packer.Pack(RecordInts[i].value);
            }
        }

        private static void packScopes(ConsoleServer.ResponsePacker packer)
        {
            packer.Pack("EVENT_SCOPE");

            packer.PackArrayHeader(RecordScopes.Count);
            for (var i = 0; i < RecordScopes.Count; ++i)
            {
                packer.PackMapHeader(5);

                //
                packer.Pack("name");
                packer.Pack(RecordScopes[i].name);

                //
                packer.Pack("depth");
                packer.Pack(RecordScopes[i].depth);

                //
                packer.Pack("start");
                packer.Pack(RecordScopes[i].start);

                //
                packer.Pack("end");
                packer.Pack(RecordScopes[i].end);

                //
                packer.Pack("workerid");
                packer.Pack(RecordScopes[i].workerid);
            }
        }

        private struct RecordFloat
        {
            public string name;
            public float value;
        }

        private struct RecordInt
        {
            public string name;
            public int value;
        }

        private struct Scope
        {
            public string name;
            public double start;
            public double end;
            public int depth;
            public int workerid;
        }
    }
}