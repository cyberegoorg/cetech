using System.IO;
using CETech.Lua.Api;
using MoonSharp.Interpreter;
using MoonSharp.Interpreter.Loaders;

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