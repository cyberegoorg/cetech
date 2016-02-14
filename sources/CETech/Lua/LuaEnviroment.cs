using System.IO;
using CETech.Lua.Api;
using MoonSharp.Interpreter;
using MoonSharp.Interpreter.Loaders;

namespace CETech.Lua
{
    public static class LuaEnviroment
    {
        public static Script EnviromentScript { get; private set; }

        public static void Init()
        {
            EnviromentScript = new Script();
            UserData.RegisterAssembly();

            EnviromentScript.Options.ScriptLoader = new ScriptLoader {ModulePaths = new[] {"?"}};

            EnviromentScript.Globals["Log"] = new LogApi();
            EnviromentScript.Globals["Keyboard"] = new KeyboardApi();
        }

        public static void Shutdown()
        {
            EnviromentScript = null;
        }

        public static void DoResouece(StringId name)
        {
            var ms = new MemoryStream(ResourceManager.Get<byte[]>(LuaResource.Type, name));
            EnviromentScript.DoStream(ms);
        }

        private class ScriptLoader : ScriptLoaderBase
        {
            public override object LoadFile(string file, Table globalContext)
            {
                var name = new StringId(file);
                return ResourceManager.Get<byte[]>(LuaResource.Type, name);
            }

            public override bool ScriptFileExists(string name)
            {
                StringId[] names = {new StringId(name)};
                return ResourceManager.CanGet(LuaResource.Type, names);
            }
        }
    }
}