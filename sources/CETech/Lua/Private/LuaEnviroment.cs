using System;
using System.IO;
using System.Linq;
using CETech.CEMath;
using CETech.Develop;
using CETech.Lua.Api;
using CETech.Resource;
using CETech.Utils;
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
        private static DynValue _renderFce;

        private static bool _Pause;

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
            _Pause = false;
            _enviromentScript = new Script();
            UserData.RegisterAssembly();

            _enviromentScript.Options.ScriptLoader = new ScriptLoader {ModulePaths = new[] {"?"}};
            _enviromentScript.Options.UseLuaErrorLocations = true;

            _enviromentScript.Globals["Application"] = new ApplicationApi();
            _enviromentScript.Globals["Log"] = new LogApi();
            _enviromentScript.Globals["Keyboard"] = new KeyboardApi();
            _enviromentScript.Globals["Mouse"] = new MouseApi();
            _enviromentScript.Globals["Package"] = new PackageManagerApi();
            _enviromentScript.Globals["World"] = new WorldApi();
            _enviromentScript.Globals["Unit"] = new UnitManagerApi();
            _enviromentScript.Globals["Renderer"] = new RenderSystemApi();
            _enviromentScript.Globals["Transform"] = new TransformSystemApi();
            _enviromentScript.Globals["Camera"] = new CameraApi();

            _enviromentScript.Globals["Vec3f"] = new Vector3fApi();

            ConsoleServer.RegisterCommand("lua.execute", (args, response) =>
            {
                try
                {
                    var ret = _enviromentScript.DoString(args["script"].AsString());
                    PackDynValue(ret, response);
                }
                catch (ScriptRuntimeException ex)
                {
                    var msg = string.Format("Error: {0}", ex.DecoratedMessage);

                    Log.Error("lua", msg);

                    response.PackMapHeader(1);
                    response.Pack("error_msg");
                    response.Pack(msg);
                }
                catch (SyntaxErrorException syntaxEx)
                {
                    var msg = string.Format("Syntax error: {0}", syntaxEx.DecoratedMessage);

                    Log.Error("lua", msg);

                    response.PackMapHeader(1);
                    response.Pack("error_msg");
                    response.Pack(msg);
                }

            });
        }

        private static void ShutdownImpl()
        {
            _enviromentScript = null;
        }

        private static void DoResourceImpl(long name)
        {
            var ms = new MemoryStream(Resource.Resource.Get<byte[]>(LuaResource.Type, name));

            try {
                _enviromentScript.DoStream(ms);
            }
            catch (ScriptRuntimeException ex)
            {
                var msg = string.Format("error: {0}", ex.DecoratedMessage);
                Log.Error("lua", msg);
            }
            catch (SyntaxErrorException syntaxEx)
            {
                var msg = string.Format("Syntax error: {0}", syntaxEx.DecoratedMessage);
                Log.Error("lua", msg);
            }
        }

        private static void BootScriptInitImpl(long name)
        {
            DoResource(name);

            _initFce = _enviromentScript.Globals.Get("init");
            _updateFce = _enviromentScript.Globals.Get("update");
            _shutdownFce = _enviromentScript.Globals.Get("shutdown");
            _renderFce = _enviromentScript.Globals.Get("render");
        }

        private static void BootScriptCallInitImpl()
        {
            try
            {
                _enviromentScript.Call(_initFce);
            }
            catch (ScriptRuntimeException ex)
            {
                var msg = string.Format("Init error: {0}", ex.DecoratedMessage);
                Log.Error("lua", msg);
                _Pause = true;
            }
        }

        private static void BootScriptCallUpdateImpl(float dt)
        {
            if (!_Pause)
            {
                try
                {
                    _enviromentScript.Call(_updateFce, dt);
                }
                catch (ScriptRuntimeException ex)
                {
                    var msg = string.Format("Update error: {0}", ex.DecoratedMessage);
                    Log.Error("lua", msg);
                    _Pause = true;
                }
            }
        }

        private static void BootScriptCallShutdownImpl()
        {
            try
            {
                _enviromentScript.Call(_shutdownFce);
            }
            catch (ScriptRuntimeException ex)
            {
                var msg = string.Format("Shutdown error: {0}", ex.DecoratedMessage);
                Log.Error("lua", msg);
                _Pause = true;
            }
        }

        private static void BootScriptCallRenderImpl()
        {
            if (!_Pause)
            {
                try
                {
                    _enviromentScript.Call(_renderFce);
                }
                catch (ScriptRuntimeException ex)
                {
                    var msg = string.Format("Render error: {0}", ex.DecoratedMessage);
                    Log.Error("lua", msg);
                }
            }
        }

        private static void DoStreamImpl(Stream stream)
        {
            _enviromentScript.DoStream(stream);
        }

        [MoonSharpUserData]
        internal class Vector3fApi
        {
            public static readonly Vec3f Zero = new Vec3f();
            public static readonly Vec3f UnitX = new Vec3f(1.0f, 0.0f, 0.0f);
            public static readonly Vec3f UnitY = new Vec3f(0.0f, 1.0f, 0.0f);
            public static readonly Vec3f UnitZ = new Vec3f(0.0f, 0.0f, 1.0f);
            public static readonly Vec3f Unit = new Vec3f(1.0f, 1.0f, 1.0f);

            public static Vec3f make(float x, float y, float z)
            {
                return new Vec3f(x, y, z);
            }
        }

        private class ScriptLoader : ScriptLoaderBase
        {
            public override object LoadFile(string file, Table globalContext)
            {
                var name = StringId.FromString(file);
                return Resource.Resource.Get<byte[]>(LuaResource.Type, name);
            }

            public override bool ScriptFileExists(string name)
            {
                if (Resource.Resource.AutoLoad)
                {
                    return true;
                }

                long[] names = {StringId.FromString(name)};
                bool can_get = Resource.Resource.CanGet(LuaResource.Type, names);

                return can_get;
            }
        }
    }
}