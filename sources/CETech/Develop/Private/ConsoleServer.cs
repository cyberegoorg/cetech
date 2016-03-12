using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using CETech.Utils;
using MoonSharp.Interpreter.CoreLib;
using MsgPack;
using MsgPack.Serialization;
using NNanomsg;
using NNanomsg.Protocols;

namespace CETech.Develop
{
    public static partial class ConsoleServer
    {
        private static LogHandler.NanoLog _nanoLog;
        private static ReplySocket _socket;
        private static NanomsgListener _listener;
        private static Dictionary<string, CommandHandler> _commandHandlers;

        private static void InitImpl()
        {

            _nanoLog = new LogHandler.NanoLog("ws://*:5556");
            Log.LogEvent += _nanoLog.Log;


            _commandHandlers = new Dictionary<string, CommandHandler>();
            _socket = new ReplySocket();

            _socket.Bind("tcp://*:5557");
            _listener = new NanomsgListener();
            _listener.AddSocket(_socket);
            _listener.ReceivedMessage += socketId =>
            {
                var msg = _socket.Receive();
                ParseMsg(msg);
            };
        }

        private static void ParseMsg(byte[] msg)
        {
            var mpoDict =
                MessagePackSerializer.Create<Dictionary<MessagePackObject, MessagePackObject>>()
                    .Unpack(new MemoryStream(msg));

            string name = (string) mpoDict["name"];
            var args = mpoDict["args"].AsDictionary();

            if (_commandHandlers.ContainsKey(name))
            {
                _commandHandlers[name](args);
            }
            else
            {
                Log.Warning("console_server", "Invalid command \"{0}\"", name);
            }

        }

        private static void ShutdownImpl()
        {
            Log.LogEvent -= _nanoLog.Log;
            _nanoLog = null;
        }


        private static void TickImpl()
        {
            _listener.Listen(TimeSpan.FromMilliseconds(1));
        }

        private static void RegisterCommandImpl(string name, CommandHandler handler)
        {
            _commandHandlers[name] = handler;
        }
    }
}