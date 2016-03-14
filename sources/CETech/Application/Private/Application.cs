using CETech.Develop;
using CETech.Input;
using CETech.Lua;

namespace CETech
{
    /// <summary>
    ///     Application
    /// </summary>
    public static partial class Application
    {
        //private static long _lastFrameTick;
        private static bool _run;

        /// <summary>
        ///     Init application
        /// </summary>
        private static void InitImpl()
        {
            LuaEnviroment.Init();
        }

        /// <summary>
        ///     Shutdown application
        /// </summary>
        private static void ShutdownImpl()
        {
            LuaEnviroment.Shutdown();
        }

        /// <summary>
        ///     Run application
        /// </summary>
        private static void RunImpl()
        {
            _run = true;

            RenderSystem.Init(MainWindow, RenderSystem.BackendType.Default);

            LuaEnviroment.BootScriptInit(StringId.FromString("lua/boot"));

            LuaEnviroment.BootScriptCallInit();
            while (_run)
            {
                //Debug.Assert(TaskManager.OpenTaskCount < 2);

                ConsoleServer.Tick();

                PlaformUpdateEvents();

                var frameTask = TaskManager.AddNull("frame");
                var keyboardTask = TaskManager.AddBegin("keyboard", delegate { Keyboard.Process(); }, null,
                    parent: frameTask);
                var mouseTask = TaskManager.AddBegin("mouseTask", delegate { Mouse.Process(); }, null, parent: frameTask);

                TaskManager.AddEnd(new[] {frameTask, keyboardTask, mouseTask});
                TaskManager.Wait(frameTask);

                LuaEnviroment.BootScriptCallUpdate(10.0f);

                RenderSystem.BeginFrame();
                RenderSystem.EndFrame();
                MainWindow.Update();
            }

            LuaEnviroment.BootScriptCallShutdown();

            MainWindow = null;

            Shutdown();
        }

        /// <summary>
        ///     Quit application
        /// </summary>
        private static void QuitImpl()
        {
            _run = false;
        }
    }
}