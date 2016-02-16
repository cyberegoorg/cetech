using System.IO;
using CETech.Lua.Api;
using MoonSharp.Interpreter;
using MoonSharp.Interpreter.Loaders;

namespace CETech.Lua
{
    public static class LuaEnviroment
    {
        private static Script _enviromentScript;

        private static DynValue _initFce;
        private static DynValue _updateFce;
        private static DynValue _shutdownFce;

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
        }

        public static void Shutdown()
        {
            _enviromentScript = null;
        }

        public static void DoResouece(long name)
        {
            var ms = new MemoryStream(ResourceManager.Get<byte[]>(LuaResource.Type, name));
            _enviromentScript.DoStream(ms);
        }

        public static void BootScriptInit(long name)
        {
            DoResouece(name);

            _initFce = _enviromentScript.Globals.Get("init");
            _updateFce = _enviromentScript.Globals.Get("update");
            _shutdownFce = _enviromentScript.Globals.Get("shutdown");
        }

        public static void BootScriptCallInit()
        {
            _enviromentScript.Call(_initFce);
        }

        public static void BootScriptCallUpdate(float dt)
        {
            _enviromentScript.Call(_updateFce, dt);
        }

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