using System;
using System.Collections.Generic;
using System.IO;
using CETech.Utils;
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


        private static void InitConfigImpl()
        {
            ConfigSystem.CreateValue("console_server.base_port", "First used port", 5556);
        }


        private static void InitImpl()
        {
            _nanoLog = new LogHandler.NanoLog($"ws://*:{ConfigSystem.GetValueInt("console_server.base_port")}");
            Log.LogEvent += _nanoLog.Log;


            _commandHandlers = new Dictionary<string, CommandHandler>();
            _socket = new ReplySocket();

            _socket.Bind($"tcp://*:{ConfigSystem.GetValueInt("console_server.base_port") + 1}");
            _listener = new NanomsgListener();
            _listener.AddSocket(_socket);
            _listener.ReceivedMessage += socketId =>
            {
                var msg = _socket.Receive();
                ParseMsg(msg);
            };

            RegisterCommand("wait", (args, response) => { });
        }

        private static void ParseMsg(byte[] msg)
        {
            var mpoDict =
                MessagePackSerializer.Get<Dictionary<MessagePackObject, MessagePackObject>>()
                    .Unpack(new MemoryStream(msg));

            var name = (string) mpoDict["name"];
            var args = mpoDict["args"].AsDictionary();

            //Log.Debug("console_server", "Recive cmd \"{0}\"", name);

            if (!_commandHandlers.ContainsKey(name))
            {
                Log.Warning("console_server", "Invalid command \"{0}\"", name);
                return;
            }

            var packer = new ResponsePacker();
            _commandHandlers[name](args, packer);

            var stream = packer.GetMemoryStream();
            if (stream.Length == 0)
            {
                packer.PackNull();
            }

            _socket.Send(stream.ToArray());
        }

        private static void ShutdownImpl()
        {
            Log.LogEvent -= _nanoLog.Log;
            _nanoLog = null;
        }


        private static void TickImpl()
        {
            _listener.Listen(TimeSpan.FromMilliseconds(0));
        }

        private static void RegisterCommandImpl(string name, CommandHandler handler)
        {
            _commandHandlers[name] = handler;
        }

        public partial class ResponsePacker
        {
            private MemoryStream _ms;
            private Packer _packer;

            private void CtorImpl()
            {
                _ms = new MemoryStream();
                _packer = Packer.Create(_ms);
            }

            private void PackNullImpl()
            {
                _packer.PackNull();
            }

            private void PackImpl(bool boolean)
            {
                _packer.Pack(boolean);
            }

            private void PackImpl(double number)
            {
                _packer.Pack(number);
            }

            private void PackImpl(int value)
            {
                _packer.Pack(value);
            }

            private void PackImpl(long value)
            {
                _packer.Pack(value);
            }


            private void PackImpl(string @string)
            {
                _packer.Pack(@string);
            }

            private void PackMapHeaderImpl(int count)
            {
                _packer.PackMapHeader(count);
            }

            private void PackArrayHeaderImpl(int length)
            {
                _packer.PackArrayHeader(length);
            }

            private MemoryStream GetMemoryStreamImpl()
            {
                return _ms;
            }

            private void ResetImpl()
            {
                _ms.Seek(0, SeekOrigin.Begin);
            }
        }
    }
}