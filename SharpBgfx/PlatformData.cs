using System;

namespace SharpBgfx {
    /// <summary>
    /// Contains platform-specific data used to hook into the bgfx library.
    /// </summary>
    public struct PlatformData {
        /// <summary>
        /// EGL native display type.
        /// </summary>
        public IntPtr DisplayType;

        /// <summary>
        /// Platform window handle.
        /// </summary>
        public IntPtr WindowHandle;

        /// <summary>
        /// Device context to use instead of letting the library create its own.
        /// </summary>
        public IntPtr Context;

        /// <summary>
        /// Backbuffer pointer to use instead of letting the library create its own.
        /// </summary>
        public IntPtr Backbuffer;

        /// <summary>
        /// Depth-stencil pointer to use instead of letting the library create its own.
        /// </summary>
        public IntPtr BackbufferDepthStencil;
    }
}
