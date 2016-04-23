using CETech.CEMath;

namespace CETech
{
    /// <summary>
    ///     Render system
    /// </summary>
    public static partial class Renderer
    {
        /// <summary>
        ///     Render backend type.
        /// </summary>
        public enum BackendType
        {
            /// <summary>
            ///     No backend given.
            /// </summary>
            Null,

            /// <summary>
            ///     Direct3D 9
            /// </summary>
            Direct3D9,

            /// <summary>
            ///     Direct3D 11
            /// </summary>
            Direct3D11,

            /// <summary>
            ///     Direct3D 12
            /// </summary>
            Direct3D12,

            /// <summary>
            ///     Apple Metal.
            /// </summary>
            Metal,

            /// <summary>
            ///     OpenGL ES
            /// </summary>
            OpenGLES,

            /// <summary>
            ///     OpenGL
            /// </summary>
            OpenGL,

            /// <summary>
            ///     Vulkan
            /// </summary>
            Vulkan,

            /// <summary>
            ///     Used during initialization; specifies that the library should
            ///     pick the best renderer for the running hardware and OS.
            /// </summary>
            Default
        }

        /// <summary>
        ///     Init render system.
        /// </summary>
        /// <param name="window">Window for render context</param>
        /// <param name="type">Render backend</param>
        public static void Init(Window window, string backend)
        {
            InitImpl(window, backend);
        }

        public static void Shutdown()
		{
			ShutdownImpl ();
		}

        public static void RenderWorld(int world, int camera)
        {
            RenderWorldImpl(world, camera);
        }


		public static Vec2f GetSize() {
			return GetSizeImpl();
		}

        /// <summary>
        ///     Resize render view
        /// </summary>
        /// <param name="width">New width</param>
        /// <param name="height">New height</param>
        public static void Resize(int width, int height)
        {
            ResizeImpl(width, height);
        }

        public static void SetDebug(bool enabled)
        {
            SetDebugImpl(enabled);
        }

        public static void SaveScreenShot(string filename)
        {
            SaveScreenShotImpl(filename);
        }

        public static void BeginCapture()
        {
            BeginCaptureImpl();
        }


        public static void EndCapture()
        {
            EndCaptureImpl();
        }

        public static ulong TextureFormat(string format)
        {
            return TextureFormatImpl(format);
        }

        public static ulong BackbufferRation(string ration)
        {
            return BackbufferRationImpl(ration);
        }

        public static ulong RenderState(string key, string value)
        {
            return RenderStateImpl(key, value);
        }

    }
}