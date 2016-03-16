using System;
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

            LuaEnviroment.BootScriptInit(StringId.FromString(Config.GetValueString("boot.script")));

            LuaEnviroment.BootScriptCallInit();
            while (_run)
            {
                //Debug.Assert(TaskManager.OpenTaskCount < 2);

                ConsoleServer.Tick();
                DevelopSystem.FrameBegin();
                var updateScope = DevelopSystem.EnterScope();

                PlaformUpdateEvents();

                var frameTask = TaskManager.AddNull("frame");
                var keyboardTask = TaskManager.AddBegin("keyboard", delegate { var scope = DevelopSystem.EnterScope(); Keyboard.Process(); DevelopSystem.LeaveScope("Keyboard", scope); }, null,
                    parent: frameTask);
                var mouseTask = TaskManager.AddBegin("mouseTask", delegate { var scope = DevelopSystem.EnterScope();  Mouse.Process(); DevelopSystem.LeaveScope("Mouse", scope); }, null, parent: frameTask);

                TaskManager.AddEnd(new[] {frameTask, keyboardTask, mouseTask});
                TaskManager.Wait(frameTask);

                LuaEnviroment.BootScriptCallUpdate(10.0f);

                RenderSystem.BeginFrame();
                RenderSystem.EndFrame();
                MainWindow.Update();

                DevelopSystem.LeaveScope("Application::Update", updateScope);
                DevelopSystem.Send();
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