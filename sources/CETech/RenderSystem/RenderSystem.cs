namespace CETech
{
    /// <summary>
    ///     Render system
    /// </summary>
    public static partial class RenderSystem
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
        public static void Init(Window window, BackendType type)
        {
            PlatformInit(window, type);
        }

        /// <summary>
        ///     Begin frame.
        /// </summary>
        public static void BeginFrame()
        {
            PlatformBeginFrame();
        }

        /// <summary>
        ///     End frame.
        /// </summary>
        public static void EndFrame()
        {
            PlatformEndFrame();
        }

        /// <summary>
        ///     Resize render view
        /// </summary>
        /// <param name="width">New width</param>
        /// <param name="height">New height</param>
        public static void Resize(int width, int height)
        {
            PlatformResize(width, height);
        }
    }
}