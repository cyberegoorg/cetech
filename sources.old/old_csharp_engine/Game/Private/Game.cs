using CETech.Lua;

namespace CETech
{
    public partial class Game
    {
        private static void InitImpl()
        {
            LuaEnviroment.BootScriptCallInit();
        }

        private static void ShutdownImpl()
        {
            LuaEnviroment.BootScriptCallShutdown();
        }

        private static void UpdateImpl(float dt)
        {
            LuaEnviroment.BootScriptCallUpdate(dt);
        }

        private static void RenderImpl()
        {
            LuaEnviroment.BootScriptCallRender();
        }
    }
}