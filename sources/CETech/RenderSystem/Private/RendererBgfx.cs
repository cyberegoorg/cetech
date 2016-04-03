using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using CETech.Develop;
using CETech.Utils;
using CETech.World;
using SharpBgfx;

namespace CETech
{
    public static partial class RenderSystem
    {
        private static Data _data;
        private static CallbackHandler _callback_handler;
        private static bool Capture;

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
            _callback_handler = new CallbackHandler();

            Bgfx.SetPlatformData(new PlatformData
            {
                WindowHandle = window.NativeWindowPtr,
                DisplayType = window.NativeDisplayPtr
            });
            Bgfx.Init(ToRendererBackend(type), callbackHandler: _callback_handler);

            Resize(window.Width, window.Height);

            ConsoleServer.RegisterCommand("renderer.resize", (args, response) =>
            {
                var width = args["width"].AsInt32();
                var height = args["height"].AsInt32();

                Resize(width, height);
                window.Resize(width, height);
            });

            Bgfx.SetViewClear(0, ClearTargets.Color | ClearTargets.Depth, 0x66CCFFff);
        }

        private static void BeginFrameImpl()
        {
            if (_data.NeedResize)
            {
                Bgfx.Reset(_data.ResizeW, _data.ResizeH, Capture ? ResetFlags.Capture : 0 );
                _data.NeedResize = false;
            }

            Bgfx.SetViewRect(0, 0, 0, _data.ResizeW, _data.ResizeH);

            var viewMatrix = Matrix4f.CreateLookAt(new Vector3f(0.0f, 0.0f, -35.0f), Vector3f.Zero, Vector3f.UnitY);
            var projMatrix = Matrix4f.CreatePerspectiveFieldOfView((float) Math.PI/3,
                (float) _data.ResizeW/_data.ResizeH, 0.1f, 100.0f);

            unsafe
            {
                Bgfx.SetViewTransform(0, &viewMatrix.M11, &projMatrix.M11);
            }

            Bgfx.Touch(0);

            Bgfx.DebugTextClear();
        }

        private static void EndFrameImpl()
        {
            var frame = Bgfx.Frame();

#if CETECH_DEVELOP
            var stats = Bgfx.GetStats();
            DevelopSystem.PushRecordInt("renderer.frame", frame);
            DevelopSystem.PushRecordFloat("renderer.cpu_elapsed", (float) stats.CpuElapsed.TotalMilliseconds);
            DevelopSystem.PushRecordFloat("renderer.gpu_elapsed", (float) stats.GpuElapsed.TotalMilliseconds);
#endif
        }

        private static void ResizeImpl(int width, int height)
        {
            _data.NeedResize = true;
            _data.ResizeW = width;
            _data.ResizeH = height;
        }

        private static void RenderWorldImpl(int world)
        {
            BeginFrameImpl();
            PrimitiveMeshRenderer.RenderWorld(world);
            EndFrameImpl();
        }


        private struct Data
        {
            public int ResizeW;
            public int ResizeH;
            public bool NeedResize;
        }

        private class CallbackHandler : ICallbackHandler
        {
            AviWriter _aviWriter;

            public void ReportError(ErrorType errorType, string message)
            {
                Log.Error("renderer.bgfx", "{0}", message);
                Debugger.Break();
            }

            public void ReportDebug(string fileName, int line, string format, IntPtr args)
            {
                //Log.Debug("renderer.bgfx", "({0}:{1}) {2}", fileName, line, format); // TODO: args
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

            public void SaveScreenShot(string path, int width, int height, int pitch, IntPtr data, int size,
                bool flipVertical)
            {
                // save screenshot as TGA
                var filename = Path.ChangeExtension(path, "tga");
                var file = File.Create(filename);

                Log.Info("renderer.bgfx", "Save screenshot to \"{0}\"", filename);

                using (var writer = new BinaryWriter(file))
                {
                    // write header
                    var header = new byte[18];
                    header[2] = 2;      // uncompressed RGB
                    header[12] = (byte)width;
                    header[13] = (byte)(width >> 8);
                    header[14] = (byte)height;
                    header[15] = (byte)(height >> 8);
                    header[16] = 32;    // bpp
                    header[17] = 32;    // origin upper-left
                    writer.Write(header);

                    var destPitch = width * 4;
                    var srcPitch = pitch;
                    var dataPtr = data;
                    if (flipVertical)
                    {
                        dataPtr += srcPitch * (height - 1);
                        srcPitch = -srcPitch;
                    }

                    // write image data
                    var buffer = new byte[destPitch];
                    for (int y = 0; y < height; y++)
                    {
                        Marshal.Copy(dataPtr, buffer, 0, destPitch);
                        writer.Write(buffer);
                        dataPtr += srcPitch;
                    }
                }
            }

            public void CaptureStarted(int width, int height, int pitch, TextureFormat format, bool flipVertical)
            {
                Log.Info("renderer.bgfx", "Capture to avi begin.");
                _aviWriter = new AviWriter(File.Create("capture.avi", pitch * height), width, height, 30, !flipVertical);
            }

            public void CaptureFinished()
            {
                Log.Info("renderer.bgfx", "Capture to avi end.");
                _aviWriter.Close();
                _aviWriter = null;
            }

            public void CaptureFrame(IntPtr data, int size)
            {
                _aviWriter.WriteFrame(data, size);
            }
        }

        private static void SetDebugImpl(bool enabled)
        {
            if (enabled)
            {
                Bgfx.SetDebugFeatures(DebugFeatures.DisplayStatistics | DebugFeatures.DisplayText);
            }
            else
            {
                Bgfx.SetDebugFeatures(DebugFeatures.None);
            }
        }

        private static void SaveScreenShotImpl(string filename)
        {
            Bgfx.SaveScreenShot(filename);
        }

        private static void BeginCaptureImpl()
        {
            Capture = true;
            _data.NeedResize = true;
        }

        private static void EndCaptureImpl()
        {
            Capture = false;
            _data.NeedResize = true;
        }
    }
}