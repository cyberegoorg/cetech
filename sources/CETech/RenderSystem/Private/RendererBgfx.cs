using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using CETech.CEMath;
using CETech.Develop;
using CETech.Resource;
using CETech.Utils;
using CETech.World;
using SharpBgfx;

namespace CETech
{
    public static partial class Renderer
    {
        private static Data _data;
        private static CallbackHandler _callback_handler;
        private static bool Capture;

        private static RendererBackend ToRendererBackend(string type)
        {
            switch (type)
            {
                case "null":
                    return RendererBackend.Null;

                case "direct3d9":
                    return RendererBackend.Direct3D9;

                case "direct3d11":
                    return RendererBackend.Direct3D11;

                case "direct3d12":
                    return RendererBackend.Direct3D12;

                case "metal":
                    return RendererBackend.Metal;

                case "opengles":
                    return RendererBackend.OpenGLES;

                case "opengl":
                    return RendererBackend.OpenGL;

                case "vulkan":
                    return RendererBackend.Vulkan;

                case "default":
                    return RendererBackend.Default;

                default:
                    throw new ArgumentOutOfRangeException("type", type, null);
            }
        }

        private static void InitImpl(Window window, string type)
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

        private static void ShutdownImpl()
        {
            Bgfx.Shutdown();
        }

        private static void ResizeImpl(int width, int height)
        {
            _data.NeedResize = true;
            _data.ResizeW = width;
            _data.ResizeH = height;
        }

        public static Vec2f GetSizeImpl()
        {
            return new Vec2f(_data.ResizeW, _data.ResizeH);
        }

        private static void RenderWorldImpl(int world, int camera, int viewport)
        {
            if (_data.NeedResize)
            {
                Bgfx.Reset(_data.ResizeW, _data.ResizeH, (Capture ? ResetFlags.Capture : 0) | ResetFlags.Vsync);
                _data.NeedResize = false;
            }

            Mat4f viewMatrix;
            Mat4f projMatrix;
            CameraSystem.GetProjectView(world, camera, out projMatrix, out viewMatrix);

            unsafe
            {
                Bgfx.SetViewTransform(0, &viewMatrix.M11, &projMatrix.M11);
            }
            Bgfx.SetViewRect(0, 0, 0, _data.ResizeW, _data.ResizeH);

            Bgfx.Touch(0);
            Bgfx.DebugTextClear();

            MeshRenderer.RenderWorld(world);

            var frame = Bgfx.Frame();

#if CETECH_DEVELOP
            var stats = Bgfx.GetStats();
            DevelopSystem.PushRecordInt("renderer.frame", frame);
            DevelopSystem.PushRecordFloat("renderer.cpu_elapsed", (float) stats.CpuElapsed.TotalMilliseconds);
            DevelopSystem.PushRecordFloat("renderer.gpu_elapsed", (float) stats.GpuElapsed.TotalMilliseconds);
#endif
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

        private static ulong TextureFormatImpl(string format)
        {
            switch (format)
            {
                case "R1":
                    return (ulong) SharpBgfx.TextureFormat.R1;
                case "A8":
                    return (ulong) SharpBgfx.TextureFormat.A8;
                case "R8":
                    return (ulong) SharpBgfx.TextureFormat.R8;
                case "R8I":
                    return (ulong) SharpBgfx.TextureFormat.R8I;
                case "R8U":
                    return (ulong) SharpBgfx.TextureFormat.R8U;
                case "R8S":
                    return (ulong) SharpBgfx.TextureFormat.R8S;
                case "R16":
                    return (ulong) SharpBgfx.TextureFormat.R16;
                case "R16I":
                    return (ulong) SharpBgfx.TextureFormat.R16I;
                case "R16U":
                    return (ulong) SharpBgfx.TextureFormat.R16U;
                case "R16F":
                    return (ulong) SharpBgfx.TextureFormat.R16F;
                case "R16S":
                    return (ulong) SharpBgfx.TextureFormat.R16S;
                case "R32I":
                    return (ulong) SharpBgfx.TextureFormat.R32I;
                case "R32U":
                    return (ulong) SharpBgfx.TextureFormat.R32U;
                case "R32F":
                    return (ulong) SharpBgfx.TextureFormat.R32F;
                case "RG8":
                    return (ulong) SharpBgfx.TextureFormat.RG8;
                case "RG8I":
                    return (ulong) SharpBgfx.TextureFormat.RG8I;
                case "RG8U":
                    return (ulong) SharpBgfx.TextureFormat.RG8U;
                case "RG8S":
                    return (ulong) SharpBgfx.TextureFormat.RG8S;
                case "RG16":
                    return (ulong) SharpBgfx.TextureFormat.RG16;
                case "RG16I":
                    return (ulong) SharpBgfx.TextureFormat.RG16I;
                case "RG16U":
                    return (ulong) SharpBgfx.TextureFormat.RG16U;
                case "RG16F":
                    return (ulong) SharpBgfx.TextureFormat.RG16F;
                case "RG16S":
                    return (ulong) SharpBgfx.TextureFormat.RG16S;
                case "RG32I":
                    return (ulong) SharpBgfx.TextureFormat.RG32I;
                case "RG32U":
                    return (ulong) SharpBgfx.TextureFormat.RG32U;
                case "RG32F":
                    return (ulong) SharpBgfx.TextureFormat.RG32F;

                /* TODO:               case "RGB8":
                                    return (ulong)TextureFormat.RGB8;
                                case "RGB8I":
                                    return (ulong)TextureFormat.RGB8I;
                                case "RGB8U":
                                    return (ulong)TextureFormat.RGB8U;
                                case "RGB8S":
                                    return (ulong)TextureFormat.RGB8S;
                */

                case "RGB9E5F":
                    return (ulong) SharpBgfx.TextureFormat.RGB9E5F;
                case "BGRA8":
                    return (ulong) SharpBgfx.TextureFormat.BGRA8;
                case "RGBA8":
                    return (ulong) SharpBgfx.TextureFormat.RGBA8;
                case "RGBA8I":
                    return (ulong) SharpBgfx.TextureFormat.RGBA8I;
                case "RGBA8U":
                    return (ulong) SharpBgfx.TextureFormat.RGBA8U;
                case "RGBA8S":
                    return (ulong) SharpBgfx.TextureFormat.RGBA8S;
                case "RGBA16":
                    return (ulong) SharpBgfx.TextureFormat.RGBA16;
                case "RGBA16I":
                    return (ulong) SharpBgfx.TextureFormat.RGBA16I;
                case "RGBA16U":
                    return (ulong) SharpBgfx.TextureFormat.RGBA16U;
                case "RGBA16F":
                    return (ulong) SharpBgfx.TextureFormat.RGBA16F;
                case "RGBA16S":
                    return (ulong) SharpBgfx.TextureFormat.RGBA16S;
                case "RGBA32I":
                    return (ulong) SharpBgfx.TextureFormat.RGBA32I;
                case "RGBA32U":
                    return (ulong) SharpBgfx.TextureFormat.RGBA32U;
                case "RGBA32F":
                    return (ulong) SharpBgfx.TextureFormat.RGBA32F;
                case "R5G6B5":
                    return (ulong) SharpBgfx.TextureFormat.R5G6B5;
                case "RGBA4":
                    return (ulong) SharpBgfx.TextureFormat.RGBA4;
                case "RGB5A1":
                    return (ulong) SharpBgfx.TextureFormat.RGB5A1;
                case "RGB10A2":
                    return (ulong) SharpBgfx.TextureFormat.RGB10A2;
                case "R11G11B10F":
                    return (ulong) SharpBgfx.TextureFormat.R11G11B10F;
                case "D16":
                    return (ulong) SharpBgfx.TextureFormat.D16;
                case "D24":
                    return (ulong) SharpBgfx.TextureFormat.D24;
                case "D24S8":
                    return (ulong) SharpBgfx.TextureFormat.D24S8;
                case "D32":
                    return (ulong) SharpBgfx.TextureFormat.D32;
                case "D16F":
                    return (ulong) SharpBgfx.TextureFormat.D16F;
                case "D24F":
                    return (ulong) SharpBgfx.TextureFormat.D24F;
                case "D32F":
                    return (ulong) SharpBgfx.TextureFormat.D32F;
                case "D0S8":
                    return (ulong) SharpBgfx.TextureFormat.D0S8;
                default:
                    return 0;
            }
        }

        private static ulong BackbufferRationImpl(string ration)
        {
            switch (ration)
            {
                case "equal":
                    return (ulong) BackbufferRatio.Equal;

                case "half":
                    return (ulong) BackbufferRatio.Half;

                case "quater":
                    return (ulong) BackbufferRatio.Quater;

                case "eighth":
                    return (ulong) BackbufferRatio.Eighth;

                case "sixteenth":
                    return (ulong) BackbufferRatio.Sixteenth;

                case "double":
                    return (ulong) BackbufferRatio.Double;
                default:
                    return 0;
            }
        }

        private static ulong RenderStateImpl(string key, string value)
        {
            switch (key)
            {
                case "color_write":
                    if (value == "true") return (ulong) SharpBgfx.RenderState.ColorWrite;
                    return 0;

                case "alpha_write":
                    if (value == "true") return (ulong) SharpBgfx.RenderState.AlphaWrite;
                    return 0;

                case "depth_write":
                    if (value == "true") return (ulong) SharpBgfx.RenderState.DepthWrite;
                    return 0;

                case "cull_mode":
                    switch (value)
                    {
                        case "none":
                            return (ulong) SharpBgfx.RenderState.NoCulling;

                        case "cw":
                            return (ulong) SharpBgfx.RenderState.CullClockwise;

                        case "ccw":
                            return (ulong) SharpBgfx.RenderState.CullCounterclockwise;

                        default:
                            return 0;
                    }

                case "depth_func":
                    switch (value)
                    {
                        case "less":
                            return (ulong) SharpBgfx.RenderState.DepthTestLess;

                        case "less_equal":
                            return (ulong) SharpBgfx.RenderState.DepthTestLessEqual;

                        case "equal":
                            return (ulong) SharpBgfx.RenderState.DepthTestEqual;

                        case "greater_equal":
                            return (ulong) SharpBgfx.RenderState.DepthTestGreaterEqual;

                        case "greater":
                            return (ulong) SharpBgfx.RenderState.DepthTestGreater;

                        case "not_equal":
                            return (ulong) SharpBgfx.RenderState.DepthTestNotEqual;

                        case "never":
                            return (ulong) SharpBgfx.RenderState.DepthTestNever;

                        case "always":
                            return (ulong) SharpBgfx.RenderState.DepthTestAlways;

                        default:
                            return 0;
                    }

                default:
                    return 0;
            }
        }


        private struct Data
        {
            public int ResizeW;
            public int ResizeH;
            public bool NeedResize;
        }

        private class CallbackHandler : ICallbackHandler
        {
            private AviWriter _aviWriter;

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
                // TODO: IMPLEMENT
                return 0;
            }

            public bool GetCacheEntry(long id, IntPtr data, int size)
            {
                // TODO: IMPLEMENT
                return false;
            }

            public void SetCacheEntry(long id, IntPtr data, int size)
            {
                // TODO: IMPLEMENT
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
                    header[2] = 2; // uncompressed RGB
                    header[12] = (byte) width;
                    header[13] = (byte) (width >> 8);
                    header[14] = (byte) height;
                    header[15] = (byte) (height >> 8);
                    header[16] = 32; // bpp
                    header[17] = 32; // origin upper-left
                    writer.Write(header);

                    var destPitch = width*4;
                    var srcPitch = pitch;
                    var dataPtr = data;
                    if (flipVertical)
                    {
                        dataPtr += srcPitch*(height - 1);
                        srcPitch = -srcPitch;
                    }

                    // write image data
                    var buffer = new byte[destPitch];
                    for (var y = 0; y < height; y++)
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
                _aviWriter = new AviWriter(File.Create("capture.avi", pitch*height), width, height, 30, !flipVertical);
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

        private static int GetViewportImpl(long name)
        {
            var resource = Resource.Resource.Get<RenderConfig.ConfigInstance>(RenderConfig.Type,
                StringId.FromString("default"));

            for (int i = 0; i < resource.resource.ViewportName.Count; i++)
            {
                if (resource.resource.ViewportName[i] != name)
                {
                    continue;
                }

                return i;
            }

            return -1; //TODO:
        }
    }
}