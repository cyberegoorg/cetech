using System;
using System.IO;
using System.Linq;
using CETech.Develop;
using CETech.Lua.Api;
using MoonSharp.Interpreter;
using MoonSharp.Interpreter.Loaders;
using MsgPack;

namespace CETech.Lua
{
    /// <summary>
    ///     Main lua enviroment.
    /// </summary>
    public static class LuaEnviroment
    {
        private static Script _enviromentScript;

        private static DynValue _initFce;
        private static DynValue _updateFce;
        private static DynValue _shutdownFce;

        private static void PackDynValue(DynValue value, Packer packer)
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

        /// <summary>
        ///     Init lua enviroment
        /// </summary>
        public static void Init()
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
                DynValue ret = _enviromentScript.DoString(args["script"].AsString());
                PackDynValue(ret, response);
            });
        }

        /// <summary>
        ///     Shutdown lua enviroment
        /// </summary>
        public static void Shutdown()
        {
            _enviromentScript = null;
        }

        /// <summary>
        ///     Run script
        /// </summary>
        /// <param name="name">Script resource name</param>
        public static void DoResource(long name)
        {
            var ms = new MemoryStream(ResourceManager.Get<byte[]>(LuaResource.Type, name));
            _enviromentScript.DoStream(ms);
        }

        /// <summary>
        ///     Init boot script
        /// </summary>
        /// <param name="name">Boot sript name</param>
        public static void BootScriptInit(long name)
        {
            DoResource(name);

            _initFce = _enviromentScript.Globals.Get("init");
            _updateFce = _enviromentScript.Globals.Get("update");
            _shutdownFce = _enviromentScript.Globals.Get("shutdown");
        }

        /// <summary>
        ///     Call boot script init fce.
        /// </summary>
        public static void BootScriptCallInit()
        {
            _enviromentScript.Call(_initFce);
        }

        /// <summary>
        ///     Call boot script update
        /// </summary>
        /// <param name="dt">Deltatime</param>
        public static void BootScriptCallUpdate(float dt)
        {
            _enviromentScript.Call(_updateFce, dt);
        }

        /// <summary>
        ///     Call boot script shutdown
        /// </summary>
        public static void BootScriptCallShutdown()
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