using System;
using System.IO;
using System.Linq;
using CETech.Develop;
using CETech.Lua.Api;
using MoonSharp.Interpreter;
using MoonSharp.Interpreter.Loaders;

namespace CETech.Lua
{
    public static partial class LuaEnviroment
    {
        private static Script _enviromentScript;

        private static DynValue _initFce;
        private static DynValue _updateFce;
        private static DynValue _shutdownFce;

        private static void PackDynValue(DynValue value, ConsoleServer.ResponsePacker packer)
        {
            switch (value.Type)
            {
                case DataType.Nil:
                    packer.PackNull();
                    break;
                case DataType.Void:
                    packer.PackNull();
                    break;
                case DataType.Boolean:
                    packer.Pack(value.Boolean);
                    break;
                case DataType.Number:
                    packer.Pack(value.Number);
                    break;
                case DataType.String:
                    packer.Pack(value.String);
                    break;
                case DataType.Function:
                    break;
                case DataType.Table:
                    packer.PackMapHeader(value.Table.Pairs.Count());
                    foreach (var pair in value.Table.Pairs)
                    {
                        PackDynValue(pair.Key, packer);
                        PackDynValue(pair.Value, packer);
                    }
                    break;
                case DataType.Tuple:
                    packer.PackArrayHeader(value.Tuple.Length);
                    for (var i = 0; i < value.Tuple.Length; ++i)
                    {
                        PackDynValue(value.Tuple[i], packer);
                    }
                    break;
                case DataType.UserData:
                    break;
                case DataType.Thread:
                    break;
                case DataType.ClrFunction:
                    break;
                case DataType.TailCallRequest:
                    break;
                case DataType.YieldRequest:
                    break;
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }

        private static void InitImpl()
        {
            _enviromentScript = new Script();
            UserData.RegisterAssembly();

            _enviromentScript.Options.ScriptLoader = new ScriptLoader {ModulePaths = new[] {"?"}};
            _enviromentScript.Options.UseLuaErrorLocations = true;

            _enviromentScript.Globals["Application"] = new ApplicationApi();
            _enviromentScript.Globals["Log"] = new LogApi();
            _enviromentScript.Globals["Keyboard"] = new KeyboardApi();
            _enviromentScript.Globals["Mouse"] = new MouseApi();
            _enviromentScript.Globals["PackageManager"] = new PackageManagerApi();

            ConsoleServer.RegisterCommand("lua.execute", (args, response) =>
            {
                var ret = _enviromentScript.DoString(args["script"].AsString());
                PackDynValue(ret, response);
            });
        }

        private static void ShutdownImpl()
        {
            _enviromentScript = null;
        }

        private static void DoResourceImpl(long name)
        {
            var ms = new MemoryStream(ResourceManager.Get<byte[]>(LuaResource.Type, name));
            _enviromentScript.DoStream(ms);
        }

        private static void BootScriptInitImpl(long name)
        {
            DoResource(name);

            _initFce = _enviromentScript.Globals.Get("init");
            _updateFce = _enviromentScript.Globals.Get("update");
            _shutdownFce = _enviromentScript.Globals.Get("shutdown");
        }

        private static void BootScriptCallInitImpl()
        {
            _enviromentScript.Call(_initFce);
        }

        private static void BootScriptCallUpdateImpl(float dt)
        {
            _enviromentScript.Call(_updateFce, dt);
        }

        private static void BootScriptCallShutdownImpl()
        {
            _enviromentScript.Call(_shutdownFce);
        }

        private class ScriptLoader : ScriptLoaderBase
        {
            public override object LoadFile(string file, Table globalContext)
            {
                var name = StringId.FromString(file);
                return ResourceManager.Get<byte[]>(LuaResource.Type, name);
            }

            public override bool ScriptFileExists(string name)
            {
                long[] names = {StringId.FromString(name)};
                return ResourceManager.CanGet(LuaResource.Type, names);
            }
        }
    }
}