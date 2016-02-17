using System;
using SharpBgfx;

namespace CETech
{
    public static partial class RenderSystem
    {
        private static Data _data;

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
            Bgfx.SetWindowHandle(window.NativeWindowPtr);
            Bgfx.Init(ToRendererBackend(type));
            Bgfx.SetDebugFeatures(DebugFeatures.DisplayStatistics);

            Resize(window.Width, window.Height);
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
            Bgfx.Frame();
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
    }
}