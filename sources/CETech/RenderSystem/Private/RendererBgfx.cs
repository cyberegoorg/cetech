using System;
using CETech.Develop;
using CETech.Utils;
using SharpBgfx;

namespace CETech
{
    public static partial class RenderSystem
    {
        private static Data _data;
        static private CallbackHandler _callback_handler;

        private static RendererBackend ToRendererBackend(BackendType type)
        {
            switch (type)
            {
                case BackendType.Null:
                    return RendererBackend.Null;

                case BackendType.Direct3D9:
                    return RendererBackend.Direct3D9;

                case BackendType.Direct3D11:
                    return RendererBackend.Direct3D11;

                case BackendType.Direct3D12:
                    return RendererBackend.Direct3D12;

                case BackendType.Metal:
                    return RendererBackend.Metal;

                case BackendType.OpenGLES:
                    return RendererBackend.OpenGLES;

                case BackendType.OpenGL:
                    return RendererBackend.OpenGL;

                case BackendType.Vulkan:
                    return RendererBackend.Vulkan;

                case BackendType.Default:
                    return RendererBackend.Default;

                default:
                    throw new ArgumentOutOfRangeException("type", type, null);
            }
        }

        private static void InitImpl(Window window, BackendType type)
        {
			_callback_handler = null;//new CallbackHandler();

            Bgfx.SetPlatformData (new PlatformData {WindowHandle = window.NativeWindowPtr, DisplayType = window.NativeDisplayPtr } );
            Bgfx.Init(ToRendererBackend(type), callbackHandler: _callback_handler);
            Bgfx.SetDebugFeatures(DebugFeatures.DisplayStatistics);

            Resize(window.Width, window.Height);

            ConsoleServer.RegisterCommand("renderer.resize", (args, response) =>
            {
                Resize(args["width"].AsInt32(), args["height"].AsInt32());
                Application.MainWindow.Resize(args["width"].AsInt32(), args["height"].AsInt32());
            });
        }

        private static void BeginFrameImpl()
        {
            if (_data.NeedResize)
            {
                Bgfx.Reset(_data.ResizeW, _data.ResizeH);
                Bgfx.SetViewRect(0, 0, 0, _data.ResizeW, _data.ResizeH);
                _data.NeedResize = false;
            }

            Bgfx.SetDebugFeatures(DebugFeatures.DisplayStatistics | DebugFeatures.DisplayText);
            Bgfx.SetViewClear(0, ClearTargets.Color | ClearTargets.Depth, 0x66CCFFff);

            Bgfx.Touch(0);

            Bgfx.DebugTextClear();

            Bgfx.Submit(0, SharpBgfx.Program.Invalid);
        }

        private static void EndFrameImpl()
        {
            int frame = Bgfx.Frame();

#if CETECH_DEVELOP
            var stats = Bgfx.GetStats();
            DevelopSystem.PushRecordInt("renderer.frame", frame);
            DevelopSystem.PushRecordFloat("renderer.cpu_elapsed", (float)stats.CpuElapsed.TotalMilliseconds);
            DevelopSystem.PushRecordFloat("renderer.gpu_elapsed", (float)stats.GpuElapsed.TotalMilliseconds);
#endif

        }

        private static void ResizeImpl(int width, int height)
        {
            _data.NeedResize = true;
            _data.ResizeW = width;
            _data.ResizeH = height;
        }

        

        private struct Data
        {
            public int ResizeW;
            public int ResizeH;
            public bool NeedResize;
        }

    class CallbackHandler : ICallbackHandler
    {
        public void ReportError(ErrorType errorType, string message)
        {
        }

        public void ReportDebug(string fileName, int line, string format, IntPtr args)
        {
        }

        public int GetCachedSize(long id)
        {
            return 0;
        }

        public bool GetCacheEntry(long id, IntPtr data, int size)
        {
            return false;
        }

        public void SetCacheEntry(long id, IntPtr data, int size)
        {
        }

        public void SaveScreenShot(string path, int width, int height, int pitch, IntPtr data, int size, bool flipVertical)
        {
            Log.Debug("renderer.bgfx", "Save screenshot to \"{0}\"", path);
        }

        public void CaptureStarted(int width, int height, int pitch, TextureFormat format, bool flipVertical)
        {
        }

        public void CaptureFinished()
        {
        }

        public void CaptureFrame(IntPtr data, int size)
        {
        }
    }
    }
}
