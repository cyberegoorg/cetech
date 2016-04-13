using System.IO;
using CETech.Resource;

namespace CETech.Lua
{
    /// <summary>
    ///     Main lua enviroment.
    /// </summary>
    public static partial class LuaEnviroment
    {
        /// <summary>
        ///     Init lua enviroment
        /// </summary>
        public static void Init()
        {
            InitImpl();
        }

        /// <summary>
        ///     Shutdown lua enviroment
        /// </summary>
        public static void Shutdown()
        {
            ShutdownImpl();
        }

        /// <summary>
        ///     Run script
        /// </summary>
        /// <param name="name">Script resource name</param>
        public static void DoResource(long name)
        {
            DoResourceImpl(name);
        }

        public static void DoStream(Stream stream)
        {
            DoStreamImpl(stream);
        }

        /// <summary>
        ///     Init boot script
        /// </summary>
        /// <param name="name">Boot sript name</param>
        public static void BootScriptInit(long name)
        {
            BootScriptInitImpl(name);
        }

        /// <summary>
        ///     Call boot script init fce.
        /// </summary>
        public static void BootScriptCallInit()
        {
            BootScriptInit(StringId.FromString(ConfigSystem.GetValueString("boot.script")));
            BootScriptCallInitImpl();
        }

        /// <summary>
        ///     Call boot script update
        /// </summary>
        /// <param name="dt">Deltatime</param>
        public static void BootScriptCallUpdate(float dt)
        {
            BootScriptCallUpdateImpl(dt);
        }

        public static void BootScriptCallRender()
        {
            BootScriptCallRenderImpl();
        }

        /// <summary>
        ///     Call boot script shutdown
        /// </summary>
        public static void BootScriptCallShutdown()
        {
            BootScriptCallShutdownImpl();
        }
    }
}