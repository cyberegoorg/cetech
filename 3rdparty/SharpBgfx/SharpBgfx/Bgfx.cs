using System;
using System.Globalization;
using System.Linq;

namespace SharpBgfx {
    /// <summary>
    /// Managed interface to the bgfx graphics library.
    /// </summary>
    public unsafe static class Bgfx {
        /// <summary>
        /// Checks for available space to allocate transient index and vertex buffers.
        /// </summary>
        /// <param name="vertexCount">The number of vertices to allocate.</param>
        /// <param name="layout">The layout of each vertex.</param>
        /// <param name="indexCount">The number of indices to allocate.</param>
        /// <returns><c>true</c> if there is sufficient space for both vertex and index buffers.</returns>
        public static bool CheckAvailableTransientBufferSpace (int vertexCount, VertexLayout layout, int indexCount) {
            return NativeMethods.bgfx_check_avail_transient_buffers(vertexCount, ref layout.data, indexCount);
        }

        /// <summary>
        /// Attempts to allocate both a transient vertex buffer and index buffer.
        /// </summary>
        /// <param name="vertexCount">The number of vertices to allocate.</param>
        /// <param name="layout">The layout of each vertex.</param>
        /// <param name="indexCount">The number of indices to allocate.</param>
        /// <param name="vertexBuffer">Returns the allocated transient vertex buffer.</param>
        /// <param name="indexBuffer">Returns the allocated transient index buffer.</param>
        /// <returns><c>true</c> if both space requirements are satisfied and the buffers were allocated.</returns>
        public static bool AllocateTransientBuffers (int vertexCount, VertexLayout layout, int indexCount, out TransientVertexBuffer vertexBuffer, out TransientIndexBuffer indexBuffer) {
            return NativeMethods.bgfx_alloc_transient_buffers(out vertexBuffer, ref layout.data, (ushort)vertexCount, out indexBuffer, (ushort)indexCount);
        }

        /// <summary>
        /// Packs a vector into vertex stream format.
        /// </summary>
        /// <param name="input">The four element vector to pack.</param>
        /// <param name="inputNormalized"><c>true</c> if the input vector is normalized.</param>
        /// <param name="attribute">The attribute usage of the vector data.</param>
        /// <param name="layout">The layout of the vertex stream.</param>
        /// <param name="data">The pointer to the vertex data stream.</param>
        /// <param name="index">The index of the vertex within the stream.</param>
        public static void VertexPack (float* input, bool inputNormalized, VertexAttributeUsage attribute, VertexLayout layout, IntPtr data, int index = 0) {
            NativeMethods.bgfx_vertex_pack(input, inputNormalized, attribute, ref layout.data, data, index);
        }

        /// <summary>
        /// Unpack a vector from a vertex stream.
        /// </summary>
        /// <param name="output">A pointer to four floats that will receive the unpacked vector.</param>
        /// <param name="attribute">The usage of the vertex attribute.</param>
        /// <param name="layout">The layout of the vertex stream.</param>
        /// <param name="data">A pointer to the vertex data stream.</param>
        /// <param name="index">The index of the vertex within the stream.</param>
        public static void VertexUnpack (float* output, VertexAttributeUsage attribute, VertexLayout layout, IntPtr data, int index = 0) {
            NativeMethods.bgfx_vertex_unpack(output, attribute, ref layout.data, data, index);
        }

        /// <summary>
        /// Converts a stream of vertex data from one format to another.
        /// </summary>
        /// <param name="destinationLayout">The destination format.</param>
        /// <param name="destinationData">A pointer to the output location.</param>
        /// <param name="sourceLayout">The source format.</param>
        /// <param name="sourceData">A pointer to the source vertex data to convert.</param>
        /// <param name="count">The number of vertices to convert.</param>
        public static void VertexConvert (VertexLayout destinationLayout, IntPtr destinationData, VertexLayout sourceLayout, IntPtr sourceData, int count = 1) {
            NativeMethods.bgfx_vertex_convert(ref destinationLayout.data, destinationData, ref sourceLayout.data, sourceData, count);
        }

        /// <summary>
        /// Welds vertices that are close together.
        /// </summary>
        /// <param name="layout">The layout of the vertex stream.</param>
        /// <param name="data">A pointer to the vertex data stream.</param>
        /// <param name="count">The number of vertices in the stream.</param>
        /// <param name="remappingTable">An output remapping table from the original vertices to the welded ones.</param>
        /// <param name="epsilon">The tolerance for welding vertex positions.</param>
        /// <returns>
        /// The number of unique vertices after welding.
        /// </returns>
        public static int WeldVertices (VertexLayout layout, IntPtr data, int count, out int[] remappingTable, float epsilon = 0.001f) {
            var output = stackalloc ushort[count];
            var result = NativeMethods.bgfx_weld_vertices(output, ref layout.data, data, (ushort)count, epsilon);

            remappingTable = new int[count];
            for (int i = 0; i < count; i++)
                remappingTable[i] = output[i];

            return result;
        }

        /// <summary>
        /// Swizzles an RGBA8 image to BGRA8.
        /// </summary>
        /// <param name="width">The width of the image.</param>
        /// <param name="height">The height of the image.</param>
        /// <param name="pitch">The pitch of the image (in bytes).</param>
        /// <param name="source">The source image data.</param>
        /// <param name="destination">The destination image data.</param>
        /// <remarks>
        /// This method can operate in-place on the image (i.e. src == dst).
        /// </remarks>
        public static void ImageSwizzleBgra8 (int width, int height, int pitch, IntPtr source, IntPtr destination) {
            NativeMethods.bgfx_image_swizzle_bgra8(width, height, pitch, source, destination);
        }

        /// <summary>
        /// Downsamples an RGBA8 image with a 2x2 pixel average filter.
        /// </summary>
        /// <param name="width">The width of the image.</param>
        /// <param name="height">The height of the image.</param>
        /// <param name="pitch">The pitch of the image (in bytes).</param>
        /// <param name="source">The source image data.</param>
        /// <param name="destination">The destination image data.</param>
        /// <remarks>
        /// This method can operate in-place on the image (i.e. src == dst).
        /// </remarks>
        public static void ImageRgba8Downsample2x2 (int width, int height, int pitch, IntPtr source, IntPtr destination) {
            NativeMethods.bgfx_image_rgba8_downsample_2x2(width, height, pitch, source, destination);
        }

        /// <summary>
        /// Sets platform-specific data pointers to hook into low-level library functionality.
        /// </summary>
        /// <param name="platformData">A collection of platform-specific data pointers.</param>
        public static void SetPlatformData (PlatformData platformData) {
            NativeMethods.bgfx_set_platform_data(ref platformData);
        }

        /// <summary>
        /// Sets the handle of the main rendering window.
        /// </summary>
        /// <param name="windowHandle">The handle of the native OS window.</param>
        public static void SetWindowHandle (IntPtr windowHandle) {
            var data = new PlatformData { WindowHandle = windowHandle };
            NativeMethods.bgfx_set_platform_data(ref data);
        }

        /// <summary>
        /// Gets the currently active rendering backend API.
        /// </summary>
        /// <returns>The currently active rendering backend.</returns>
        public static RendererBackend GetCurrentBackend () {
            return NativeMethods.bgfx_get_renderer_type();
        }

        /// <summary>
        /// Closes the library and releases all resources.
        /// </summary>
        public static void Shutdown () {
            NativeMethods.bgfx_shutdown();
            CallbackShim.FreeShim();
        }

        /// <summary>
        /// Gets the capabilities of the rendering device.
        /// </summary>
        /// <returns>Information about the capabilities of the device.</returns>
        public static Capabilities GetCaps () {
            return new Capabilities(NativeMethods.bgfx_get_caps());
        }

        /// <summary>
        /// Gets frame performance statistics.
        /// </summary>
        /// <returns>Information about frame performance.</returns>
        public static PerfStats GetStats () {
            return new PerfStats(NativeMethods.bgfx_get_stats());
        }

        /// <summary>
        /// Resets graphics settings and surfaces.
        /// </summary>
        /// <param name="width">The width of the main window.</param>
        /// <param name="height">The height of the main window.</param>
        /// <param name="flags">Flags used to configure rendering output.</param>
        public static void Reset (int width, int height, ResetFlags flags = ResetFlags.None) {
            NativeMethods.bgfx_reset(width, height, flags);
        }

        /// <summary>
        /// Advances to the next frame.
        /// </summary>
        /// <returns>The current frame number.</returns>
        /// <remarks>
        /// When using a multithreaded renderer, this call
        /// just swaps internal buffers, kicks render thread, and returns. In a
        /// singlethreaded renderer this call does frame rendering.
        /// </remarks>
        public static int Frame () {
            return NativeMethods.bgfx_frame();
        }

        /// <summary>
		/// Initializes the graphics library on the specified adapter.
		/// </summary>
		/// <param name="backend">The backend API to use for rendering.</param>
        /// <param name="adapter">The adapter on which to create the device.</param>
        /// <param name="callbackHandler">A set of handlers for various library callbacks.</param>
		public static void Init (RendererBackend backend = RendererBackend.Default, Adapter adapter = default(Adapter), ICallbackHandler callbackHandler = null) {
            NativeMethods.bgfx_init(
                backend,
                (ushort)adapter.Vendor,
                (ushort)adapter.DeviceId,
                CallbackShim.CreateShim(callbackHandler),
                IntPtr.Zero
            );
        }

        /// <summary>
        /// Gets the set of supported rendering backends.
        /// </summary>
        /// <returns></returns>
        public static RendererBackend[] GetSupportedBackends () {
            var types = new RendererBackend[(int)RendererBackend.Default];
            var count = NativeMethods.bgfx_get_supported_renderers(types);

            return types.Take(count).ToArray();
        }

        /// <summary>
        /// Gets the friendly name of a specific rendering backend.
        /// </summary>
        /// <param name="backend">The backend for which to retrieve a name.</param>
        /// <returns>The friendly name of the specified backend.</returns>
        public static string GetBackendName (RendererBackend backend) {
            return new string(NativeMethods.bgfx_get_renderer_name(backend));
        }

        /// <summary>
        /// Enables debugging features.
        /// </summary>
        /// <param name="features">The set of debug features to enable.</param>
        public static void SetDebugFeatures (DebugFeatures features) {
            NativeMethods.bgfx_set_debug(features);
        }

        /// <summary>
        /// Sets a marker that can be used for debugging purposes.
        /// </summary>
        /// <param name="marker">The user-defined name of the marker.</param>
        public static void SetDebugMarker (string marker) {
            NativeMethods.bgfx_set_marker(marker);
        }

        /// <summary>
        /// Clears the debug text buffer.
        /// </summary>
        /// <param name="color">The color with which to clear the background.</param>
        /// <param name="smallText"><c>true</c> to use a small font for debug output; <c>false</c> to use normal sized text.</param>
        public static void DebugTextClear (DebugColor color = DebugColor.Transparent, bool smallText = false) {
            var attr = (byte)((byte)color << 4);
            NativeMethods.bgfx_dbg_text_clear(attr, smallText);
        }

        /// <summary>
        /// Writes debug text to the screen.
        /// </summary>
        /// <param name="x">The X position, in cells.</param>
        /// <param name="y">The Y position, in cells.</param>
        /// <param name="color">The color of the text.</param>
        /// <param name="format">The format of the message.</param>
        /// <param name="args">The arguments with which to format the message.</param>
        public static void DebugTextWrite (int x, int y, DebugColor foreColor, DebugColor backColor, string format, params object[] args) {
            DebugTextWrite(x, y, foreColor, backColor, string.Format(CultureInfo.CurrentCulture, format, args));
        }

        /// <summary>
        /// Writes debug text to the screen.
        /// </summary>
        /// <param name="x">The X position, in cells.</param>
        /// <param name="y">The Y position, in cells.</param>
        /// <param name="color">The color of the text.</param>
        /// <param name="message">The message to write.</param>
        public static void DebugTextWrite (int x, int y, DebugColor foreColor, DebugColor backColor, string message) {
            var attr = (byte)(((byte)backColor << 4) | (byte)foreColor);
            NativeMethods.bgfx_dbg_text_printf((ushort)x, (ushort)y, attr, "%s", message);
        }

        /// <summary>
        /// Writes debug text to the screen.
        /// </summary>
        /// <param name="x">The X position, in cells.</param>
        /// <param name="y">The Y position, in cells.</param>
        /// <param name="color">The color of the text.</param>
        /// <param name="message">The message to write.</param>
        public static void DebugTextWrite (int x, int y, DebugColor foreColor, DebugColor backColor, IntPtr message) {
            var attr = (byte)(((byte)backColor << 4) | (byte)foreColor);
            var format = stackalloc byte[3];
            format[0] = (byte)'%';
            format[1] = (byte)'s';
            format[2] = 0;
            NativeMethods.bgfx_dbg_text_printf((ushort)x, (ushort)y, attr, format, message);
        }

        /// <summary>
        /// Draws data directly into the debug text buffer.
        /// </summary>
        /// <param name="x">The X position, in cells.</param>
        /// <param name="y">The Y position, in cells.</param>
        /// <param name="width">The width of the image to draw.</param>
        /// <param name="height">The height of the image to draw.</param>
        /// <param name="data">The image data bytes.</param>
        /// <param name="pitch">The pitch of each line in the image data.</param>
        public static void DebugTextImage (int x, int y, int width, int height, IntPtr data, int pitch) {
            NativeMethods.bgfx_dbg_text_image((ushort)x, (ushort)y, (ushort)width, (ushort)height, data, (ushort)pitch);
        }

        /// <summary>
        /// Sets the name of a rendering view, for debugging purposes.
        /// </summary>
        /// <param name="id">The index of the view.</param>
        /// <param name="name">The name of the view.</param>
        public static void SetViewName (byte id, string name) {
            NativeMethods.bgfx_set_view_name(id, name);
        }

        /// <summary>
        /// Sets the viewport for the given rendering view.
        /// </summary>
        /// <param name="id">The index of the view.</param>
        /// <param name="x">The X coordinate of the viewport.</param>
        /// <param name="y">The Y coordinate of the viewport.</param>
        /// <param name="width">The width of the viewport, in pixels.</param>
        /// <param name="height">The height of the viewport, in pixels.</param>
        public static void SetViewRect (byte id, int x, int y, int width, int height) {
            NativeMethods.bgfx_set_view_rect(id, (ushort)x, (ushort)y, (ushort)width, (ushort)height);
        }

        /// <summary>
        /// Sets the viewport for the given rendering view.
        /// </summary>
        /// <param name="id">The index of the view.</param>
        /// <param name="x">The X coordinate of the viewport.</param>
        /// <param name="y">The Y coordinate of the viewport.</param>
        /// <param name="ratio">The ratio with which to automatically size the viewport.</param>
        public static void SetViewRect (byte id, int x, int y, BackbufferRatio ratio) {
            NativeMethods.bgfx_set_view_rect_auto(id, (ushort)x, (ushort)y, ratio);
        }

        /// <summary>
        /// Sets the scissor rectangle for a specific view.
        /// </summary>
        /// <param name="id">The index of the view.</param>
        /// <param name="x">The X coordinate of the scissor rectangle.</param>
        /// <param name="y">The Y coordinate of the scissor rectangle.</param>
        /// <param name="width">The width of the scissor rectangle.</param>
        /// <param name="height">The height of the scissor rectangle.</param>
        /// <remarks>
        /// Set all values to zero to disable the scissor test.
        /// </remarks>
        public static void SetViewScissor (byte id, int x, int y, int width, int height) {
            NativeMethods.bgfx_set_view_scissor(id, (ushort)x, (ushort)y, (ushort)width, (ushort)height);
        }

        /// <summary>
        /// Sets view clear flags.
        /// </summary>
        /// <param name="id">The index of the view.</param>
        /// <param name="targets">The target surfaces that should be cleared.</param>
        /// <param name="colorRgba">The clear color.</param>
        /// <param name="depth">The value to fill the depth buffer.</param>
        /// <param name="stencil">The value to fill the stencil buffer.</param>
        public static void SetViewClear (byte id, ClearTargets targets, int colorRgba, float depth = 1.0f, byte stencil = 0) {
            NativeMethods.bgfx_set_view_clear(id, targets, colorRgba, depth, stencil);
        }

        /// <summary>
        /// Sets view clear flags for multiple render targets.
        /// </summary>
        /// <param name="id">The index of the view.</param>
        /// <param name="targets">The target surfaces that should be cleared.</param>
        /// <param name="depth">The value to fill the depth buffer.</param>
        /// <param name="stencil">The value to fill the stencil buffer.</param>
        /// <param name="rt0">The color palette index for render target 0.</param>
        /// <param name="rt1">The color palette index for render target 1.</param>
        /// <param name="rt2">The color palette index for render target 2.</param>
        /// <param name="rt3">The color palette index for render target 3.</param>
        /// <param name="rt4">The color palette index for render target 4.</param>
        /// <param name="rt5">The color palette index for render target 5.</param>
        /// <param name="rt6">The color palette index for render target 6.</param>
        /// <param name="rt7">The color palette index for render target 7.</param>
        public static void SetViewClear (
            byte id,
            ClearTargets targets,
            float depth,
            byte stencil,
            byte rt0 = byte.MaxValue,
            byte rt1 = byte.MaxValue,
            byte rt2 = byte.MaxValue,
            byte rt3 = byte.MaxValue,
            byte rt4 = byte.MaxValue,
            byte rt5 = byte.MaxValue,
            byte rt6 = byte.MaxValue,
            byte rt7 = byte.MaxValue
        ) {
            NativeMethods.bgfx_set_view_clear_mrt(
                id,
                targets,
                depth,
                stencil,
                rt0,
                rt1,
                rt2,
                rt3,
                rt4,
                rt5,
                rt6,
                rt7
            );
        }

        /// <summary>
        /// Sets an entry in the color palette.
        /// </summary>
        /// <param name="index">The index of the palette entry to set.</param>
        /// <param name="color">The color to set.</param>
        /// <remarks>
        /// The clear color palette is used with SetViewClear for clearing multiple render targets
        /// to different color values.
        /// </remarks>
        public static void SetPaletteColor (byte index, float* color) {
            NativeMethods.bgfx_set_palette_color(index, color);
        }

        /// <summary>
        /// Enables or disables sequential mode for a view. Sequential mode issues draw calls in the order they are received.
        /// </summary>
        /// <param name="id">The index of the view.</param>
        /// <param name="enabled"><c>true</c> to enable sequential mode; otherwise, <c>false</c>.</param>
        public static void SetViewSequential (byte id, bool enabled) {
            NativeMethods.bgfx_set_view_seq(id, enabled);
        }

        /// <summary>
        /// Sets the view and projection transforms for the given rendering view.
        /// </summary>
        /// <param name="id">The index of the view.</param>
        /// <param name="view">The 4x4 view transform matrix.</param>
        /// <param name="projection">The 4x4 projection transform matrix.</param>
        public static void SetViewTransform (byte id, float* view, float* projection) {
            NativeMethods.bgfx_set_view_transform(id, view, projection);
        }

        /// <summary>
        /// Sets the frame buffer used by a particular view.
        /// </summary>
        /// <param name="id">The index of the view.</param>
        /// <param name="frameBuffer">The frame buffer to set.</param>
        public static void SetViewFrameBuffer (byte id, FrameBuffer frameBuffer) {
            NativeMethods.bgfx_set_view_frame_buffer(id, frameBuffer.handle);
        }

        /// <summary>
        /// Sets the model transform to use for drawing primitives.
        /// </summary>
        /// <param name="matrix">A pointer to one or more matrices to set.</param>
        /// <param name="count">The number of matrices in the array.</param>
        /// <returns>An index into the matrix cache to allow reusing the matrix in other calls.</returns>
        public static int SetTransform (float* matrix, int count = 1) {
            return NativeMethods.bgfx_set_transform(matrix, (ushort)count);
        }

        /// <summary>
        /// Sets a model transform from the cache.
        /// </summary>
        /// <param name="cacheIndex">The index of the cached matrix.</param>
        /// <param name="count">The number of matrices to set from the cache.</param>
        public static void SetTransform (int cacheIndex, int count = 1) {
            NativeMethods.bgfx_set_transform_cached(cacheIndex, (ushort)count);
        }

        /// <summary>
        /// Sets the scissor rectangle to use for clipping primitives.
        /// </summary>
        /// <param name="x">The X coordinate of the scissor rectangle.</param>
        /// <param name="y">The Y coordinate of the scissor rectangle.</param>
        /// <param name="width">The width of the rectangle.</param>
        /// <param name="height">The height of the rectangle.</param>
        /// <returns>
        /// An index into the scissor cache to allow reusing the rectangle in other calls.
        /// </returns>
        public static int SetScissor (int x, int y, int width, int height) {
            return NativeMethods.bgfx_set_scissor((ushort)x, (ushort)y, (ushort)width, (ushort)height);
        }

        /// <summary>
        /// Sets a scissor rectangle from the cache.
        /// </summary>
        /// <param name="cacheIndex">The index of the cached scissor rectangle, or -1 to unset.</param>
        public static void SetScissor (int cacheIndex = -1) {
            NativeMethods.bgfx_set_scissor_cached((ushort)cacheIndex);
        }

        /// <summary>
        /// Sets the index buffer to use for drawing primitives.
        /// </summary>
        /// <param name="indexBuffer">The index buffer to set.</param>
        /// <param name="firstIndex">The first index in the buffer to use.</param>
        /// <param name="count">The number of indices to pull from the buffer.</param>
        public static void SetIndexBuffer (IndexBuffer indexBuffer, int firstIndex = 0, int count = -1) {
            NativeMethods.bgfx_set_index_buffer(indexBuffer.handle, firstIndex, count);
        }

        /// <summary>
        /// Sets the vertex buffer to use for drawing primitives.
        /// </summary>
        /// <param name="vertexBuffer">The vertex buffer to set.</param>
        /// <param name="firstVertex">The index of the first vertex to use.</param>
        /// <param name="count">The number of vertices to pull from the buffer.</param>
        public static void SetVertexBuffer (VertexBuffer vertexBuffer, int firstVertex = 0, int count = -1) {
            NativeMethods.bgfx_set_vertex_buffer(vertexBuffer.handle, firstVertex, count);
        }

        /// <summary>
        /// Sets the index buffer to use for drawing primitives.
        /// </summary>
        /// <param name="indexBuffer">The index buffer to set.</param>
        /// <param name="firstIndex">The first index in the buffer to use.</param>
        /// <param name="count">The number of indices to pull from the buffer.</param>
        public static void SetIndexBuffer (DynamicIndexBuffer indexBuffer, int firstIndex = 0, int count = -1) {
            NativeMethods.bgfx_set_dynamic_index_buffer(indexBuffer.handle, firstIndex, count);
        }

        /// <summary>
        /// Sets the vertex buffer to use for drawing primitives.
        /// </summary>
        /// <param name="vertexBuffer">The vertex buffer to set.</param>
        /// <param name="count">The number of vertices to pull from the buffer.</param>
        public static void SetVertexBuffer (DynamicVertexBuffer vertexBuffer, int count = -1) {
            NativeMethods.bgfx_set_dynamic_vertex_buffer(vertexBuffer.handle, count);
        }

        /// <summary>
        /// Sets the index buffer to use for drawing primitives.
        /// </summary>
        /// <param name="indexBuffer">The index buffer to set.</param>
        /// <param name="firstIndex">The first index in the buffer to use.</param>
        /// <param name="count">The number of indices to pull from the buffer.</param>
        public static void SetIndexBuffer (TransientIndexBuffer indexBuffer, int firstIndex = 0, int count = -1) {
            NativeMethods.bgfx_set_transient_index_buffer(ref indexBuffer, firstIndex, count);
        }

        /// <summary>
        /// Sets the vertex buffer to use for drawing primitives.
        /// </summary>
        /// <param name="vertexBuffer">The vertex buffer to set.</param>
        /// <param name="firstVertex">The index of the first vertex to use.</param>
        /// <param name="count">The number of vertices to pull from the buffer.</param>
        public static void SetVertexBuffer (TransientVertexBuffer vertexBuffer, int firstVertex = 0, int count = -1) {
            NativeMethods.bgfx_set_transient_vertex_buffer(ref vertexBuffer, firstVertex, count);
        }

        /// <summary>
        /// Sets instance data to use for drawing primitives.
        /// </summary>
        /// <param name="instanceData">The instance data.</param>
        /// <param name="count">The number of entries to pull from the buffer.</param>
        public static void SetInstanceDataBuffer (InstanceDataBuffer instanceData, int count = -1) {
            NativeMethods.bgfx_set_instance_data_buffer(instanceData.ptr, (ushort)count);
        }

        /// <summary>
        /// Sets instance data to use for drawing primitives.
        /// </summary>
        /// <param name="vertexBuffer">The vertex buffer containing instance data.</param>
        /// <param name="firstVertex">The index of the first vertex to use.</param>
        /// <param name="count">The number of vertices to pull from the buffer.</param>
        public static void SetInstanceDataBuffer (VertexBuffer vertexBuffer, int firstVertex, int count) {
            NativeMethods.bgfx_set_instance_data_from_vertex_buffer(vertexBuffer.handle, firstVertex, count);
        }

        /// <summary>
        /// Sets instance data to use for drawing primitives.
        /// </summary>
        /// <param name="vertexBuffer">The vertex buffer containing instance data.</param>
        /// <param name="firstVertex">The index of the first vertex to use.</param>
        /// <param name="count">The number of vertices to pull from the buffer.</param>
        public static void SetInstanceDataBuffer (DynamicVertexBuffer vertexBuffer, int firstVertex, int count) {
            NativeMethods.bgfx_set_instance_data_from_dynamic_vertex_buffer(vertexBuffer.handle, firstVertex, count);
        }

        /// <summary>
        /// Sets the value of a uniform parameter.
        /// </summary>
        /// <param name="uniform">The uniform to set.</param>
        /// <param name="value">A pointer to the uniform's data.</param>
        /// <param name="arraySize">The size of the data array, if the uniform is an array.</param>
        public static void SetUniform (Uniform uniform, float value, int arraySize = 1) {
            NativeMethods.bgfx_set_uniform(uniform.handle, &value, (ushort)arraySize);
        }

        /// <summary>
        /// Sets the value of a uniform parameter.
        /// </summary>
        /// <param name="uniform">The uniform to set.</param>
        /// <param name="value">A pointer to the uniform's data.</param>
        /// <param name="arraySize">The size of the data array, if the uniform is an array.</param>
        public static void SetUniform (Uniform uniform, void* value, int arraySize = 1) {
            NativeMethods.bgfx_set_uniform(uniform.handle, value, (ushort)arraySize);
        }

        /// <summary>
        /// Sets the value of a uniform parameter.
        /// </summary>
        /// <param name="uniform">The uniform to set.</param>
        /// <param name="value">A pointer to the uniform's data.</param>
        /// <param name="arraySize">The size of the data array, if the uniform is an array.</param>
        public static void SetUniform (Uniform uniform, IntPtr value, int arraySize = 1) {
            NativeMethods.bgfx_set_uniform(uniform.handle, value.ToPointer(), (ushort)arraySize);
        }

        /// <summary>
        /// Sets a texture to use for drawing primitives.
        /// </summary>
        /// <param name="textureUnit">The texture unit to set.</param>
        /// <param name="sampler">The sampler uniform.</param>
        /// <param name="texture">The texture to set.</param>
        public static void SetTexture (byte textureUnit, Uniform sampler, Texture texture) {
            NativeMethods.bgfx_set_texture(textureUnit, sampler.handle, texture.handle, uint.MaxValue);
        }

        /// <summary>
        /// Sets a texture to use for drawing primitives.
        /// </summary>
        /// <param name="textureUnit">The texture unit to set.</param>
        /// <param name="sampler">The sampler uniform.</param>
        /// <param name="texture">The texture to set.</param>
        /// <param name="flags">Sampling flags that override the default flags in the texture itself.</param>
        public static void SetTexture (byte textureUnit, Uniform sampler, Texture texture, TextureFlags flags) {
            NativeMethods.bgfx_set_texture(textureUnit, sampler.handle, texture.handle, (uint)flags);
        }

        /// <summary>
        /// Sets a texture to use for drawing primitives.
        /// </summary>
        /// <param name="textureUnit">The texture unit to set.</param>
        /// <param name="sampler">The sampler uniform.</param>
        /// <param name="frameBuffer">The frame buffer.</param>
        /// <param name="attachment">The index of the frame buffer attachment to set as a texture.</param>
        public static void SetTexture (byte textureUnit, Uniform sampler, FrameBuffer frameBuffer, byte attachment = 0) {
            NativeMethods.bgfx_set_texture_from_frame_buffer(textureUnit, sampler.handle, frameBuffer.handle, attachment, uint.MaxValue);
        }

        /// <summary>
        /// Sets a texture to use for drawing primitives.
        /// </summary>
        /// <param name="textureUnit">The texture unit to set.</param>
        /// <param name="sampler">The sampler uniform.</param>
        /// <param name="frameBuffer">The frame buffer.</param>
        /// <param name="attachment">The index of the attachment to set.</param>
        /// <param name="flags">Sampling flags that override the default flags in the texture itself.</param>
        public static void SetTexture (byte textureUnit, Uniform sampler, FrameBuffer frameBuffer, byte attachment, TextureFlags flags) {
            NativeMethods.bgfx_set_texture_from_frame_buffer(textureUnit, sampler.handle, frameBuffer.handle, attachment, (uint)flags);
        }

        /// <summary>
        /// Sets a texture mip as a compute image.
        /// </summary>
        /// <param name="stage">The buffer stage to set.</param>
        /// <param name="sampler">The sampler uniform.</param>
        /// <param name="texture">The texture to set.</param>
        /// <param name="mip">The index of the mip level within the texture to set.</param>
        /// <param name="format">The format of the buffer data.</param>
        /// <param name="access">Access control flags.</param>
        public static void SetComputeImage (byte stage, Uniform sampler, Texture texture, byte mip, ComputeBufferAccess access, TextureFormat format = TextureFormat.Unknown) {
            NativeMethods.bgfx_set_image(stage, sampler.handle, texture.handle, mip, format, access);
        }

        /// <summary>
        /// Sets a frame buffer attachment as a compute image.
        /// </summary>
        /// <param name="stage">The buffer stage to set.</param>
        /// <param name="sampler">The sampler uniform.</param>
        /// <param name="frameBuffer">The frame buffer.</param>
        /// <param name="attachment">The attachment index.</param>
        /// <param name="format">The format of the buffer data.</param>
        /// <param name="access">Access control flags.</param>
        public static void SetComputeImage (byte stage, Uniform sampler, FrameBuffer frameBuffer, byte attachment, ComputeBufferAccess access, TextureFormat format = TextureFormat.Unknown) {
            NativeMethods.bgfx_set_image_from_frame_buffer(stage, sampler.handle, frameBuffer.handle, attachment, format, access);
        }

        /// <summary>
        /// Sets an index buffer as a compute resource.
        /// </summary>
        /// <param name="stage">The resource stage to set.</param>
        /// <param name="buffer">The buffer to set.</param>
        /// <param name="access">Access control flags.</param>
        public static void SetComputeBuffer (byte stage, IndexBuffer buffer, ComputeBufferAccess access) {
            NativeMethods.bgfx_set_compute_index_buffer(stage, buffer.handle, access);
        }

        /// <summary>
        /// Sets a verterx buffer as a compute resource.
        /// </summary>
        /// <param name="stage">The resource stage to set.</param>
        /// <param name="buffer">The buffer to set.</param>
        /// <param name="access">Access control flags.</param>
        public static void SetComputeBuffer (byte stage, VertexBuffer buffer, ComputeBufferAccess access) {
            NativeMethods.bgfx_set_compute_vertex_buffer(stage, buffer.handle, access);
        }

        /// <summary>
        /// Sets a dynamic index buffer as a compute resource.
        /// </summary>
        /// <param name="stage">The resource stage to set.</param>
        /// <param name="buffer">The buffer to set.</param>
        /// <param name="access">Access control flags.</param>
        public static void SetComputeBuffer (byte stage, DynamicIndexBuffer buffer, ComputeBufferAccess access) {
            NativeMethods.bgfx_set_compute_dynamic_index_buffer(stage, buffer.handle, access);
        }

        /// <summary>
        /// Sets a dynamic vertex buffer as a compute resource.
        /// </summary>
        /// <param name="stage">The resource stage to set.</param>
        /// <param name="buffer">The buffer to set.</param>
        /// <param name="access">Access control flags.</param>
        public static void SetComputeBuffer (byte stage, DynamicVertexBuffer buffer, ComputeBufferAccess access) {
            NativeMethods.bgfx_set_compute_dynamic_vertex_buffer(stage, buffer.handle, access);
        }

        /// <summary>
        /// Sets an indirect buffer as a compute resource.
        /// </summary>
        /// <param name="stage">The resource stage to set.</param>
        /// <param name="buffer">The buffer to set.</param>
        /// <param name="access">Access control flags.</param>
        public static void SetComputeBuffer (byte stage, IndirectBuffer buffer, ComputeBufferAccess access) {
            NativeMethods.bgfx_set_compute_indirect_buffer(stage, buffer.handle, access);
        }

        /// <summary>
        /// Marks a view as "touched", ensuring that its background is cleared even if nothing is rendered.
        /// </summary>
        /// <param name="id">The index of the view to touch.</param>
        /// <returns>The number of draw calls.</returns>
        public static int Touch (byte id) {
            return NativeMethods.bgfx_touch(id);
        }

        /// <summary>
        /// Resets all view settings to default.
        /// </summary>
        /// <param name="id">The index of the view to reset.</param>
        public static void ResetView (byte id) {
            NativeMethods.bgfx_reset_view(id);
        }

        /// <summary>
        /// Submits the current batch of primitives for rendering.
        /// </summary>
        /// <param name="id">The index of the view to submit.</param>
        /// <param name="program">The program with which to render.</param>
        /// <param name="depth">A depth value to use for sorting the batch.</param>
        /// <returns>The number of draw calls.</returns>
        public static int Submit (byte id, Program program, int depth = 0) {
            return NativeMethods.bgfx_submit(id, program.handle, depth);
        }

        /// <summary>
        /// Submits the current batch of primitives for rendering.
        /// </summary>
        /// <param name="id">The index of the view to submit.</param>
        /// <param name="program">The program with which to render.</param>
        /// <param name="query">An occlusion query to use as a predicate during rendering.</param>
        /// <param name="depth">A depth value to use for sorting the batch.</param>
        /// <returns>The number of draw calls.</returns>
        public static int Submit (byte id, Program program, OcclusionQuery query, int depth = 0) {
            return NativeMethods.bgfx_submit_occlusion_query(id, program.handle, query.handle, depth);
        }

        /// <summary>
        /// Submits an indirect batch of drawing commands to be used for rendering.
        /// </summary>
        /// <param name="id">The index of the view to submit.</param>
        /// <param name="program">The program with which to render.</param>
        /// <param name="indirectBuffer">The buffer containing drawing commands.</param>
        /// <param name="startIndex">The index of the first command to process.</param>
        /// <param name="count">The number of commands to process from the buffer.</param>
        /// <param name="depth">A depth value to use for sorting the batch.</param>
        /// <returns>The number of draw calls.</returns>
        public static int Submit (byte id, Program program, IndirectBuffer indirectBuffer, int startIndex = 0, int count = 1, int depth = 0) {
            return NativeMethods.bgfx_submit_indirect(id, program.handle, indirectBuffer.handle, (ushort)startIndex, (ushort)count, depth);
        }

        /// <summary>
        /// Discards all previously set state for the draw call.
        /// </summary>
        public static void Discard () {
            NativeMethods.bgfx_discard();
        }

        /// <summary>
        /// Dispatches a compute job.
        /// </summary>
        /// <param name="id">The index of the view to dispatch.</param>
        /// <param name="program">The shader program to use.</param>
        /// <param name="xCount">The size of the job in the first dimension.</param>
        /// <param name="yCount">The size of the job in the second dimension.</param>
        /// <param name="zCount">The size of the job in the third dimension.</param>
        public static void Dispatch (byte id, Program program, int xCount = 1, int yCount = 1, int zCount = 1) {
            // TODO: unused
            byte unused = 0;
            NativeMethods.bgfx_dispatch(id, program.handle, (ushort)xCount, (ushort)yCount, (ushort)zCount, unused);
        }

        /// <summary>
        /// Dispatches an indirect compute job.
        /// </summary>
        /// <param name="id">The index of the view to dispatch.</param>
        /// <param name="program">The shader program to use.</param>
        /// <param name="indirectBuffer">The buffer containing drawing commands.</param>
        /// <param name="startIndex">The index of the first command to process.</param>
        /// <param name="count">The number of commands to process from the buffer.</param>
        public static void Dispatch (byte id, Program program, IndirectBuffer indirectBuffer, int startIndex = 0, int count = 1) {
            // TODO: unused
            byte unused = 0;
            NativeMethods.bgfx_dispatch_indirect(id, program.handle, indirectBuffer.handle, (ushort)startIndex, (ushort)count, unused);
        }

        /// <summary>
        /// Requests that a screenshot be saved. The ScreenshotTaken event will be fired to save the result.
        /// </summary>
        /// <param name="filePath">The file path that will be passed to the callback event.</param>
        public static void SaveScreenShot (string filePath) {
            NativeMethods.bgfx_save_screen_shot(filePath);
        }

        /// <summary>
        /// Set rendering states used to draw primitives.
        /// </summary>
        /// <param name="state">The set of states to set.</param>
        public static void SetRenderState (RenderState state) {
            NativeMethods.bgfx_set_state((ulong)state, 0);
        }

        /// <summary>
        /// Set rendering states used to draw primitives.
        /// </summary>
        /// <param name="state">The set of states to set.</param>
        /// <param name="colorRgba">The color used for "factor" blending modes.</param>
        public static void SetRenderState (RenderState state, int colorRgba) {
            NativeMethods.bgfx_set_state((ulong)state, colorRgba);
        }

        /// <summary>
        /// Sets stencil test state.
        /// </summary>
        /// <param name="frontFace">The stencil state to use for front faces.</param>
        public static void SetStencil (StencilFlags frontFace) {
            SetStencil(frontFace, StencilFlags.None);
        }

        /// <summary>
        /// Sets stencil test state.
        /// </summary>
        /// <param name="frontFace">The stencil state to use for front faces.</param>
        /// <param name="backFace">The stencil state to use for back faces.</param>
        public static void SetStencil (StencilFlags frontFace, StencilFlags backFace) {
            NativeMethods.bgfx_set_stencil((uint)frontFace, (uint)backFace);
        }
    }
}
