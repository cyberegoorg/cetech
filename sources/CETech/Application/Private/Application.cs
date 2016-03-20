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
		private static float _delta_time;

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

            LuaEnviroment.BootScriptInit(StringId.FromString(ConfigSystem.GetValueString("boot.script")));

            LuaEnviroment.BootScriptCallInit();

            DateTime last_frame_tick = DateTime.Now;
			DateTime curent_frame_tick;
            while (_run)
            {
				//Debug.Assert(TaskManager.OpenTaskCount < 2);
				DevelopSystem.FrameBegin();

				curent_frame_tick = DateTime.Now;
				_delta_time = (float)(curent_frame_tick - last_frame_tick).TotalMilliseconds;
				last_frame_tick = curent_frame_tick;


				DevelopSystem.PushRecordFloat ("application.dt", _delta_time);
                var updateScope = DevelopSystem.EnterScope();

                PlaformUpdateEvents();

                var frameTask = TaskManager.AddNull("frame");
                var keyboardTask = TaskManager.AddBegin("keyboard", delegate { var scope = DevelopSystem.EnterScope(); Keyboard.Process(); DevelopSystem.LeaveScope("Keyboard", scope); }, null,
                    parent: frameTask);
                var mouseTask = TaskManager.AddBegin("mouseTask", delegate { var scope = DevelopSystem.EnterScope();  Mouse.Process(); DevelopSystem.LeaveScope("Mouse", scope); }, null, parent: frameTask);

                TaskManager.AddEnd(new[] {frameTask, keyboardTask, mouseTask});
                TaskManager.Wait(frameTask);

				LuaEnviroment.BootScriptCallUpdate(_delta_time);

                RenderSystem.BeginFrame();
                RenderSystem.EndFrame();
                MainWindow.Update();

                DevelopSystem.LeaveScope("Application::Update", updateScope);
                DevelopSystem.PushRecordInt("gc.total_memory", (int)GC.GetTotalMemory(false));
                DevelopSystem.Send();

                ConsoleServer.Tick();
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