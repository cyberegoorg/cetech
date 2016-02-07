// Copyright (c) 2015 Michael Popoloski
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

using System;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security;

namespace SharpBgfx {
    /// <summary>
    /// Provides an interface for programs to respond to callbacks from the bgfx library.
    /// </summary>
    public interface ICallbackHandler {
        /// <summary>
        /// Called when an error occurs in the library.
        /// </summary>
        /// <param name="errorType">The type of error that occurred.</param>
        /// <param name="message">Message string detailing what went wrong.</param>
        /// <remarks>
        /// If the error type is not <see cref="ErrorType.DebugCheck"/>, bgfx is in an
        /// unrecoverable state and the application should terminate.
        /// 
        /// This method can be called from any thread.
        /// </remarks>
        void ReportError (ErrorType errorType, string message);

        /// <summary>
        /// Called to print debug messages.
        /// </summary>
        /// <param name="fileName">The name of the source file in which the message originated.</param>
        /// <param name="line">The line number in which the message originated.</param>
        /// <param name="format">The message format string.</param>
        /// <param name="args">A pointer to format arguments.</param>
        /// <remarks>This method can be called from any thread.</remarks>
        void ReportDebug (string fileName, int line, string format, IntPtr args);

        /// <summary>
        /// Queries the size of a cache item.
        /// </summary>
        /// <param name="id">The cache entry ID.</param>
        /// <returns>The size of the cache item, or 0 if the item is not found.</returns>
        int GetCachedSize (long id);

        /// <summary>
        /// Retrieves an entry from the cache.
        /// </summary>
        /// <param name="id">The cache entry ID.</param>
        /// <param name="data">A pointer that should be filled with data from the cache.</param>
        /// <param name="size">The size of the memory block pointed to be <paramref name="data"/>.</param>
        /// <returns><c>true</c> if the item is found in the cache; otherwise, <c>false</c>.</returns>
        bool GetCacheEntry (long id, IntPtr data, int size);

        /// <summary>
        /// Saves an entry in the cache.
        /// </summary>
        /// <param name="id">The cache entry ID.</param>
        /// <param name="data">A pointer to the data to save in the cache.</param>
        /// <param name="size">The size of the memory block pointed to be <paramref name="data"/>.</param>
        void SetCacheEntry (long id, IntPtr data, int size);

        /// <summary>
        /// Save a captured screenshot.
        /// </summary>
        /// <param name="path">The path at which to save the image.</param>
        /// <param name="width">The width of the image.</param>
        /// <param name="height">The height of the image.</param>
        /// <param name="pitch">The number of bytes between lines in the image.</param>
        /// <param name="data">A pointer to the image data to save.</param>
        /// <param name="size">The size of the image memory.</param>
        /// <param name="flipVertical"><c>true</c> if the image origin is bottom left instead of top left; otherwise, <c>false</c>.</param>
        void SaveScreenShot (string path, int width, int height, int pitch, IntPtr data, int size, bool flipVertical);

        /// <summary>
        /// Notifies that a frame capture has begun.
        /// </summary>
        /// <param name="width">The width of the capture surface.</param>
        /// <param name="height">The height of the capture surface.</param>
        /// <param name="pitch">The number of bytes between lines in the captured frames.</param>
        /// <param name="format">The format of captured frames.</param>
        /// <param name="flipVertical"><c>true</c> if the image origin is bottom left instead of top left; otherwise, <c>false</c>.</param>
        void CaptureStarted (int width, int height, int pitch, TextureFormat format, bool flipVertical);

        /// <summary>
        /// Notifies that a frame capture has finished.
        /// </summary>
        void CaptureFinished ();

        /// <summary>
        /// Notifies that a frame has been captured.
        /// </summary>
        /// <param name="data">A pointer to the frame data.</param>
        /// <param name="size">The size of the frame data.</param>
        void CaptureFrame (IntPtr data, int size);
    }

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

    /// <summary>
    /// Represents a loaded texture.
    /// </summary>
    public unsafe sealed class Texture : IDisposable, IEquatable<Texture> {
        internal readonly ushort handle;

        /// <summary>
        /// The width of the texture.
        /// </summary>
        public int Width { get; private set; }

        /// <summary>
        /// The height of the texture.
        /// </summary>
        public int Height { get; private set; }

        /// <summary>
        /// The depth of the texture, if 3D.
        /// </summary>
        public int Depth { get; private set; }

        /// <summary>
        /// Indicates whether the texture is a cubemap.
        /// </summary>
        public bool IsCubeMap { get; private set; }

        /// <summary>
        /// The number of mip levels in the texture.
        /// </summary>
        public int MipLevels { get; private set; }

        /// <summary>
        /// The number of bits per pixel.
        /// </summary>
        public int BitsPerPixel { get; private set; }

        /// <summary>
        /// The size of the entire texture, in bytes.
        /// </summary>
        public int SizeInBytes { get; private set; }

        /// <summary>
        /// The format of the image data.
        /// </summary>
        public TextureFormat Format { get; private set; }

        Texture (ushort handle, ref TextureInfo info) {
            this.handle = handle;

            Width = info.Width;
            Height = info.Height;
            Depth = info.Depth;
            MipLevels = info.MipCount;
            BitsPerPixel = info.BitsPerPixel;
            SizeInBytes = info.StorageSize;
            Format = info.Format;
            IsCubeMap = info.IsCubeMap;
        }

        /// <summary>
        /// Creates a new texture from a file loaded in memory.
        /// </summary>
        /// <param name="memory">The content of the file.</param>
        /// <param name="flags">Flags that control texture behavior.</param>
        /// <param name="skipMips">A number of top level mips to skip when parsing texture data.</param>
        /// <returns>The newly created texture.</returns>
        /// <remarks>
        /// This function supports textures in the following container formats:
        /// - DDS
        /// - KTX
        /// - PVR
        /// </remarks>
        public static Texture FromFile (MemoryBlock memory, TextureFlags flags = TextureFlags.None, int skipMips = 0) {
            TextureInfo info;
            var handle = NativeMethods.bgfx_create_texture(memory.ptr, flags, (byte)skipMips, out info);

            return new Texture(handle, ref info);
        }

        /// <summary>
        /// Creates a new 2D texture.
        /// </summary>
        /// <param name="width">The width of the texture.</param>
        /// <param name="height">The height of the texture.</param>
        /// <param name="mipCount">The number of mip levels.</param>
        /// <param name="format">The format of the texture data.</param>
        /// <param name="flags">Flags that control texture behavior.</param>
        /// <param name="memory">If not <c>null</c>, contains the texture's image data.</param>
        /// <returns>
        /// The newly created texture handle.
        /// </returns>
        public static Texture Create2D (int width, int height, int mipCount, TextureFormat format, TextureFlags flags = TextureFlags.None, MemoryBlock? memory = null) {
            var info = new TextureInfo();
            NativeMethods.bgfx_calc_texture_size(ref info, (ushort)width, (ushort)height, 1, false, (byte)mipCount, format);

            var handle = NativeMethods.bgfx_create_texture_2d(info.Width, info.Height, info.MipCount, format, flags, memory == null ? null : memory.Value.ptr);
            return new Texture(handle, ref info);
        }

        /// <summary>
        /// Creates a new 2D texture that scales with backbuffer size.
        /// </summary>
        /// <param name="ratio">The amount to scale when the backbuffer resizes.</param>
        /// <param name="mipCount">The number of mip levels.</param>
        /// <param name="format">The format of the texture data.</param>
        /// <param name="flags">Flags that control texture behavior.</param>
        /// <returns>
        /// The newly created texture handle.
        /// </returns>
        public static Texture Create2D (BackbufferRatio ratio, int mipCount, TextureFormat format, TextureFlags flags = TextureFlags.None) {
            var info = new TextureInfo {
                Format = format,
                MipCount = (byte)mipCount
            };

            var handle = NativeMethods.bgfx_create_texture_2d_scaled(ratio, info.MipCount, format, flags);
            return new Texture(handle, ref info);
        }

        /// <summary>
        /// Creates a new 3D texture.
        /// </summary>
        /// <param name="width">The width of the texture.</param>
        /// <param name="height">The height of the texture.</param>
        /// <param name="depth">The depth of the texture.</param>
        /// <param name="mipCount">The number of mip levels.</param>
        /// <param name="format">The format of the texture data.</param>
        /// <param name="flags">Flags that control texture behavior.</param>
        /// <param name="memory">If not <c>null</c>, contains the texture's image data.</param>
        /// <returns>The newly created texture handle.</returns>
        public static Texture Create3D (int width, int height, int depth, int mipCount, TextureFormat format, TextureFlags flags = TextureFlags.None, MemoryBlock? memory = null) {
            var info = new TextureInfo();
            NativeMethods.bgfx_calc_texture_size(ref info, (ushort)width, (ushort)height, (ushort)depth, false, (byte)mipCount, format);

            var handle = NativeMethods.bgfx_create_texture_3d(info.Width, info.Height, info.Depth, info.MipCount, format, flags, memory == null ? null : memory.Value.ptr);
            return new Texture(handle, ref info);
        }

        /// <summary>
        /// Creates a new cube texture.
        /// </summary>
        /// <param name="size">The size of each cube face.</param>
        /// <param name="mipCount">The number of mip levels.</param>
        /// <param name="format">The format of the texture data.</param>
        /// <param name="flags">Flags that control texture behavior.</param>
        /// <param name="memory">If not <c>null</c>, contains the texture's image data.</param>
        /// <returns>
        /// The newly created texture handle.
        /// </returns>
        public static Texture CreateCube (int size, int mipCount, TextureFormat format, TextureFlags flags = TextureFlags.None, MemoryBlock? memory = null) {
            var info = new TextureInfo();
            NativeMethods.bgfx_calc_texture_size(ref info, (ushort)size, (ushort)size, 1, true, (byte)mipCount, format);

            var handle = NativeMethods.bgfx_create_texture_cube(info.Width, info.MipCount, format, flags, memory == null ? null : memory.Value.ptr);
            return new Texture(handle, ref info);
        }

        /// <summary>
        /// Releases the texture.
        /// </summary>
        public void Dispose () {
            NativeMethods.bgfx_destroy_texture(handle);
        }

        /// <summary>
        /// Updates the data in a 2D texture.
        /// </summary>
        /// <param name="mipLevel">The mip level.</param>
        /// <param name="x">The X coordinate of the rectangle to update.</param>
        /// <param name="y">The Y coordinate of the rectangle to update.</param>
        /// <param name="width">The width of the rectangle to update.</param>
        /// <param name="height">The height of the rectangle to update.</param>
        /// <param name="memory">The new image data.</param>
        /// <param name="pitch">The pitch of the image data.</param>
        public void Update2D (int mipLevel, int x, int y, int width, int height, MemoryBlock memory, int pitch) {
            NativeMethods.bgfx_update_texture_2d(handle, (byte)mipLevel, (ushort)x, (ushort)y, (ushort)width, (ushort)height, memory.ptr, (ushort)pitch);
        }

        /// <summary>
        /// Updates the data in a 3D texture.
        /// </summary>
        /// <param name="mipLevel">The mip level.</param>
        /// <param name="x">The X coordinate of the volume to update.</param>
        /// <param name="y">The Y coordinate of the volume to update.</param>
        /// <param name="z">The Z coordinate of the volume to update.</param>
        /// <param name="width">The width of the volume to update.</param>
        /// <param name="height">The height of the volume to update.</param>
        /// <param name="depth">The depth of the volume to update.</param>
        /// <param name="memory">The new image data.</param>
        public void Update3D (int mipLevel, int x, int y, int z, int width, int height, int depth, MemoryBlock memory) {
            NativeMethods.bgfx_update_texture_3d(handle, (byte)mipLevel, (ushort)x, (ushort)y, (ushort)z, (ushort)width, (ushort)height, (ushort)depth, memory.ptr);
        }

        /// <summary>
        /// Updates the data in a cube texture.
        /// </summary>
        /// <param name="face">The cube map face to update.</param>
        /// <param name="mipLevel">The mip level.</param>
        /// <param name="x">The X coordinate of the rectangle to update.</param>
        /// <param name="y">The Y coordinate of the rectangle to update.</param>
        /// <param name="width">The width of the rectangle to update.</param>
        /// <param name="height">The height of the rectangle to update.</param>
        /// <param name="memory">The new image data.</param>
        /// <param name="pitch">The pitch of the image data.</param>
        public void UpdateCube (CubeMapFace face, int mipLevel, int x, int y, int width, int height, MemoryBlock memory, int pitch) {
            NativeMethods.bgfx_update_texture_cube(handle, face, (byte)mipLevel, (ushort)x, (ushort)y, (ushort)width, (ushort)height, memory.ptr, (ushort)pitch);
        }

        /// <summary>
        /// Blits the contents of the texture to another texture.
        /// </summary>
        /// <param name="viewId">The view in which the blit will be ordered.</param>
        /// <param name="dest">The destination texture.</param>
        /// <param name="destX">The destination X position.</param>
        /// <param name="destY">The destination Y position.</param>
        /// <param name="sourceX">The source X position.</param>
        /// <param name="sourceY">The source Y position.</param>
        /// <param name="width">The width of the region to blit.</param>
        /// <param name="height">The height of the region to blit.</param>
        /// <remarks>The destination texture must be created with the <see cref="TextureFlags.BlitDestination"/> flag.</remarks>
        public void BlitTo (byte viewId, Texture dest, int destX, int destY, int sourceX = 0, int sourceY = 0,
                            int width = ushort.MaxValue, int height = ushort.MaxValue) {
            BlitTo(viewId, dest, 0, destX, destY, 0, 0, sourceX, sourceY, 0, width, height, 0);
        }

        /// <summary>
        /// Blits the contents of the texture to another texture.
        /// </summary>
        /// <param name="viewId">The view in which the blit will be ordered.</param>
        /// <param name="dest">The destination texture.</param>
        /// <param name="destMip">The destination mip level.</param>
        /// <param name="destX">The destination X position.</param>
        /// <param name="destY">The destination Y position.</param>
        /// <param name="destZ">The destination Z position.</param>
        /// <param name="sourceMip">The source mip level.</param>
        /// <param name="sourceX">The source X position.</param>
        /// <param name="sourceY">The source Y position.</param>
        /// <param name="sourceZ">The source Z position.</param>
        /// <param name="width">The width of the region to blit.</param>
        /// <param name="height">The height of the region to blit.</param>
        /// <param name="depth">The depth of the region to blit.</param>
        /// <remarks>The destination texture must be created with the <see cref="TextureFlags.BlitDestination"/> flag.</remarks>
        public void BlitTo (byte viewId, Texture dest, int destMip, int destX, int destY, int destZ,
                            int sourceMip = 0, int sourceX = 0, int sourceY = 0, int sourceZ = 0,
                            int width = ushort.MaxValue, int height = ushort.MaxValue, int depth = ushort.MaxValue) {
            NativeMethods.bgfx_blit(viewId, dest.handle, (byte)destMip, (ushort)destX, (ushort)destY, (ushort)destZ,
                handle, (byte)sourceMip, (ushort)sourceX, (ushort)sourceY, (ushort)sourceZ, (ushort)width, (ushort)height, (ushort)depth);
        }

        /// <summary>
        /// Reads the contents of the texture and stores them in memory pointed to by <paramref name="data"/>.
        /// </summary>
        /// <param name="data">The destination for the read image data.</param>
        /// <remarks>The texture must have been created with the <see cref="TextureFlags.ReadBack"/> flag.</remarks>
        public void Read (IntPtr data) {
            NativeMethods.bgfx_read_texture(handle, data);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (Texture other) {
            if (ReferenceEquals(other, null))
                return false;

            if (ReferenceEquals(other, this))
                return true;

            return handle == other.handle;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            return Equals(obj as Texture);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return handle.GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Handle: {0}", handle);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator == (Texture left, Texture right) {
            if (ReferenceEquals(left, null))
                return ReferenceEquals(right, null);

            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator != (Texture left, Texture right) {
            return !(left == right);
        }

        internal struct TextureInfo {
            public TextureFormat Format;
            public int StorageSize;
            public ushort Width;
            public ushort Height;
            public ushort Depth;
            public byte MipCount;
            public byte BitsPerPixel;
            public bool IsCubeMap;
        }
    }

    /// <summary>
    /// Describes the layout of data in a vertex stream.
    /// </summary>
    public sealed class VertexLayout {
        internal Data data;

        /// <summary>
        /// The stride of a single vertex using this layout.
        /// </summary>
        public int Stride {
            get { return data.Stride; }
        }

        /// <summary>
        /// Starts a stream of vertex attribute additions to the layout.
        /// </summary>
        /// <param name="backend">The rendering backend with which to associate the attributes.</param>
        /// <returns>This instance, for use in a fluent API.</returns>
        public VertexLayout Begin (RendererBackend backend = RendererBackend.Null) {
            NativeMethods.bgfx_vertex_decl_begin(ref data, backend);
            return this;
        }

        /// <summary>
        /// Starts a stream of vertex attribute additions to the layout.
        /// </summary>
        /// <param name="attribute">The kind of attribute to add.</param>
        /// <param name="count">The number of elements in the attribute (1, 2, 3, or 4).</param>
        /// <param name="type">The type of data described by the attribute.</param>
        /// <param name="normalized">if set to <c>true</c>, values will be normalized from a 0-255 range to 0.0 - 0.1 in the shader.</param>
        /// <param name="asInt">if set to <c>true</c>, the attribute is packaged as an integer in the shader.</param>
        /// <returns>
        /// This instance, for use in a fluent API.
        /// </returns>
        public VertexLayout Add (VertexAttributeUsage attribute, int count, VertexAttributeType type, bool normalized = false, bool asInt = false) {
            NativeMethods.bgfx_vertex_decl_add(ref data, attribute, (byte)count, type, normalized, asInt);
            return this;
        }

        /// <summary>
        /// Skips the specified number of bytes in the vertex stream.
        /// </summary>
        /// <param name="count">The number of bytes to skip.</param>
        /// <returns>This instance, for use in a fluent API.</returns>
        public VertexLayout Skip (int count) {
            NativeMethods.bgfx_vertex_decl_skip(ref data, (byte)count);
            return this;
        }

        /// <summary>
        /// Marks the end of the vertex stream.
        /// </summary>
        /// <returns>This instance, for use in a fluent API.</returns>
        public VertexLayout End () {
            NativeMethods.bgfx_vertex_decl_end(ref data);
            return this;
        }

        /// <summary>
        /// Gets the byte offset of a particular attribute in the layout.
        /// </summary>
        /// <param name="attribute">The attribute for which to get the offset.</param>
        /// <returns>The offset of the attribute, in bytes.</returns>
        public unsafe int GetOffset (VertexAttributeUsage attribute) {
            fixed (Data* ptr = &data)
                return ptr->Offset[(int)attribute];
        }

        /// <summary>
        /// Determines whether the layout contains the given attribute.
        /// </summary>
        /// <param name="attribute">The attribute to check/</param>
        /// <returns><c>true</c> if the layout contains the attribute; otherwise, <c>false</c>.</returns>
        public unsafe bool HasAttribute (VertexAttributeUsage attribute) {
            fixed (Data* ptr = &data)
                return ptr->Attributes[(int)attribute] != 0xff;
        }

        internal unsafe struct Data {
            const int MaxAttribCount = 16;

            public uint Hash;
            public ushort Stride;
            public fixed ushort Offset[MaxAttribCount];
            public fixed ushort Attributes[MaxAttribCount];
        }
    }

    /// <summary>
    /// Contains details about an installed graphics adapter.
    /// </summary>
    public struct Adapter {
        /// <summary>
        /// Represents the default adapter for the system.
        /// </summary>
        public static readonly Adapter Default = new Adapter(Vendor.None, 0);

        /// <summary>
        /// The IHV that published the adapter.
        /// </summary>
        public readonly Vendor Vendor;

        /// <summary>
        /// A vendor-specific identifier for the adapter type.
        /// </summary>
        public readonly int DeviceId;

        /// <summary>
        /// Initializes a new instance of the <see cref="Adapter"/> struct.
        /// </summary>
        /// <param name="vendor">The vendor.</param>
        /// <param name="deviceId">The device ID.</param>
        public Adapter (Vendor vendor, int deviceId) {
            Vendor = vendor;
            DeviceId = deviceId;
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Vendor: {0}, Device: {0}", Vendor, DeviceId);
        }
    }

    /// <summary>
    /// Contains information about the capabilities of the rendering device.
    /// </summary>
    public unsafe struct Capabilities {
        Caps* data;

        /// <summary>
        /// The currently active rendering backend API.
        /// </summary>
        public RendererBackend Backend {
            get { return data->Backend; }
        }

        /// <summary>
        /// A set of extended features supported by the device.
        /// </summary>
        public DeviceFeatures SupportedFeatures {
            get { return data->Supported; }
        }

        /// <summary>
        /// The maximum size of a texture, in pixels.
        /// </summary>
        public int MaxTextureSize {
            get { return data->MaxTextureSize; }
        }

        /// <summary>
        /// The maximum number of render views supported.
        /// </summary>
        public int MaxViews {
            get { return data->MaxViews; }
        }

        /// <summary>
        /// The maximum number of draw calls in a single frame.
        /// </summary>
        public int MaxDrawCalls {
            get { return data->MaxDrawCalls; }
        }

        /// <summary>
        /// The maximum number of attachments to a single framebuffer.
        /// </summary>
        public int MaxFramebufferAttachments {
            get { return data->MaxFramebufferAttachements; }
        }

        /// <summary>
        /// Details about the currently active graphics adapter.
        /// </summary>
        public Adapter CurrentAdapter {
            get { return new Adapter((Vendor)data->VendorId, data->DeviceId); }
        }

        /// <summary>
        /// A list of all graphics adapters installed on the system.
        /// </summary>
        public AdapterCollection Adapters {
            get { return new AdapterCollection(data->GPUs, data->GPUCount); }
        }

        internal Capabilities (Caps* data) {
            this.data = data;
        }

        /// <summary>
        /// Checks device support for a specific texture format.
        /// </summary>
        /// <param name="format">The format to check.</param>
        /// <returns>The level of support for the given format.</returns>
        public TextureFormatSupport CheckTextureSupport (TextureFormat format) {
            return (TextureFormatSupport)data->Formats[(int)format];
        }

        /// <summary>
        /// Provides access to a collection of adapters.
        /// </summary>
        public unsafe struct AdapterCollection : IReadOnlyList<Adapter> {
            ushort* data;
            int count;

            /// <summary>
            /// Accesses the element at the specified index.
            /// </summary>
            /// <param name="index">The index of the element to retrieve.</param>
            /// <returns>The element at the given index.</returns>
            public Adapter this[int index] {
                get { return new Adapter((Vendor)data[index * 2], data[index * 2 + 1]); }
            }

            /// <summary>
            /// The number of elements in the collection.
            /// </summary>
            public int Count {
                get { return count; }
            }

            internal AdapterCollection (ushort* data, int count) {
                this.data = data;
                this.count = count;
            }

            /// <summary>
            /// Gets an enumerator for the collection.
            /// </summary>
            /// <returns>A collection enumerator.</returns>
            public Enumerator GetEnumerator () {
                return new Enumerator(this);
            }

            IEnumerator<Adapter> IEnumerable<Adapter>.GetEnumerator () {
                return GetEnumerator();
            }

            IEnumerator IEnumerable.GetEnumerator () {
                return GetEnumerator();
            }

            /// <summary>
            /// Implements an enumerator for an AdapterCollection.
            /// </summary>
            public struct Enumerator : IEnumerator<Adapter> {
                AdapterCollection collection;
                int index;

                /// <summary>
                /// The current enumerated item.
                /// </summary>
                public Adapter Current {
                    get { return collection[index]; }
                }

                object IEnumerator.Current {
                    get { return Current; }
                }

                internal Enumerator (AdapterCollection collection) {
                    this.collection = collection;
                    index = -1;
                }

                /// <summary>
                /// Advances to the next item in the sequence.
                /// </summary>
                /// <returns><c>true</c> if there are more items in the collection; otherwise, <c>false</c>.</returns>
                public bool MoveNext () {
                    var newIndex = index + 1;
                    if (newIndex >= collection.Count)
                        return false;

                    index = newIndex;
                    return true;
                }

                /// <summary>
                /// Empty; does nothing.
                /// </summary>
                public void Dispose () {
                }

                /// <summary>
                /// Not implemented.
                /// </summary>
                public void Reset () {
                    throw new NotImplementedException();
                }
            }
        }

#pragma warning disable 649
        internal unsafe struct Caps {
            const int TextureFormatCount = 48;

            public RendererBackend Backend;
            public DeviceFeatures Supported;
            public ushort MaxTextureSize;
            public ushort MaxViews;
            public ushort MaxDrawCalls;
            public byte MaxFramebufferAttachements;
            public byte GPUCount;
            public ushort VendorId;
            public ushort DeviceId;

            public fixed ushort GPUs[8];
            public fixed byte Formats[TextureFormatCount];
        }
#pragma warning restore 649
    }

    /// <summary>
    /// Represents a dynamically updateable index buffer.
    /// </summary>
    public unsafe struct DynamicIndexBuffer : IDisposable, IEquatable<DynamicIndexBuffer> {
        internal readonly ushort handle;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly DynamicIndexBuffer Invalid = new DynamicIndexBuffer();

        /// <summary>
        /// Initializes a new instance of the <see cref="DynamicIndexBuffer"/> struct.
        /// </summary>
        /// <param name="indexCount">The number of indices that can fit in the buffer.</param>
        /// <param name="flags">Flags used to control buffer behavior.</param>
        public DynamicIndexBuffer (int indexCount, BufferFlags flags = BufferFlags.None) {
            handle = NativeMethods.bgfx_create_dynamic_index_buffer(indexCount, flags);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="DynamicIndexBuffer"/> struct.
        /// </summary>
        /// <param name="memory">The initial index data with which to populate the buffer.</param>
        /// <param name="flags">Flags used to control buffer behavior.</param>
        public DynamicIndexBuffer (MemoryBlock memory, BufferFlags flags = BufferFlags.None) {
            handle = NativeMethods.bgfx_create_dynamic_index_buffer_mem(memory.ptr, flags);
        }

        /// <summary>
        /// Updates the data in the buffer.
        /// </summary>
        /// <param name="startIndex">Index of the first index to update.</param>
        /// <param name="memory">The new index data with which to fill the buffer.</param>
        public void Update (int startIndex, MemoryBlock memory) {
            NativeMethods.bgfx_update_dynamic_index_buffer(handle, startIndex, memory.ptr);
        }

        /// <summary>
        /// Releases the index buffer.
        /// </summary>
        public void Dispose () {
            NativeMethods.bgfx_destroy_dynamic_index_buffer(handle);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (DynamicIndexBuffer other) {
            return handle == other.handle;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as DynamicIndexBuffer?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return handle.GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Handle: {0}", handle);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(DynamicIndexBuffer left, DynamicIndexBuffer right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(DynamicIndexBuffer left, DynamicIndexBuffer right) {
            return !left.Equals(right);
        }
    }

    /// <summary>
    /// Represents a dynamically updateable vertex buffer.
    /// </summary>
    public unsafe struct DynamicVertexBuffer : IDisposable, IEquatable<DynamicVertexBuffer> {
        internal readonly ushort handle;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly DynamicVertexBuffer Invalid = new DynamicVertexBuffer();

        /// <summary>
        /// Initializes a new instance of the <see cref="DynamicVertexBuffer"/> struct.
        /// </summary>
        /// <param name="vertexCount">The number of vertices that fit in the buffer.</param>
        /// <param name="layout">The layout of the vertex data.</param>
        /// <param name="flags">Flags used to control buffer behavior.</param>
        public DynamicVertexBuffer (int vertexCount, VertexLayout layout, BufferFlags flags = BufferFlags.None) {
            handle = NativeMethods.bgfx_create_dynamic_vertex_buffer(vertexCount, ref layout.data, flags);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="DynamicVertexBuffer"/> struct.
        /// </summary>
        /// <param name="memory">The initial vertex data with which to populate the buffer.</param>
        /// <param name="layout">The layout of the vertex data.</param>
        /// <param name="flags">Flags used to control buffer behavior.</param>
        public DynamicVertexBuffer (MemoryBlock memory, VertexLayout layout, BufferFlags flags = BufferFlags.None) {
            handle = NativeMethods.bgfx_create_dynamic_vertex_buffer_mem(memory.ptr, ref layout.data, flags);
        }

        /// <summary>
        /// Updates the data in the buffer.
        /// </summary>
        /// <param name="startVertex">Index of the first vertex to update.</param>
        /// <param name="memory">The new vertex data with which to fill the buffer.</param>
        public void Update (int startVertex, MemoryBlock memory) {
            NativeMethods.bgfx_update_dynamic_vertex_buffer(handle, startVertex, memory.ptr);
        }

        /// <summary>
        /// Releases the vertex buffer.
        /// </summary>
        public void Dispose () {
            NativeMethods.bgfx_destroy_dynamic_vertex_buffer(handle);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (DynamicVertexBuffer other) {
            return handle == other.handle;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as DynamicVertexBuffer?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return handle.GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Handle: {0}", handle);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(DynamicVertexBuffer left, DynamicVertexBuffer right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(DynamicVertexBuffer left, DynamicVertexBuffer right) {
            return !left.Equals(right);
        }
    }

    /// <summary>
    /// An aggregated frame buffer, with one or more attached texture surfaces.
    /// </summary>
    public unsafe struct FrameBuffer : IDisposable, IEquatable<FrameBuffer> {
        internal readonly ushort handle;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly FrameBuffer Invalid = new FrameBuffer();

        /// <summary>
        /// Initializes a new instance of the <see cref="FrameBuffer"/> struct.
        /// </summary>
        /// <param name="width">The width of the render target.</param>
        /// <param name="height">The height of the render target.</param>
        /// <param name="format">The format of the new surface.</param>
        /// <param name="flags">Texture sampling flags.</param>
        public FrameBuffer (int width, int height, TextureFormat format, TextureFlags flags = TextureFlags.ClampU | TextureFlags.ClampV) {
            handle = NativeMethods.bgfx_create_frame_buffer((ushort)width, (ushort)height, format, flags);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="FrameBuffer"/> struct.
        /// </summary>
        /// <param name="ratio">The amount to scale when the backbuffer resizes.</param>
        /// <param name="format">The format of the new surface.</param>
        /// <param name="flags">Texture sampling flags.</param>
        public FrameBuffer (BackbufferRatio ratio, TextureFormat format, TextureFlags flags = TextureFlags.ClampU | TextureFlags.ClampV) {
            handle = NativeMethods.bgfx_create_frame_buffer_scaled(ratio, format, flags);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="FrameBuffer"/> struct.
        /// </summary>
        /// <param name="attachments">A set of attachments from which to build the frame buffer.</param>
        /// <param name="destroyTextures">if set to <c>true</c>, attached textures will be destroyed when the frame buffer is destroyed.</param>
        public FrameBuffer (Texture[] attachments, bool destroyTextures = false) {
            var count = (byte)attachments.Length;
            var handles = stackalloc ushort[count];
            for (int i = 0; i < count; i++)
                handles[i] = attachments[i].handle;

            handle = NativeMethods.bgfx_create_frame_buffer_from_handles(count, handles, destroyTextures);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="FrameBuffer"/> struct.
        /// </summary>
        /// <param name="windowHandle">The OS window handle to which the frame buffer is attached.</param>
        /// <param name="width">The width of the render target.</param>
        /// <param name="height">The height of the render target.</param>
        /// <param name="depthFormat">A desired format for a depth buffer, if applicable.</param>
        public FrameBuffer (IntPtr windowHandle, int width, int height, TextureFormat depthFormat = TextureFormat.UnknownDepth) {
            handle = NativeMethods.bgfx_create_frame_buffer_from_nwh(windowHandle, (ushort)width, (ushort)height, depthFormat);
        }

        /// <summary>
        /// Releases the frame buffer.
        /// </summary>
        public void Dispose () {
            NativeMethods.bgfx_destroy_frame_buffer(handle);
        }

        /// <summary>
        /// Blits the contents of the framebuffer to a texture.
        /// </summary>
        /// <param name="viewId">The view in which the blit will be ordered.</param>
        /// <param name="dest">The destination texture.</param>
        /// <param name="destX">The destination X position.</param>
        /// <param name="destY">The destination Y position.</param>
        /// <param name="attachment">The frame buffer attachment from which to blit.</param>
        /// <param name="sourceX">The source X position.</param>
        /// <param name="sourceY">The source Y position.</param>
        /// <param name="width">The width of the region to blit.</param>
        /// <param name="height">The height of the region to blit.</param>
        /// <remarks>The destination texture must be created with the <see cref="TextureFlags.BlitDestination"/> flag.</remarks>
        public void BlitTo (byte viewId, Texture dest, int destX, int destY, int attachment = 0, int sourceX = 0, int sourceY = 0,
                            int width = ushort.MaxValue, int height = ushort.MaxValue) {
            BlitTo(viewId, dest, 0, destX, destY, 0, attachment, 0, sourceX, sourceY, 0, width, height, 0);
        }

        /// <summary>
        /// Blits the contents of the framebuffer to a texture.
        /// </summary>
        /// <param name="viewId">The view in which the blit will be ordered.</param>
        /// <param name="dest">The destination texture.</param>
        /// <param name="destMip">The destination mip level.</param>
        /// <param name="destX">The destination X position.</param>
        /// <param name="destY">The destination Y position.</param>
        /// <param name="destZ">The destination Z position.</param>
        /// <param name="attachment">The frame buffer attachment from which to blit.</param>
        /// <param name="sourceMip">The source mip level.</param>
        /// <param name="sourceX">The source X position.</param>
        /// <param name="sourceY">The source Y position.</param>
        /// <param name="sourceZ">The source Z position.</param>
        /// <param name="width">The width of the region to blit.</param>
        /// <param name="height">The height of the region to blit.</param>
        /// <param name="depth">The depth of the region to blit.</param>
        /// <remarks>The destination texture must be created with the <see cref="TextureFlags.BlitDestination"/> flag.</remarks>
        public void BlitTo (byte viewId, Texture dest, int destMip, int destX, int destY, int destZ, int attachment = 0,
                            int sourceMip = 0, int sourceX = 0, int sourceY = 0, int sourceZ = 0,
                            int width = ushort.MaxValue, int height = ushort.MaxValue, int depth = ushort.MaxValue) {
            NativeMethods.bgfx_blit_frame_buffer(viewId, dest.handle, (byte)destMip, (ushort)destX, (ushort)destY, (ushort)destZ,
                handle, (byte)attachment, (byte)sourceMip, (ushort)sourceX, (ushort)sourceY, (ushort)sourceZ, (ushort)width, (ushort)height, (ushort)depth);
        }

        /// <summary>
        /// Reads the contents of the frame buffer and stores them in memory pointed to by <paramref name="data"/>.
        /// </summary>
        /// <param name="attachment">The frame buffer attachment from which to read.</param>
        /// <param name="data">The destination for the read image data.</param>
        /// <remarks>The attachment must have been created with the <see cref="TextureFlags.ReadBack"/> flag.</remarks>
        public void Read (int attachment, IntPtr data) {
            NativeMethods.bgfx_read_frame_buffer(handle, (byte)attachment, data);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (FrameBuffer other) {
            return handle == other.handle;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as FrameBuffer?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return handle.GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Handle: {0}", handle);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(FrameBuffer left, FrameBuffer right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(FrameBuffer left, FrameBuffer right) {
            return !left.Equals(right);
        }
    }

    /// <summary>
    /// Represents a static index buffer.
    /// </summary>
    public unsafe struct IndexBuffer : IDisposable, IEquatable<IndexBuffer> {
        internal readonly ushort handle;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly IndexBuffer Invalid = new IndexBuffer();

        /// <summary>
        /// Initializes a new instance of the <see cref="IndexBuffer"/> struct.
        /// </summary>
        /// <param name="memory">The 16-bit index data used to populate the buffer.</param>
        /// <param name="flags">Flags used to control buffer behavior.</param>
        public IndexBuffer (MemoryBlock memory, BufferFlags flags = BufferFlags.None) {
            handle = NativeMethods.bgfx_create_index_buffer(memory.ptr, flags);
        }

        /// <summary>
        /// Releases the index buffer.
        /// </summary>
        public void Dispose () {
            NativeMethods.bgfx_destroy_index_buffer(handle);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (IndexBuffer other) {
            return handle == other.handle;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as IndexBuffer?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return handle.GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Handle: {0}", handle);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(IndexBuffer left, IndexBuffer right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(IndexBuffer left, IndexBuffer right) {
            return !left.Equals(right);
        }
    }

    /// <summary>
    /// Represents a buffer that can contain indirect drawing commands created and processed entirely on the GPU.
    /// </summary>
    public unsafe struct IndirectBuffer : IDisposable, IEquatable<IndirectBuffer> {
        internal readonly ushort handle;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly IndirectBuffer Invalid = new IndirectBuffer();

        /// <summary>
        /// Initializes a new instance of the <see cref="IndirectBuffer"/> struct.
        /// </summary>
        /// <param name="size">The number of commands that can fit in the buffer.</param>
        public IndirectBuffer (int size) {
            handle = NativeMethods.bgfx_create_indirect_buffer(size);
        }

        /// <summary>
        /// Releases the index buffer.
        /// </summary>
        public void Dispose () {
            NativeMethods.bgfx_destroy_indirect_buffer(handle);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (IndirectBuffer other) {
            return handle == other.handle;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as IndirectBuffer?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return handle.GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Handle: {0}", handle);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(IndirectBuffer left, IndirectBuffer right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(IndirectBuffer left, IndirectBuffer right) {
            return !left.Equals(right);
        }
    }

    /// <summary>
    /// Maintains a data buffer that contains instancing data.
    /// </summary>
    public unsafe struct InstanceDataBuffer : IEquatable<InstanceDataBuffer> {
        internal readonly NativeStruct* ptr;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly InstanceDataBuffer Invalid = new InstanceDataBuffer();

        /// <summary>
        /// A pointer that can be filled with instance data.
        /// </summary>
        public IntPtr Data { get { return ptr->data; } }

        /// <summary>
        /// The size of the data buffer.
        /// </summary>
        public int Size { get { return ptr->size; } }

        /// <summary>
        /// Initializes a new instance of the <see cref="InstanceDataBuffer" /> struct.
        /// </summary>
        /// <param name="count">The number of elements in the buffer.</param>
        /// <param name="stride">The stride of each element.</param>
        public InstanceDataBuffer (int count, int stride) {
            ptr = NativeMethods.bgfx_alloc_instance_data_buffer(count, (ushort)stride);
        }

        /// <summary>
        /// Checks for available space to allocate an instance buffer.
        /// </summary>
        /// <param name="count">The number of elements to allocate.</param>
        /// <param name="stride">The stride of each element.</param>
        /// <returns><c>true</c> if there is space available to allocate the buffer.</returns>
        public static bool CheckAvailableSpace (int count, int stride) {
            return NativeMethods.bgfx_check_avail_instance_data_buffer(count, (ushort)stride);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (InstanceDataBuffer other) {
            return ptr == other.ptr;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as InstanceDataBuffer?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return new IntPtr(ptr).GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Size: {0}", Size);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(InstanceDataBuffer left, InstanceDataBuffer right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(InstanceDataBuffer left, InstanceDataBuffer right) {
            return !left.Equals(right);
        }

#pragma warning disable 649
        internal struct NativeStruct {
            public IntPtr data;
            public int size;
            public int offset;
            public ushort stride;
            public ushort num;
            public ushort handle;
        }
#pragma warning restore 649
    }

    /// <summary>
    /// Represents a block of memory managed by the graphics API.
    /// </summary>
    public unsafe struct MemoryBlock : IEquatable<MemoryBlock> {
        internal readonly DataPtr* ptr;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly MemoryBlock Invalid = new MemoryBlock();

        /// <summary>
        /// The pointer to the raw data.
        /// </summary>
        public IntPtr Data {
            get { return ptr == null ? IntPtr.Zero : ptr->Data; }
        }

        /// <summary>
        /// The size of the block, in bytes.
        /// </summary>
        public int Size {
            get { return ptr == null ? 0 : ptr->Size; }
        }

        MemoryBlock (DataPtr* ptr) {
            this.ptr = ptr;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="MemoryBlock"/> struct.
        /// </summary>
        /// <param name="size">The size of the block, in bytes.</param>
        public MemoryBlock (int size) {
            ptr = NativeMethods.bgfx_alloc(size);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="MemoryBlock"/> struct.
        /// </summary>
        /// <param name="data">A pointer to the initial data to copy into the new block.</param>
        /// <param name="size">The size of the block, in bytes.</param>
        public MemoryBlock (IntPtr data, int size) {
            ptr = NativeMethods.bgfx_copy(data, size);
        }

        /// <summary>
        /// Copies a managed array into a native graphics memory block.
        /// </summary>
        /// <typeparam name="T">The type of data in the array.</typeparam>
        /// <param name="data">The array to copy.</param>
        /// <returns>The native memory block containing the copied data.</returns>
        public static MemoryBlock FromArray<T>(T[] data) where T : struct {
            if (data == null || data.Length == 0)
                throw new ArgumentNullException("data");

            var gcHandle = GCHandle.Alloc(data, GCHandleType.Pinned);
            var block = new MemoryBlock(gcHandle.AddrOfPinnedObject(), Marshal.SizeOf(typeof(T)) * data.Length);

            gcHandle.Free();
            return block;
        }

        /// <summary>
        /// Creates a reference to the given data.
        /// </summary>
        /// <typeparam name="T">The type of data in the array.</typeparam>
        /// <param name="data">The array to reference.</param>
        /// <returns>The native memory block referring to the data.</returns>
        /// <remarks>
        /// The array must not be modified for at least 2 rendered frames.
        /// </remarks>
        public static MemoryBlock MakeRef<T>(T[] data) where T : struct {
            if (data == null || data.Length == 0)
                throw new ArgumentNullException("data");

            var gcHandle = GCHandle.Alloc(data, GCHandleType.Pinned);
            return MakeRef(gcHandle.AddrOfPinnedObject(), Marshal.SizeOf(typeof(T)) * data.Length, GCHandle.ToIntPtr(gcHandle), ReleaseHandleCallback);
        }

        /// <summary>
        /// Makes a reference to the given memory block.
        /// </summary>
        /// <param name="data">A pointer to the memory.</param>
        /// <param name="size">The size of the memory block.</param>
        /// <param name="userData">Arbitrary user data passed to the release callback.</param>
        /// <param name="callback">A function that will be called when the data is ready to be released.</param>
        /// <returns>A new memory block referring to the given data.</returns>
        /// <remarks>
        /// The memory referred to by the returned memory block must not be modified
        /// or released until the callback fires.
        /// </remarks>
        public static MemoryBlock MakeRef (IntPtr data, int size, IntPtr userData, ReleaseCallback callback) {
            return new MemoryBlock(NativeMethods.bgfx_make_ref_release(data, size, Marshal.GetFunctionPointerForDelegate(callback), userData));
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (MemoryBlock other) {
            return ptr == other.ptr;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as MemoryBlock?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return new IntPtr(ptr).GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Size: {0}", Size);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(MemoryBlock left, MemoryBlock right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(MemoryBlock left, MemoryBlock right) {
            return !left.Equals(right);
        }

#pragma warning disable 649
        internal struct DataPtr {
            public IntPtr Data;
            public int Size;
        }
#pragma warning restore 649

        static ReleaseCallback ReleaseHandleCallback = ReleaseHandle;
        static void ReleaseHandle (IntPtr userData) {
            var handle = GCHandle.FromIntPtr(userData);
            handle.Free();
        }
    }

    /// <summary>
    /// Represents an occlusion query.
    /// </summary>
    public unsafe struct OcclusionQuery : IDisposable, IEquatable<OcclusionQuery> {
        internal readonly ushort handle;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly OcclusionQuery Invalid = new OcclusionQuery();

        /// <summary>
        /// Gets the result of the query.
        /// </summary>
        public OcclusionQueryResult Result {
            get { return NativeMethods.bgfx_get_result(handle); }
        }

        OcclusionQuery (ushort handle) {
            this.handle = handle;
        }

        /// <summary>
        /// Creates a new query.
        /// </summary>
        /// <returns>The new occlusion query.</returns>
        public static OcclusionQuery Create() {
            return new OcclusionQuery(NativeMethods.bgfx_create_occlusion_query());
        }

        /// <summary>
        /// Releases the query.
        /// </summary>
        public void Dispose () {
            NativeMethods.bgfx_destroy_occlusion_query(handle);
        }

        /// <summary>
        /// Sets the condition for which the query should check.
        /// </summary>
        /// <param name="visible"><c>true</c> for visible; <c>false</c> for invisible.</param>
        public void SetCondition (bool visible) {
            NativeMethods.bgfx_set_condition(handle, visible);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (OcclusionQuery other) {
            return handle == other.handle;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as OcclusionQuery?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return handle.GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Handle: {0}", handle);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator == (OcclusionQuery left, OcclusionQuery right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator != (OcclusionQuery left, OcclusionQuery right) {
            return !left.Equals(right);
        }
    }

    /// <summary>
    /// Contains various performance metrics tracked by the library.
    /// </summary>
    public unsafe struct PerfStats {
        Stats* data;

        /// <summary>
        /// CPU frame start time.
        /// </summary>
        public long CpuTimeStart {
            get { return data->CpuTimeBegin; }
        }

        /// <summary>
        /// CPU frame end time.
        /// </summary>
        public long CpuTimeEnd {
            get { return data->CpuTimeEnd; }
        }

        /// <summary>
        /// CPU timer frequency.
        /// </summary>
        public long CpuTimerFrequency {
            get { return data->CpuTimerFrequency; }
        }

        /// <summary>
        /// Elapsed CPU time.
        /// </summary>
        public TimeSpan CpuElapsed {
            get { return TimeSpan.FromSeconds((double)(CpuTimeEnd - CpuTimeStart) / CpuTimerFrequency); }
        }

        /// <summary>
        /// GPU frame start time.
        /// </summary>
        public long GpuTimeStart {
            get { return data->GpuTimeBegin; }
        }

        /// <summary>
        /// GPU frame end time.
        /// </summary>
        public long GpuTimeEnd {
            get { return data->GpuTimeEnd; }
        }

        /// <summary>
        /// GPU timer frequency.
        /// </summary>
        public long GpuTimerFrequency {
            get { return data->GpuTimerFrequency; }
        }

        /// <summary>
        /// Elapsed GPU time.
        /// </summary>
        public TimeSpan GpuElapsed {
            get { return TimeSpan.FromSeconds((double)(GpuTimeEnd - GpuTimeStart) / GpuTimerFrequency); }
        }

        internal PerfStats (Stats* data) {
            this.data = data;
        }

#pragma warning disable 649
        internal struct Stats {
            public long CpuTimeBegin;
            public long CpuTimeEnd;
            public long CpuTimerFrequency;
            public long GpuTimeBegin;
            public long GpuTimeEnd;
            public long GpuTimerFrequency;
        }
#pragma warning restore 649
    }

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

    /// <summary>
    /// Represents a compiled and linked shader program.
    /// </summary>
    public struct Program : IDisposable, IEquatable<Program> {
        internal readonly ushort handle;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly Program Invalid = new Program();

        /// <summary>
        /// Initializes a new instance of the <see cref="Program"/> struct.
        /// </summary>
        /// <param name="vertexShader">The vertex shader.</param>
        /// <param name="fragmentShader">The fragment shader.</param>
        /// <param name="destroyShaders">if set to <c>true</c>, the shaders will be released after creating the program.</param>
        public Program (Shader vertexShader, Shader fragmentShader, bool destroyShaders = false) {
            handle = NativeMethods.bgfx_create_program(vertexShader.handle, fragmentShader.handle, destroyShaders);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Program"/> struct.
        /// </summary>
        /// <param name="computeShader">The compute shader.</param>
        /// <param name="destroyShaders">if set to <c>true</c>, the compute shader will be released after creating the program.</param>
        public Program (Shader computeShader, bool destroyShaders = false) {
            handle = NativeMethods.bgfx_create_compute_program(computeShader.handle, destroyShaders);
        }

        /// <summary>
        /// Releases the program.
        /// </summary>
        public void Dispose () {
            NativeMethods.bgfx_destroy_program(handle);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (Program other) {
            return handle == other.handle;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as Program?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return handle.GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Handle: {0}", handle);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(Program left, Program right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(Program left, Program right) {
            return !left.Equals(right);
        }
    }

    /// <summary>
    /// Specifies state information used to configure rendering operations.
    /// </summary>
    public struct RenderState : IEquatable<RenderState> {
        const int AlphaRefShift = 40;
        const int PointSizeShift = 52;
        const ulong AlphaRefMask = 0x0000ff0000000000;
        const ulong PointSizeMask = 0x0ff0000000000000;

        readonly ulong value;

        /// <summary>
        /// No state bits set.
        /// </summary>
        public static readonly RenderState None = 0;

        /// <summary>
        /// Enable writing color data to the framebuffer.
        /// </summary>
        public static readonly RenderState ColorWrite = 0x0000000000000001;

        /// <summary>
        /// Enable writing alpha data to the framebuffer.
        /// </summary>
        public static readonly RenderState AlphaWrite = 0x0000000000000002;

        /// <summary>
        /// Enable writing to the depth buffer.
        /// </summary>
        public static readonly RenderState DepthWrite = 0x0000000000000004;

        /// <summary>
        /// Use a "less than" comparison to pass the depth test.
        /// </summary>
        public static readonly RenderState DepthTestLess = 0x0000000000000010;

        /// <summary>
        /// Use a "less than or equal to" comparison to pass the depth test.
        /// </summary>
        public static readonly RenderState DepthTestLessEqual = 0x0000000000000020;

        /// <summary>
        /// Pass the depth test if both values are equal.
        /// </summary>
        public static readonly RenderState DepthTestEqual = 0x0000000000000030;

        /// <summary>
        /// Use a "greater than or equal to" comparison to pass the depth test.
        /// </summary>
        public static readonly RenderState DepthTestGreaterEqual = 0x0000000000000040;

        /// <summary>
        /// Use a "greater than" comparison to pass the depth test.
        /// </summary>
        public static readonly RenderState DepthTestGreater = 0x0000000000000050;

        /// <summary>
        /// Pass the depth test if both values are not equal.
        /// </summary>
        public static readonly RenderState DepthTestNotEqual = 0x0000000000000060;

        /// <summary>
        /// Never pass the depth test.
        /// </summary>
        public static readonly RenderState DepthTestNever = 0x0000000000000070;

        /// <summary>
        /// Always pass the depth test.
        /// </summary>
        public static readonly RenderState DepthTestAlways = 0x0000000000000080;

        /// <summary>
        /// Use a value of 0 as an input to a blend equation.
        /// </summary>
        public static readonly RenderState BlendZero = 0x0000000000001000;

        /// <summary>
        /// Use a value of 1 as an input to a blend equation.
        /// </summary>
        public static readonly RenderState BlendOne = 0x0000000000002000;

        /// <summary>
        /// Use the source pixel color as an input to a blend equation.
        /// </summary>
        public static readonly RenderState BlendSourceColor = 0x0000000000003000;

        /// <summary>
        /// Use one minus the source pixel color as an input to a blend equation.
        /// </summary>
        public static readonly RenderState BlendInverseSourceColor = 0x0000000000004000;

        /// <summary>
        /// Use the source pixel alpha as an input to a blend equation.
        /// </summary>
        public static readonly RenderState BlendSourceAlpha = 0x0000000000005000;

        /// <summary>
        /// Use one minus the source pixel alpha as an input to a blend equation.
        /// </summary>
        public static readonly RenderState BlendInverseSourceAlpha = 0x0000000000006000;

        /// <summary>
        /// Use the destination pixel alpha as an input to a blend equation.
        /// </summary>
        public static readonly RenderState BlendDestinationAlpha = 0x0000000000007000;

        /// <summary>
        /// Use one minus the destination pixel alpha as an input to a blend equation.
        /// </summary>
        public static readonly RenderState BlendInverseDestinationAlpha = 0x0000000000008000;

        /// <summary>
        /// Use the destination pixel color as an input to a blend equation.
        /// </summary>
        public static readonly RenderState BlendDestinationColor = 0x0000000000009000;

        /// <summary>
        /// Use one minus the destination pixel color as an input to a blend equation.
        /// </summary>
        public static readonly RenderState BlendInverseDestinationColor = 0x000000000000a000;

        /// <summary>
        /// Use the source pixel alpha (saturated) as an input to a blend equation.
        /// </summary>
        public static readonly RenderState BlendSourceAlphaSaturate = 0x000000000000b000;

        /// <summary>
        /// Use an application supplied blending factor as an input to a blend equation.
        /// </summary>
        public static readonly RenderState BlendFactor = 0x000000000000c000;

        /// <summary>
        /// Use one minus an application supplied blending factor as an input to a blend equation.
        /// </summary>
        public static readonly RenderState BlendInverseFactor = 0x000000000000d000;

        /// <summary>
        /// Blend equation: A + B
        /// </summary>
        public static readonly RenderState BlendEquationAdd = 0x0000000000000000;

        /// <summary>
        /// Blend equation: B - A
        /// </summary>
        public static readonly RenderState BlendEquationSub = 0x0000000010000000;

        /// <summary>
        /// Blend equation: A - B
        /// </summary>
        public static readonly RenderState BlendEquationReverseSub = 0x0000000020000000;

        /// <summary>
        /// Blend equation: min(a, b)
        /// </summary>
        public static readonly RenderState BlendEquationMin = 0x0000000030000000;

        /// <summary>
        /// Blend equation: max(a, b)
        /// </summary>
        public static readonly RenderState BlendEquationMax = 0x0000000040000000;

        /// <summary>
        /// Enable independent blending of simultaenous render targets.
        /// </summary>
        public static readonly RenderState BlendIndependent = 0x0000000400000000;

        /// <summary>
        /// Don't perform culling of back faces.
        /// </summary>
        public static readonly RenderState NoCulling = 0x0000000000000000;

        /// <summary>
        /// Perform culling of clockwise faces.
        /// </summary>
        public static readonly RenderState CullClockwise = 0x0000001000000000;

        /// <summary>
        /// Perform culling of counter-clockwise faces.
        /// </summary>
        public static readonly RenderState CullCounterclockwise = 0x0000002000000000;

        /// <summary>
        /// Primitive topology: triangle list.
        /// </summary>
        public static readonly RenderState PrimitiveTriangles = 0x0000000000000000;

        /// <summary>
        /// Primitive topology: triangle strip.
        /// </summary>
        public static readonly RenderState PrimitiveTriangleStrip = 0x0001000000000000;

        /// <summary>
        /// Primitive topology: line list.
        /// </summary>
        public static readonly RenderState PrimitiveLines = 0x0002000000000000;

        /// <summary>
        /// Primitive topology: line strip.
        /// </summary>
        public static readonly RenderState PrimitiveLineStrip = 0x0003000000000000;

        /// <summary>
        /// Primitive topology: point list.
        /// </summary>
        public static readonly RenderState PrimitivePoints = 0x0004000000000000;

        /// <summary>
        /// Enable multisampling.
        /// </summary>
        public static readonly RenderState Multisampling = 0x1000000000000000;

        /// <summary>
        /// Provides a set of sane defaults.
        /// </summary>
        public static readonly RenderState Default =
            ColorWrite |
            AlphaWrite |
            DepthWrite |
            DepthTestLess |
            CullClockwise |
            Multisampling;

        /// <summary>
        /// Predefined blend effect: additive blending.
        /// </summary>
        public static readonly RenderState BlendAdd = BlendFunction(BlendOne, BlendOne);

        /// <summary>
        /// Predefined blend effect: alpha blending.
        /// </summary>
        public static readonly RenderState BlendAlpha = BlendFunction(BlendSourceAlpha, BlendInverseSourceAlpha);

        /// <summary>
        /// Predefined blend effect: "darken" blending.
        /// </summary>
        public static readonly RenderState BlendDarken = BlendFunction(BlendOne, BlendOne) | BlendEquation(BlendEquationMin);

        /// <summary>
        /// Predefined blend effect: "lighten" blending.
        /// </summary>
        public static readonly RenderState BlendLighten = BlendFunction(BlendOne, BlendOne) | BlendEquation(BlendEquationMax);

        /// <summary>
        /// Predefined blend effect: multiplicative blending.
        /// </summary>
        public static readonly RenderState BlendMultiply = BlendFunction(BlendDestinationColor, BlendZero);

        /// <summary>
        /// Predefined blend effect: normal blending based on alpha.
        /// </summary>
        public static readonly RenderState BlendNormal = BlendFunction(BlendOne, BlendInverseSourceAlpha);

        /// <summary>
        /// Predefined blend effect: "screen" blending.
        /// </summary>
        public static readonly RenderState BlendScreen = BlendFunction(BlendOne, BlendInverseSourceColor);

        /// <summary>
        /// Predefined blend effect: "linear burn" blending.
        /// </summary>
        public static readonly RenderState BlendLinearBurn = BlendFunction(BlendDestinationColor, BlendInverseDestinationColor) | BlendEquation(BlendEquationSub);

        /// <summary>
        /// Initializes a new instance of the <see cref="RenderState"/> struct.
        /// </summary>
        /// <param name="value">The integer value of the state.</param>
        public RenderState (long value) {
            this.value = (ulong)value;
        }

        /// <summary>
        /// Encodes an alpha reference value in a render state.
        /// </summary>
        /// <param name="alpha">The alpha reference value.</param>
        /// <returns>The encoded render state.</returns>
        public static RenderState AlphaRef (byte alpha) {
            return (((ulong)alpha) << AlphaRefShift) & AlphaRefMask;
        }

        /// <summary>
        /// Encodes a point size value in a render state.
        /// </summary>
        /// <param name="size">The point size.</param>
        /// <returns>The encoded render state.</returns>
        public static RenderState PointSize (byte size) {
            return (((ulong)size) << PointSizeShift) & PointSizeMask;
        }

        /// <summary>
        /// Builds a render state for a blend function.
        /// </summary>
        /// <param name="source">The source blend operation.</param>
        /// <param name="destination">The destination blend operation.</param>
        /// <returns>The render state for the blend function.</returns>
        public static RenderState BlendFunction (RenderState source, RenderState destination) {
            return BlendFunction(source, destination, source, destination);
        }

        /// <summary>
        /// Builds a render state for a blend function.
        /// </summary>
        /// <param name="sourceColor">The source color blend operation.</param>
        /// <param name="destinationColor">The destination color blend operation.</param>
        /// <param name="sourceAlpha">The source alpha blend operation.</param>
        /// <param name="destinationAlpha">The destination alpha blend operation.</param>
        /// <returns>
        /// The render state for the blend function.
        /// </returns>
        public static RenderState BlendFunction (RenderState sourceColor, RenderState destinationColor, RenderState sourceAlpha, RenderState destinationAlpha) {
            return (sourceColor | (destinationColor << 4)) | ((sourceAlpha | (destinationAlpha << 4)) << 8);
        }

        /// <summary>
        /// Builds a render state for a blend equation.
        /// </summary>
        /// <param name="equation">The equation.</param>
        /// <returns>
        /// The render state for the blend equation.
        /// </returns>
        public static RenderState BlendEquation (RenderState equation) {
            return BlendEquation(equation, equation);
        }

        /// <summary>
        /// Builds a render state for a blend equation.
        /// </summary>
        /// <param name="sourceEquation">The source equation.</param>
        /// <param name="alphaEquation">The alpha equation.</param>
        /// <returns>
        /// The render state for the blend equation.
        /// </returns>
        public static RenderState BlendEquation (RenderState sourceEquation, RenderState alphaEquation) {
            return sourceEquation | (alphaEquation << 3);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return value.GetHashCode();
        }

        /// <summary>
        /// Determines whether the specific value is equal to this instance.
        /// </summary>
        /// <param name="other">The value to compare with this instance.</param>
        /// <returns><c>true</c> if the value is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (RenderState other) {
            return value == other.value;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var state = obj as RenderState?;
            if (state == null)
                return false;

            return Equals(state);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(RenderState left, RenderState right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(RenderState left, RenderState right) {
            return !left.Equals(right);
        }

        /// <summary>
        /// Performs an implicit conversion from ulong.
        /// </summary>
        /// <param name="value">The value to convert.</param>
        public static implicit operator RenderState (ulong value) {
            return new RenderState((long)value);
        }

        /// <summary>
        /// Performs an explicit conversion to ulong.
        /// </summary>
        /// <param name="state">The value to convert.</param>
        public static explicit operator ulong (RenderState state) {
            return state.value;
        }

        /// <summary>
        /// Implements the bitwise-or operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// The result of the operator.
        /// </returns>
        public static RenderState operator |(RenderState left, RenderState right) {
            return left.value | right.value;
        }

        /// <summary>
        /// Implements the bitwise-and operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// The result of the operator.
        /// </returns>
        public static RenderState operator &(RenderState left, RenderState right) {
            return left.value & right.value;
        }

        /// <summary>
        /// Implements the bitwise-complement operator.
        /// </summary>
        /// <param name="state">The operand.</param>
        /// <returns>
        /// The result of the operator.
        /// </returns>
        public static RenderState operator ~(RenderState state) {
            return ~state.value;
        }

        /// <summary>
        /// Implements the left shift operator.
        /// </summary>
        /// <param name="state">The value to shift.</param>
        /// <param name="amount">The amount to shift.</param>
        /// <returns>
        /// The result of the operator.
        /// </returns>
        public static RenderState operator <<(RenderState state, int amount) {
            return state.value << amount;
        }

        /// <summary>
        /// Implements the right shift operator.
        /// </summary>
        /// <param name="state">The value to shift.</param>
        /// <param name="amount">The amount to shift.</param>
        /// <returns>
        /// The result of the operator.
        /// </returns>
        public static RenderState operator >>(RenderState state, int amount) {
            return state.value >> amount;
        }
    }

    /// <summary>
    /// Represents a single compiled shader component.
    /// </summary>
    public unsafe struct Shader : IDisposable, IEquatable<Shader> {
        Uniform[] uniforms;
        internal readonly ushort handle;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly Shader Invalid = new Shader();

        /// <summary>
        /// The set of uniforms exposed by the shader.
        /// </summary>
        public IReadOnlyList<Uniform> Uniforms {
            get {
                if (uniforms == null) {
                    var count = NativeMethods.bgfx_get_shader_uniforms(handle, null, 0);
                    uniforms = new Uniform[count];
                    NativeMethods.bgfx_get_shader_uniforms(handle, uniforms, count);
                }

                return uniforms;
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Shader"/> struct.
        /// </summary>
        /// <param name="memory">The compiled shader memory.</param>
        public Shader (MemoryBlock memory) {
            handle = NativeMethods.bgfx_create_shader(memory.ptr);
            uniforms = null;
        }

        /// <summary>
        /// Releases the shader.
        /// </summary>
        public void Dispose () {
            NativeMethods.bgfx_destroy_shader(handle);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (Shader other) {
            return handle == other.handle;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as Shader?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return handle.GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Handle: {0}", handle);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(Shader left, Shader right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(Shader left, Shader right) {
            return !left.Equals(right);
        }
    }

    /// <summary>
    /// Specifies state information used to configure rendering operations.
    /// </summary>
    public struct StencilFlags : IEquatable<StencilFlags> {
        const int ReadMaskShift = 8;
        const uint RefMask = 0x000000ff;
        const uint ReadMaskMask = 0x0000ff00;

        readonly uint value;

        /// <summary>
        /// No state bits set.
        /// </summary>
        public static readonly StencilFlags None = 0;

        /// <summary>
        /// Perform a "less than" stencil test.
        /// </summary>
        public static readonly StencilFlags TestLess = 0x00010000;

        /// <summary>
        /// Perform a "less than or equal" stencil test.
        /// </summary>
        public static readonly StencilFlags TestLessEqual = 0x00020000;

        /// <summary>
        /// Perform an equality stencil test.
        /// </summary>
        public static readonly StencilFlags TestEqual = 0x00030000;

        /// <summary>
        /// Perform a "greater than or equal" stencil test.
        /// </summary>
        public static readonly StencilFlags TestGreaterEqual = 0x00040000;

        /// <summary>
        /// Perform a "greater than" stencil test.
        /// </summary>
        public static readonly StencilFlags TestGreater = 0x00050000;

        /// <summary>
        /// Perform an inequality stencil test.
        /// </summary>
        public static readonly StencilFlags TestNotEqual = 0x00060000;

        /// <summary>
        /// Never pass the stencil test.
        /// </summary>
        public static readonly StencilFlags TestNever = 0x00070000;

        /// <summary>
        /// Always pass the stencil test.
        /// </summary>
        public static readonly StencilFlags TestAlways = 0x00080000;

        /// <summary>
        /// On failing the stencil test, zero out the stencil value.
        /// </summary>
        public static readonly StencilFlags FailSZero = 0x00000000;

        /// <summary>
        /// On failing the stencil test, keep the old stencil value.
        /// </summary>
        public static readonly StencilFlags FailSKeep = 0x00100000;

        /// <summary>
        /// On failing the stencil test, replace the stencil value.
        /// </summary>
        public static readonly StencilFlags FailSReplace = 0x00200000;

        /// <summary>
        /// On failing the stencil test, increment the stencil value.
        /// </summary>
        public static readonly StencilFlags FailSIncrement = 0x00300000;

        /// <summary>
        /// On failing the stencil test, increment the stencil value (with saturation).
        /// </summary>
        public static readonly StencilFlags FailSIncrementSaturate = 0x00400000;

        /// <summary>
        /// On failing the stencil test, decrement the stencil value.
        /// </summary>
        public static readonly StencilFlags FailSDecrement = 0x00500000;

        /// <summary>
        /// On failing the stencil test, decrement the stencil value (with saturation).
        /// </summary>
        public static readonly StencilFlags FailSDecrementSaturate = 0x00600000;

        /// <summary>
        /// On failing the stencil test, invert the stencil value.
        /// </summary>
        public static readonly StencilFlags FailSInvert = 0x00700000;

        /// <summary>
        /// On failing the stencil test, zero out the depth value.
        /// </summary>
        public static readonly StencilFlags FailZZero = 0x00000000;

        /// <summary>
        /// On failing the stencil test, keep the depth value.
        /// </summary>
        public static readonly StencilFlags FailZKeep = 0x01000000;

        /// <summary>
        /// On failing the stencil test, replace the depth value.
        /// </summary>
        public static readonly StencilFlags FailZReplace = 0x02000000;

        /// <summary>
        /// On failing the stencil test, increment the depth value.
        /// </summary>
        public static readonly StencilFlags FailZIncrement = 0x03000000;

        /// <summary>
        /// On failing the stencil test, increment the depth value (with saturation).
        /// </summary>
        public static readonly StencilFlags FailZIncrementSaturate = 0x04000000;

        /// <summary>
        /// On failing the stencil test, decrement the depth value.
        /// </summary>
        public static readonly StencilFlags FailZDecrement = 0x05000000;

        /// <summary>
        /// On failing the stencil test, decrement the depth value (with saturation).
        /// </summary>
        public static readonly StencilFlags FailZDecrementSaturate = 0x06000000;

        /// <summary>
        /// On failing the stencil test, invert the depth value.
        /// </summary>
        public static readonly StencilFlags FailZInvert = 0x07000000;

        /// <summary>
        /// On passing the stencil test, zero out the depth value.
        /// </summary>
        public static readonly StencilFlags PassZZero = 0x00000000;

        /// <summary>
        /// On passing the stencil test, keep the old depth value.
        /// </summary>
        public static readonly StencilFlags PassZKeep = 0x10000000;

        /// <summary>
        /// On passing the stencil test, replace the depth value.
        /// </summary>
        public static readonly StencilFlags PassZReplace = 0x20000000;

        /// <summary>
        /// On passing the stencil test, increment the depth value.
        /// </summary>
        public static readonly StencilFlags PassZIncrement = 0x30000000;

        /// <summary>
        /// On passing the stencil test, increment the depth value (with saturation).
        /// </summary>
        public static readonly StencilFlags PassZIncrementSaturate = 0x40000000;

        /// <summary>
        /// On passing the stencil test, decrement the depth value.
        /// </summary>
        public static readonly StencilFlags PassZDecrement = 0x50000000;

        /// <summary>
        /// On passing the stencil test, decrement the depth value (with saturation).
        /// </summary>
        public static readonly StencilFlags PassZDecrementSaturate = 0x60000000;

        /// <summary>
        /// On passing the stencil test, invert the depth value.
        /// </summary>
        public static readonly StencilFlags PassZInvert = 0x70000000;

        /// <summary>
        /// Initializes a new instance of the <see cref="StencilFlags"/> struct.
        /// </summary>
        /// <param name="value">The integer value of the state.</param>
        public StencilFlags (int value) {
            this.value = (uint)value;
        }

        /// <summary>
        /// Encodes a reference value in a stencil state.
        /// </summary>
        /// <param name="reference">The stencil reference value.</param>
        /// <returns>The encoded stencil state.</returns>
        public static StencilFlags ReferenceValue (byte reference) {
            return reference & RefMask;
        }

        /// <summary>
        /// Encodes a read mask in a stencil state.
        /// </summary>
        /// <param name="mask">The mask.</param>
        /// <returns>
        /// The encoded stencil state.
        /// </returns>
        public static StencilFlags ReadMask (byte mask) {
            return (((uint)mask) << ReadMaskShift) & ReadMaskMask;
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return value.GetHashCode();
        }

        /// <summary>
        /// Determines whether the specific value is equal to this instance.
        /// </summary>
        /// <param name="other">The value to compare with this instance.</param>
        /// <returns><c>true</c> if the value is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (StencilFlags other) {
            return value == other.value;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var state = obj as StencilFlags?;
            if (state == null)
                return false;

            return Equals(state);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(StencilFlags left, StencilFlags right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(StencilFlags left, StencilFlags right) {
            return !left.Equals(right);
        }

        /// <summary>
        /// Performs an implicit conversion from uint.
        /// </summary>
        /// <param name="value">The value to convert.</param>
        public static implicit operator StencilFlags (uint value) {
            return new StencilFlags((int)value);
        }

        /// <summary>
        /// Performs an explicit conversion to uint.
        /// </summary>
        /// <param name="state">The value to convert.</param>
        public static explicit operator uint (StencilFlags state) {
            return state.value;
        }

        /// <summary>
        /// Implements the bitwise-or operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// The result of the operator.
        /// </returns>
        public static StencilFlags operator |(StencilFlags left, StencilFlags right) {
            return left.value | right.value;
        }

        /// <summary>
        /// Implements the bitwise-and operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// The result of the operator.
        /// </returns>
        public static StencilFlags operator &(StencilFlags left, StencilFlags right) {
            return left.value & right.value;
        }

        /// <summary>
        /// Implements the bitwise-complement operator.
        /// </summary>
        /// <param name="state">The operand.</param>
        /// <returns>
        /// The result of the operator.
        /// </returns>
        public static StencilFlags operator ~(StencilFlags state) {
            return ~state.value;
        }

        /// <summary>
        /// Implements the left shift operator.
        /// </summary>
        /// <param name="state">The value to shift.</param>
        /// <param name="amount">The amount to shift.</param>
        /// <returns>
        /// The result of the operator.
        /// </returns>
        public static StencilFlags operator <<(StencilFlags state, int amount) {
            return state.value << amount;
        }

        /// <summary>
        /// Implements the right shift operator.
        /// </summary>
        /// <param name="state">The value to shift.</param>
        /// <param name="amount">The amount to shift.</param>
        /// <returns>
        /// The result of the operator.
        /// </returns>
        public static StencilFlags operator >>(StencilFlags state, int amount) {
            return state.value >> amount;
        }
    }

    /// <summary>
    /// Maintains a transient index buffer.
    /// </summary>
    /// <remarks>
    /// The contents of the buffer are valid for the current frame only.
    /// You must call SetVertexBuffer with the buffer or a leak could occur.
    /// </remarks>
    public unsafe struct TransientIndexBuffer : IEquatable<TransientIndexBuffer> {
        readonly IntPtr data;
        int size;
        int startIndex;
        readonly ushort handle;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly TransientIndexBuffer Invalid = new TransientIndexBuffer();

        /// <summary>
        /// A pointer that can be filled with index data.
        /// </summary>
        public IntPtr Data { get { return data; } }

        /// <summary>
        /// The size of the buffer.
        /// </summary>
        public int Count { get { return size; } }

        /// <summary>
        /// Initializes a new instance of the <see cref="TransientIndexBuffer"/> struct.
        /// </summary>
        /// <param name="indexCount">The number of 16-bit indices that fit in the buffer.</param>
        public TransientIndexBuffer (int indexCount) {
            NativeMethods.bgfx_alloc_transient_index_buffer(out this, indexCount);
        }

        /// <summary>
        /// Check if there is available space in the global transient index buffer.
        /// </summary>
        /// <param name="count">The number of 16-bit indices to allocate.</param>
        /// <returns><c>true</c> if there is sufficient space for the give number of indices.</returns>
        public static bool CheckAvailableSpace (int count) {
            return NativeMethods.bgfx_check_avail_transient_index_buffer(count);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (TransientIndexBuffer other) {
            return handle == other.handle && data == other.data;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as TransientIndexBuffer?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return handle.GetHashCode() >> 13 ^ data.GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Count: {0}", Count);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(TransientIndexBuffer left, TransientIndexBuffer right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(TransientIndexBuffer left, TransientIndexBuffer right) {
            return !left.Equals(right);
        }
    }

    /// <summary>
    /// Maintains a transient vertex buffer.
    /// </summary>
    /// <remarks>
    /// The contents of the buffer are valid for the current frame only.
    /// You must call SetVertexBuffer with the buffer or a leak could occur.
    /// </remarks>
    public unsafe struct TransientVertexBuffer : IEquatable<TransientVertexBuffer> {
        readonly IntPtr data;
        int size;
        int startVertex;
        ushort stride;
        readonly ushort handle;
        ushort decl;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly TransientVertexBuffer Invalid = new TransientVertexBuffer();

        /// <summary>
        /// A pointer that can be filled with vertex data.
        /// </summary>
        public IntPtr Data { get { return data; } }

        /// <summary>
        /// The size of the buffer.
        /// </summary>
        public int Count { get { return size; } }

        /// <summary>
        /// Initializes a new instance of the <see cref="TransientVertexBuffer"/> struct.
        /// </summary>
        /// <param name="vertexCount">The number of vertices that fit in the buffer.</param>
        /// <param name="layout">The layout of the vertex data.</param>
        public TransientVertexBuffer (int vertexCount, VertexLayout layout) {
            NativeMethods.bgfx_alloc_transient_vertex_buffer(out this, vertexCount, ref layout.data);
        }

        /// <summary>
        /// Check if there is available space in the global transient vertex buffer.
        /// </summary>
        /// <param name="count">The number of vertices to allocate.</param>
        /// <param name="layout">The layout of each vertex.</param>
        /// <returns>
        ///   <c>true</c> if there is sufficient space for the give number of vertices.
        /// </returns>
        public static bool CheckAvailableSpace (int count, VertexLayout layout) {
            return NativeMethods.bgfx_check_avail_transient_vertex_buffer(count, ref layout.data);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (TransientVertexBuffer other) {
            return handle == other.handle && data == other.data;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as TransientVertexBuffer?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return handle.GetHashCode() >> 13 ^ data.GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Handle: {0}", handle);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(TransientVertexBuffer left, TransientVertexBuffer right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(TransientVertexBuffer left, TransientVertexBuffer right) {
            return !left.Equals(right);
        }
    }

    /// <summary>
    /// Represents a shader uniform.
    /// </summary>
    public struct Uniform : IDisposable, IEquatable<Uniform> {
        internal readonly ushort handle;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly Uniform Invalid = new Uniform();

        /// <summary>
        /// Initializes a new instance of the <see cref="Uniform"/> struct.
        /// </summary>
        /// <param name="name">The name of the uniform.</param>
        /// <param name="type">The type of data represented by the uniform.</param>
        /// <param name="arraySize">Size of the array, if the uniform is an array type.</param>
        /// <remarks>
        /// Predefined uniform names:
        /// u_viewRect vec4(x, y, width, height) - view rectangle for current view.
        /// u_viewTexel vec4 (1.0/width, 1.0/height, undef, undef) - inverse width and height
        /// u_view mat4 - view matrix
        /// u_invView mat4 - inverted view matrix
        /// u_proj mat4 - projection matrix
        /// u_invProj mat4 - inverted projection matrix
        /// u_viewProj mat4 - concatenated view projection matrix
        /// u_invViewProj mat4 - concatenated inverted view projection matrix
        /// u_model mat4[BGFX_CONFIG_MAX_BONES] - array of model matrices.
        /// u_modelView mat4 - concatenated model view matrix, only first model matrix from array is used.
        /// u_modelViewProj mat4 - concatenated model view projection matrix.
        /// u_alphaRef float - alpha reference value for alpha test.
        /// </remarks>
        public Uniform (string name, UniformType type, int arraySize = 1) {
            handle = NativeMethods.bgfx_create_uniform(name, type, (ushort)arraySize);
        }

        /// <summary>
        /// Releases the uniform.
        /// </summary>
        public void Dispose () {
            NativeMethods.bgfx_destroy_uniform(handle);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (Uniform other) {
            return handle == other.handle;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as Uniform?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return handle.GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Handle: {0}", handle);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(Uniform left, Uniform right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(Uniform left, Uniform right) {
            return !left.Equals(right);
        }
    }

    /// <summary>
    /// Represents a static vertex buffer.
    /// </summary>
    public unsafe struct VertexBuffer : IDisposable, IEquatable<VertexBuffer> {
        internal readonly ushort handle;

        /// <summary>
        /// Represents an invalid handle.
        /// </summary>
        public static readonly VertexBuffer Invalid = new VertexBuffer();

        /// <summary>
        /// Initializes a new instance of the <see cref="VertexBuffer"/> struct.
        /// </summary>
        /// <param name="memory">The vertex data with which to populate the buffer.</param>
        /// <param name="layout">The layout of the vertex data.</param>
        /// <param name="flags">Flags used to control buffer behavior.</param>
        public VertexBuffer (MemoryBlock memory, VertexLayout layout, BufferFlags flags = BufferFlags.None) {
            handle = NativeMethods.bgfx_create_vertex_buffer(memory.ptr, ref layout.data, flags);
        }

        /// <summary>
        /// Releases the vertex buffer.
        /// </summary>
        public void Dispose () {
            NativeMethods.bgfx_destroy_vertex_buffer(handle);
        }

        /// <summary>
        /// Determines whether the specified object is equal to this instance.
        /// </summary>
        /// <param name="other">The object to compare with this instance.</param>
        /// <returns><c>true</c> if the specified object is equal to this instance; otherwise, <c>false</c>.</returns>
        public bool Equals (VertexBuffer other) {
            return handle == other.handle;
        }

        /// <summary>
        /// Determines whether the specified <see cref="System.Object" /> is equal to this instance.
        /// </summary>
        /// <param name="obj">The <see cref="System.Object" /> to compare with this instance.</param>
        /// <returns>
        ///   <c>true</c> if the specified <see cref="System.Object" /> is equal to this instance; otherwise, <c>false</c>.
        /// </returns>
        public override bool Equals (object obj) {
            var other = obj as VertexBuffer?;
            if (other == null)
                return false;

            return Equals(other);
        }

        /// <summary>
        /// Returns a hash code for this instance.
        /// </summary>
        /// <returns>
        /// A hash code for this instance, suitable for use in hashing algorithms and data structures like a hash table.
        /// </returns>
        public override int GetHashCode () {
            return handle.GetHashCode();
        }

        /// <summary>
        /// Returns a <see cref="System.String"/> that represents this instance.
        /// </summary>
        /// <returns>
        /// A <see cref="System.String"/> that represents this instance.
        /// </returns>
        public override string ToString () {
            return string.Format("Handle: {0}", handle);
        }

        /// <summary>
        /// Implements the equality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator ==(VertexBuffer left, VertexBuffer right) {
            return left.Equals(right);
        }

        /// <summary>
        /// Implements the inequality operator.
        /// </summary>
        /// <param name="left">The left side of the operator.</param>
        /// <param name="right">The right side of the operator.</param>
        /// <returns>
        /// <c>true</c> if the two objects are not equal; otherwise, <c>false</c>.
        /// </returns>
        public static bool operator !=(VertexBuffer left, VertexBuffer right) {
            return !left.Equals(right);
        }
    }

    /// <summary>
    /// Specifies scaling relative to the size of the backbuffer.
    /// </summary>
    public enum BackbufferRatio {
        /// <summary>
        /// Surface is equal to the backbuffer size.
        /// </summary>
        Equal,

        /// <summary>
        /// Surface is half the backbuffer size.
        /// </summary>
        Half,

        /// <summary>
        /// Surface is a quater of the backbuffer size.
        /// </summary>
        Quater,

        /// <summary>
        /// Surface is an eighth of the backbuffer size.
        /// </summary>
        Eighth,

        /// <summary>
        /// Surface is a sixteenth of the backbuffer size.
        /// </summary>
        Sixteenth,

        /// <summary>
        /// Surface is double the backbuffer size.
        /// </summary>
        Double
    }

    /// <summary>
    /// Specifies various flags that control vertex and index buffer behavior.
    /// </summary>
    [Flags]
    public enum BufferFlags : short {
        /// <summary>
        /// No flags specified.
        /// </summary>
        None,

        /// <summary>
        /// Specifies the format of data in a compute buffer as being 8x1.
        /// </summary>
        ComputeFormat8x1 = 0x1,

        /// <summary>
        /// Specifies the format of data in a compute buffer as being 8x2.
        /// </summary>
        ComputeFormat8x2 = 0x2,

        /// <summary>
        /// Specifies the format of data in a compute buffer as being 8x4.
        /// </summary>
        ComputeFormat8x4 = 0x3,

        /// <summary>
        /// Specifies the format of data in a compute buffer as being 16x1.
        /// </summary>
        ComputeFormat16x1 = 0x4,

        /// <summary>
        /// Specifies the format of data in a compute buffer as being 16x2.
        /// </summary>
        ComputeFormat16x2 = 0x5,

        /// <summary>
        /// Specifies the format of data in a compute buffer as being 16x4.
        /// </summary>
        ComputeFormat16x4 = 0x6,

        /// <summary>
        /// Specifies the format of data in a compute buffer as being 32x1.
        /// </summary>
        ComputeFormat32x1 = 0x7,

        /// <summary>
        /// Specifies the format of data in a compute buffer as being 32x2.
        /// </summary>
        ComputeFormat32x2 = 0x8,

        /// <summary>
        /// Specifies the format of data in a compute buffer as being 32x4.
        /// </summary>
        ComputeFormat32x4 = 0x9,

        /// <summary>
        /// Specifies the type of data in a compute buffer as being unsigned integers.
        /// </summary>
        ComputeTypeUInt = 0x10,

        /// <summary>
        /// Specifies the type of data in a compute buffer as being signed integers.
        /// </summary>
        ComputeTypeInt = 0x20,

        /// <summary>
        /// Specifies the type of data in a compute buffer as being floating point values.
        /// </summary>
        ComputeTypeFloat = 0x30,

        /// <summary>
        /// Buffer will be read by a compute shader.
        /// </summary>
        ComputeRead = 0x100,

        /// <summary>
        /// Buffer will be written into by a compute shader. It cannot be accessed by the CPU.
        /// </summary>
        ComputeWrite = 0x200,

        /// <summary>
        /// Buffer is the source of indirect draw commands.
        /// </summary>
        DrawIndirect = 0x400,

        /// <summary>
        /// Buffer will resize on update if a different quantity of data is passed. If this flag is not set
        /// the data will be trimmed to fit in the existing buffer size. Effective only for dynamic buffers.
        /// </summary>
        AllowResize = 0x800,

        /// <summary>
        /// Buffer is using 32-bit indices. Useful only for index buffers.
        /// </summary>
        Index32 = 0x1000,

        /// <summary>
        /// Buffer will be read and written by a compute shader.
        /// </summary>
        ComputeReadWrite = ComputeRead | ComputeWrite
    }

    /// <summary>
    /// Specifies flags for clearing surfaces.
    /// </summary>
    [Flags]
    public enum ClearTargets : short {
        /// <summary>
        /// Don't clear anything.
        /// </summary>
        None = 0,

        /// <summary>
        /// Clear the color channels.
        /// </summary>
        Color = 0x1,

        /// <summary>
        /// Clear the depth buffer.
        /// </summary>
        Depth = 0x2,

        /// <summary>
        /// Clear the stencil buffer.
        /// </summary>
        Stencil = 0x4,

        /// <summary>
        /// Discard the first color framebuffer.
        /// </summary>
        DiscardColor0 = 0x8,

        /// <summary>
        /// Discard the second color framebuffer.
        /// </summary>
        DiscardColor1 = 0x10,

        /// <summary>
        /// Discard the third color framebuffer.
        /// </summary>
        DiscardColor2 = 0x20,

        /// <summary>
        /// Discard the fourth color framebuffer.
        /// </summary>
        DiscardColor3 = 0x40,

        /// <summary>
        /// Discard the fifth color framebuffer.
        /// </summary>
        DiscardColor4 = 0x80,

        /// <summary>
        /// Discard the sixth color framebuffer.
        /// </summary>
        DiscardColor5 = 0x100,

        /// <summary>
        /// Discard the seventh color framebuffer.
        /// </summary>
        DiscardColor6 = 0x200,

        /// <summary>
        /// Discard the eighth color framebuffer.
        /// </summary>
        DiscardColor7 = 0x400,

        /// <summary>
        /// Discard the depth buffer.
        /// </summary>
        DiscardDepth = 0x800,

        /// <summary>
        /// Discard the stencil buffer.
        /// </summary>
        DiscardStencil = 0x1000,
    }

    /// <summary>
    /// Describes access rights for a compute buffer.
    /// </summary>
    public enum ComputeBufferAccess {
        /// <summary>
        /// The buffer can only be read.
        /// </summary>
        Read,

        /// <summary>
        /// The buffer can only be written to.
        /// </summary>
        Write,

        /// <summary>
        /// The buffer can be read and written.
        /// </summary>
        ReadWrite
    }

    /// <summary>
    /// Addresses a particular face of a cube map.
    /// </summary>
    public enum CubeMapFace : byte {
        /// <summary>
        /// The right face.
        /// </summary>
        Right,

        /// <summary>
        /// The left face.
        /// </summary>
        Left,

        /// <summary>
        /// The top face.
        /// </summary>
        Top,

        /// <summary>
        /// The bottom face.
        /// </summary>
        Bottom,

        /// <summary>
        /// The front face.
        /// </summary>
        Front,

        /// <summary>
        /// The back face.
        /// </summary>
        Back
    }

    /// <summary>
    /// Specifies debug text colors.
    /// </summary>
    public enum DebugColor {
        /// <summary>
        /// Transparent.
        /// </summary>
        Transparent,

        /// <summary>
        /// Red.
        /// </summary>
        Red,

        /// <summary>
        /// Green.
        /// </summary>
        Green,

        /// <summary>
        /// Yellow.
        /// </summary>
        Yellow,

        /// <summary>
        /// Blue.
        /// </summary>
        Blue,

        /// <summary>
        /// Purple.
        /// </summary>
        Purple,

        /// <summary>
        /// Cyan.
        /// </summary>
        Cyan,

        /// <summary>
        /// Gray.
        /// </summary>
        Gray,

        /// <summary>
        /// Dark gray.
        /// </summary>
        DarkGray,

        /// <summary>
        /// Light red.
        /// </summary>
        LightRed,

        /// <summary>
        /// Light green.
        /// </summary>
        LightGreen,

        /// <summary>
        /// Light yellow.
        /// </summary>
        LightYellow,

        /// <summary>
        /// Light blue.
        /// </summary>
        LightBlue,

        /// <summary>
        /// Light purple.
        /// </summary>
        LightPurple,

        /// <summary>
        /// Light cyan.
        /// </summary>
        LightCyan,

        /// <summary>
        /// White.
        /// </summary>
        White
    }

    /// <summary>
    /// Specifies various debug options.
    /// </summary>
    [Flags]
    public enum DebugFeatures {
        /// <summary>
        /// Don't enable any debugging features.
        /// </summary>
        None = 0,

        /// <summary>
        /// Force wireframe rendering.
        /// </summary>
        Wireframe = 0x1,

        /// <summary>
        /// When set, all rendering calls are skipped. This is useful when profiling to
        /// find bottlenecks between the CPU and GPU.
        /// </summary>
        InfinitelyFastHardware = 0x2,

        /// <summary>
        /// Display internal statistics.
        /// </summary>
        DisplayStatistics = 0x4,

        /// <summary>
        /// Display debug text.
        /// </summary>
        DisplayText = 0x8
    }

    /// <summary>
    /// Specifies various capabilities supported by the rendering device.
    /// </summary>
    [Flags]
    public enum DeviceFeatures : long {
        /// <summary>
        /// No extra features supported.
        /// </summary>
        None = 0,

        /// <summary>
        /// Device supports "Less than or equal to" texture comparison mode.
        /// </summary>
        TextureCompareLessEqual = 0x1,

        /// <summary>
        /// Device supports other texture comparison modes.
        /// </summary>
        TextureCompareExtended = 0x2,

        /// <summary>
        /// Device supports all texture comparison modes.
        /// </summary>
        TextureCompareAll = TextureCompareLessEqual | TextureCompareExtended,

        /// <summary>
        /// Device supports 3D textures.
        /// </summary>
        Texture3D = 0x4,

        /// <summary>
        /// Device supports 16-bit floats as vertex attributes.
        /// </summary>
        VertexAttributeHalf = 0x8,

        /// <summary>
        /// UInt10 vertex attributes are supported.
        /// </summary>
        VertexAttributeUInt10 = 0x10,

        /// <summary>
        /// Device supports instancing.
        /// </summary>
        Instancing = 0x20,

        /// <summary>
        /// Device supports multithreaded rendering.
        /// </summary>
        RendererMultithreaded = 0x40,

        /// <summary>
        /// Fragment shaders can access depth values.
        /// </summary>
        FragmentDepth = 0x80,

        /// <summary>
        /// Device supports independent blending of simultaneous render targets.
        /// </summary>
        BlendIndependent = 0x100,

        /// <summary>
        /// Device supports compute shaders.
        /// </summary>
        Compute = 0x200,

        /// <summary>
        /// Device supports ordering of fragment output.
        /// </summary>
        FragmentOrdering = 0x400,

        /// <summary>
        /// Indicates whether the device can render to multiple swap chains.
        /// </summary>
        SwapChain = 0x800,

        /// <summary>
        /// Head mounted displays are supported.
        /// </summary>
        HeadMountedDisplay = 0x1000,

        /// <summary>
        /// Device supports 32-bit indices.
        /// </summary>
        Index32 = 0x2000,

        /// <summary>
        /// Device supports indirect drawing via GPU buffers.
        /// </summary>
        DrawIndirect = 0x4000,

        /// <summary>
        /// Device supports high-DPI rendering.
        /// </summary>
        HighDPI = 0x8000,

        /// <summary>
        /// Device supports texture blits.
        /// </summary>
        TextureBlit = 0x10000,

        /// <summary>
        /// Device supports reading back texture data.
        /// </summary>
        TextureReadBack = 0x20000,

        /// <summary>
        /// Device supports occlusion queries.
        /// </summary>
        OcclusionQuery = 0x40000
    }

    /// <summary>
    /// Specifies various error types that can be reported by bgfx.
    /// </summary>
    public enum ErrorType {
        /// <summary>
        /// A debug check failed; the program can safely continue, but the issue should be investigated.
        /// </summary>
        DebugCheck,

        /// <summary>
        /// The user's hardware failed checks for the minimum required specs.
        /// </summary>
        MinimumRequiredSpecs,

        /// <summary>
        /// The program tried to compile an invalid shader.
        /// </summary>
        InvalidShader,

        /// <summary>
        /// An error occurred during bgfx library initialization.
        /// </summary>
        UnableToInitialize,

        /// <summary>
        /// Failed while trying to create a texture.
        /// </summary>
        UnableToCreateTexture,

        /// <summary>
        /// The graphics device was lost and the library was unable to recover.
        /// </summary>
        DeviceLost
    }

    /// <summary>
    /// Specifies results of an occlusion query.
    /// </summary>
    public enum OcclusionQueryResult {
        /// <summary>
        /// Objects are invisible.
        /// </summary>
        Invisible,

        /// <summary>
        /// Objects are visible.
        /// </summary>
        Visible,

        /// <summary>
        /// Result is not ready or is unknown.
        /// </summary>
        NoResult
    }

    /// <summary>
    /// Specifies the supported rendering backend APIs.
    /// </summary>
    public enum RendererBackend {
        /// <summary>
        /// No backend given.
        /// </summary>
        Null,

        /// <summary>
        /// Direct3D 9
        /// </summary>
        Direct3D9,

        /// <summary>
        /// Direct3D 11
        /// </summary>
        Direct3D11,

        /// <summary>
        /// Direct3D 12
        /// </summary>
        Direct3D12,

        /// <summary>
        /// Apple Metal.
        /// </summary>
        Metal,

        /// <summary>
        /// OpenGL ES
        /// </summary>
        OpenGLES,

        /// <summary>
        /// OpenGL
        /// </summary>
        OpenGL,

        /// <summary>
        /// Vulkan
        /// </summary>
        Vulkan,

        /// <summary>
        /// Used during initialization; specifies that the library should
        /// pick the best renderer for the running hardware and OS.
        /// </summary>
        Default
    }

    /// <summary>
    /// Specifies various settings to change during a reset call.
    /// </summary>
    [Flags]
    public enum ResetFlags {
        /// <summary>
        /// No features to change.
        /// </summary>
        None = 0,

        /// <summary>
        /// Not yet supported.
        /// </summary>
        Fullscreen = 0x1,

        /// <summary>
        /// Enable 2x multisampling.
        /// </summary>
        MSAA2x = 0x10,

        /// <summary>
        /// Enable 4x multisampling.
        /// </summary>
        MSAA4x = 0x20,

        /// <summary>
        /// Enable 8x multisampling.
        /// </summary>
        MSAA8x = 0x30,

        /// <summary>
        /// Enable 16x multisampling.
        /// </summary>
        MSAA16x = 0x40,

        /// <summary>
        /// Enable v-sync.
        /// </summary>
        Vsync = 0x80,

        /// <summary>
        /// Use the maximum anisotropic filtering level available.
        /// </summary>
        MaxAnisotropy = 0x100,

        /// <summary>
        /// Begin screen capture.
        /// </summary>
        Capture = 0x200,

        /// <summary>
        /// Enable head mounted display support.
        /// </summary>
        HeadMountedDisplay = 0x400,

        /// <summary>
        /// Enable debugging for head mounted display rendering.
        /// </summary>
        HeadMountedDisplayDebug = 0x800,

        /// <summary>
        /// Recenter the head mounted display.
        /// </summary>
        HeadMountedDisplayRecenter = 0x1000,

        /// <summary>
        /// Flush all commands to the device after rendering.
        /// </summary>
        FlushAfterRender = 0x2000,

        /// <summary>
        /// Flip the backbuffer immediately after rendering for reduced latency.
        /// Only useful when multithreading is disabled.
        /// </summary>
        FlipAfterRender = 0x4000,

        /// <summary>
        /// Write data to the backbuffer in non-linear sRGB format.
        /// </summary>
        SrgbBackbuffer = 0x8000,

        /// <summary>
        /// Enable High-DPI rendering.
        /// </summary>
        HighDPI = 0x10000,

        /// <summary>
        /// Enables depth clamping.
        /// </summary>
        DepthClamp = 0x20000
    }

    /// <summary>
    /// Specifies various texture flags.
    /// </summary>
    [Flags]
    public enum TextureFlags {
        /// <summary>
        /// No flags set.
        /// </summary>
        None = 0x00000000,

        /// <summary>
        /// Mirror the texture in the U coordinate.
        /// </summary>
        MirrorU = 0x00000001,

        /// <summary>
        /// Clamp the texture in the U coordinate.
        /// </summary>
        ClampU = 0x00000002,

        /// <summary>
        /// Use a border color for addresses outside the range in the U coordinate.
        /// </summary>
        BorderU = 0x00000003,

        /// <summary>
        /// Mirror the texture in the V coordinate.
        /// </summary>
        MirrorV = 0x00000004,

        /// <summary>
        /// Clamp the texture in the V coordinate.
        /// </summary>
        ClampV = 0x00000008,

        /// <summary>
        /// Use a border color for addresses outside the range in the V coordinate.
        /// </summary>
        BorderV = 0x0000000c,

        /// <summary>
        /// Mirror the texture in the W coordinate.
        /// </summary>
        MirrorW = 0x00000010,

        /// <summary>
        /// Clamp the texture in the W coordinate.
        /// </summary>
        ClampW = 0x00000020,

        /// <summary>
        /// Use a border color for addresses outside the range in the W coordinate.
        /// </summary>
        BorderW = 0x00000030,

        /// <summary>
        /// Use point filtering for texture minification.
        /// </summary>
        MinFilterPoint = 0x00000040,

        /// <summary>
        /// Use anisotropic filtering for texture minification.
        /// </summary>
        MinFilterAnisotropic = 0x00000080,

        /// <summary>
        /// Use point filtering for texture magnification.
        /// </summary>
        MagFilterPoint = 0x00000100,

        /// <summary>
        /// Use anisotropic filtering for texture magnification.
        /// </summary>
        MagFilterAnisotropic = 0x00000200,

        /// <summary>
        /// Use point filtering for texture mipmaps.
        /// </summary>
        MipFilterPoint = 0x00000400,

        /// <summary>
        /// The texture will be used as a render target.
        /// </summary>
        RenderTarget = 0x00001000,

        /// <summary>
        /// The render target texture support 2x multisampling.
        /// </summary>
        RenderTargetMultisample2x = 0x00002000,

        /// <summary>
        /// The render target texture support 4x multisampling.
        /// </summary>
        RenderTargetMultisample4x = 0x00003000,

        /// <summary>
        /// The render target texture support 8x multisampling.
        /// </summary>
        RenderTargetMultisample8x = 0x00004000,

        /// <summary>
        /// The render target texture support 16x multisampling.
        /// </summary>
        RenderTargetMultisample16x = 0x00005000,

        /// <summary>
        /// The texture is only usable as a render target, not as a shader resource.
        /// </summary>
        RenderTargetBufferOnly = 0x00008000,

        /// <summary>
        /// Use a "less than" operator when comparing textures.
        /// </summary>
        CompareLess = 0x00010000,

        /// <summary>
        /// Use a "less than or equal" operator when comparing textures.
        /// </summary>
        CompareLessEqual = 0x00020000,

        /// <summary>
        /// Use an equality operator when comparing textures.
        /// </summary>
        CompareEqual = 0x00030000,

        /// <summary>
        /// Use a "greater than or equal" operator when comparing textures.
        /// </summary>
        CompareGreaterEqual = 0x00040000,

        /// <summary>
        /// Use a "greater than" operator when comparing textures.
        /// </summary>
        CompareGreater = 0x00050000,

        /// <summary>
        /// Use an inequality operator when comparing textures.
        /// </summary>
        CompareNotEqual = 0x00060000,

        /// <summary>
        /// Never compare two textures as equal.
        /// </summary>
        CompareNever = 0x00070000,

        /// <summary>
        /// Always compare two textures as equal.
        /// </summary>
        CompareAlways = 0x00080000,

        /// <summary>
        /// Texture is the target of compute shader writes.
        /// </summary>
        ComputeWrite = 0x00100000,

        /// <summary>
        /// Texture data is in non-linear sRGB format.
        /// </summary>
        Srgb = 0x00200000,

        /// <summary>
        /// Texture can be used as the destination of a blit operation.
        /// </summary>
        BlitDestination = 0x00400000,

        /// <summary>
        /// Texture data can be read back.
        /// </summary>
        ReadBack = 0x00800000
    }

    /// <summary>
    /// Specifies the format of a texture's data.
    /// </summary>
    /// <remarks>
    /// Check Caps flags for hardware format support.
    /// </remarks>
    public enum TextureFormat {
        /// <summary>
        /// Block compression with three color channels, 1 bit alpha.
        /// </summary>
        BC1,

        /// <summary>
        /// Block compression with three color channels, 4 bits alpha.
        /// </summary>
        BC2,

        /// <summary>
        /// Block compression with three color channels, 8 bits alpha.
        /// </summary>
        BC3,

        /// <summary>
        /// Block compression for 1-channel color.
        /// </summary>
        BC4,

        /// <summary>
        /// Block compression for 2-channel color.
        /// </summary>
        BC5,

        /// <summary>
        /// Block compression for three-channel HDR color.
        /// </summary>
        BC6H,

        /// <summary>
        /// Highest quality block compression.
        /// </summary>
        BC7,

        /// <summary>
        /// Original ETC block compression.
        /// </summary>
        ETC1,

        /// <summary>
        /// Improved ETC block compression (no alpha).
        /// </summary>
        ETC2,

        /// <summary>
        /// Improved ETC block compression with full alpha.
        /// </summary>
        ETC2A,

        /// <summary>
        /// Improved ETC block compression with 1-bit punchthrough alpha.
        /// </summary>
        ETC2A1,

        /// <summary>
        /// PVRTC1 compression (2 bits per pixel)
        /// </summary>
        PTC12,

        /// <summary>
        /// PVRTC1 compression (4 bits per pixel)
        /// </summary>
        PTC14,

        /// <summary>
        /// PVRTC1 compression with alpha (2 bits per pixel)
        /// </summary>
        PTC12A,

        /// <summary>
        /// PVRTC1 compression with alpha (4 bits per pixel)
        /// </summary>
        PTC14A,

        /// <summary>
        /// PVRTC2 compression with alpha (2 bits per pixel)
        /// </summary>
        PTC22,

        /// <summary>
        /// PVRTC2 compression with alpha (4 bits per pixel)
        /// </summary>
        PTC24,

        /// <summary>
        /// Unknown texture format.
        /// </summary>
        Unknown,

        /// <summary>
        /// 1-bit single channel.
        /// </summary>
        R1,

        /// <summary>
        /// 8-bit single channel (alpha).
        /// </summary>
        A8,

        /// <summary>
        /// 8-bit single channel.
        /// </summary>
        R8,

        /// <summary>
        /// 8-bit single channel (integer).
        /// </summary>
        R8I,

        /// <summary>
        /// 8-bit single channel (unsigned).
        /// </summary>
        R8U,

        /// <summary>
        /// 8-bit single channel (signed).
        /// </summary>
        R8S,

        /// <summary>
        /// 16-bit single channel.
        /// </summary>
        R16,

        /// <summary>
        /// 16-bit single channel (integer).
        /// </summary>
        R16I,

        /// <summary>
        /// 16-bit single channel (unsigned).
        /// </summary>
        R16U,

        /// <summary>
        /// 16-bit single channel (float).
        /// </summary>
        R16F,

        /// <summary>
        /// 16-bit single channel (signed).
        /// </summary>
        R16S,

        /// <summary>
        /// 32-bit single channel (integer).
        /// </summary>
        R32I,

        /// <summary>
        /// 32-bit single channel (unsigned).
        /// </summary>
        R32U,

        /// <summary>
        /// 32-bit single channel (float).
        /// </summary>
        R32F,

        /// <summary>
        /// 8-bit two channel.
        /// </summary>
        RG8,

        /// <summary>
        /// 8-bit two channel (integer).
        /// </summary>
        RG8I,

        /// <summary>
        /// 8-bit two channel (unsigned).
        /// </summary>
        RG8U,

        /// <summary>
        /// 8-bit two channel (signed).
        /// </summary>
        RG8S,

        /// <summary>
        /// 16-bit two channel.
        /// </summary>
        RG16,

        /// <summary>
        /// 16-bit two channel (integer).
        /// </summary>
        RG16I,

        /// <summary>
        /// 16-bit two channel (unsigned).
        /// </summary>
        RG16U,

        /// <summary>
        /// 16-bit two channel (float).
        /// </summary>
        RG16F,

        /// <summary>
        /// 16-bit two channel (signed).
        /// </summary>
        RG16S,

        /// <summary>
        /// 32-bit two channel (integer).
        /// </summary>
        RG32I,

        /// <summary>
        /// 32-bit two channel (unsigned).
        /// </summary>
        RG32U,

        /// <summary>
        /// 32-bit two channel (float).
        /// </summary>
        RG32F,

        /// <summary>
        /// 9-bit three channel floating point with shared 5-bit exponent.
        /// </summary>
        RGB9E5F,

        /// <summary>
        /// 8-bit BGRA color.
        /// </summary>
        BGRA8,

        /// <summary>
        /// 8-bit RGBA color.
        /// </summary>
        RGBA8,

        /// <summary>
        /// 8-bit RGBA color (integer).
        /// </summary>
        RGBA8I,

        /// <summary>
        /// 8-bit RGBA color (unsigned).
        /// </summary>
        RGBA8U,

        /// <summary>
        /// 8-bit RGBA color (signed).
        /// </summary>
        RGBA8S,

        /// <summary>
        /// 16-bit RGBA color.
        /// </summary>
        RGBA16,

        /// <summary>
        /// 16-bit RGBA color (integer).
        /// </summary>
        RGBA16I,

        /// <summary>
        /// 16-bit RGBA color (unsigned).
        /// </summary>
        RGBA16U,

        /// <summary>
        /// 16-bit RGBA color (float).
        /// </summary>
        RGBA16F,

        /// <summary>
        /// 16-bit RGBA color (signed).
        /// </summary>
        RGBA16S,

        /// <summary>
        /// 32-bit RGBA color (integer).
        /// </summary>
        RGBA32I,

        /// <summary>
        /// 32-bit RGBA color (unsigned).
        /// </summary>
        RGBA32U,

        /// <summary>
        /// 32-bit RGBA color (float).
        /// </summary>
        RGBA32F,

        /// <summary>
        /// 5-6-6 color.
        /// </summary>
        R5G6B5,

        /// <summary>
        /// 4-bit RGBA color.
        /// </summary>
        RGBA4,

        /// <summary>
        /// 5-bit RGB color with 1-bit alpha.
        /// </summary>
        RGB5A1,

        /// <summary>
        /// 10-bit RGB color with 2-bit alpha.
        /// </summary>
        RGB10A2,

        /// <summary>
        /// 11-11-10 color (float).
        /// </summary>
        R11G11B10F,

        /// <summary>
        /// Unknown depth format.
        /// </summary>
        UnknownDepth,

        /// <summary>
        /// 16-bit depth.
        /// </summary>
        D16,

        /// <summary>
        /// 24-bit depth.
        /// </summary>
        D24,

        /// <summary>
        /// 24-bit depth, 8-bit stencil.
        /// </summary>
        D24S8,

        /// <summary>
        /// 32-bit depth.
        /// </summary>
        D32,

        /// <summary>
        /// 16-bit depth (float).
        /// </summary>
        D16F,

        /// <summary>
        /// 24-bit depth (float).
        /// </summary>
        D24F,

        /// <summary>
        /// 32-bit depth (float).
        /// </summary>
        D32F,

        /// <summary>
        /// 8-bit stencil.
        /// </summary>
        D0S8
    }

    /// <summary>
    /// Indicates the level of support for a specific texture format.
    /// </summary>
    [Flags]
    public enum TextureFormatSupport {
        /// <summary>
        /// The format is unsupported.
        /// </summary>
        Unsupported = 0x0,

        /// <summary>
        /// The format is supported for 2D color data and operations.
        /// </summary>
        Color2D = 0x1,

        /// <summary>
        /// The format is supported for 2D sRGB operations.
        /// </summary>
        Srgb2D = 0x2,

        /// <summary>
        /// The format is supported for 2D textures through library emulation.
        /// </summary>
        Emulated2D = 0x4,

        /// <summary>
        /// The format is supported for 3D color data and operations.
        /// </summary>
        Color3D = 0x8,

        /// <summary>
        /// The format is supported for 3D sRGB operations.
        /// </summary>
        Srgb3D = 0x10,

        /// <summary>
        /// The format is supported for 3D textures through library emulation.
        /// </summary>
        Emulated3D = 0x20,

        /// <summary>
        /// The format is supported for cube color data and operations.
        /// </summary>
        ColorCube = 0x40,

        /// <summary>
        /// The format is supported for cube sRGB operations.
        /// </summary>
        SrgbCube = 0x80,

        /// <summary>
        /// The format is supported for cube textures through library emulation.
        /// </summary>
        EmulatedCube = 0x100,

        /// <summary>
        /// The format is supported for vertex texturing.
        /// </summary>
        Vertex = 0x200,

        /// <summary>
        /// The format is supported for compute image operations.
        /// </summary>
        Image = 0x400,

        /// <summary>
        /// The format is supported for framebuffers.
        /// </summary>
        Framebuffer = 0x800,

        /// <summary>
        /// The format is supported for MSAA framebuffers.
        /// </summary>
        FramebufferMSAA = 0x1000,

        /// <summary>
        /// The format is supported for MSAA sampling.
        /// </summary>
        MSAA = 0x2000
    }

    /// <summary>
    /// Specifies the type of uniform data.
    /// </summary>
    public enum UniformType {
        /// <summary>
        /// Single integer.
        /// </summary>
        Int1,

        /// <summary>
        /// 4D vector.
        /// </summary>
        Vector4 = 2,

        /// <summary>
        /// 3x3 matrix.
        /// </summary>
        Matrix3x3,

        /// <summary>
        ///4x4 matrix.
        /// </summary>
        Matrix4x4
    }

    /// <summary>
    /// Specifies known vendor IDs.
    /// </summary>
    public enum Vendor {
        /// <summary>
        /// No vendor specified.
        /// </summary>
        None = 0,

        /// <summary>
        /// Special flag to use platform's software rasterizer, if available.
        /// </summary>
        SoftwareRasterizer = 0x1,

        /// <summary>
        /// AMD
        /// </summary>
        AMD = 0x1002,

        /// <summary>
        /// Intel
        /// </summary>
        Intel = 0x8086,

        /// <summary>
        /// NVIDIA
        /// </summary>
        Nvidia = 0x10de,

        /// <summary>
        /// Microsoft
        /// </summary>
        Microsoft = 0x1414
    }

    /// <summary>
    /// Specifies data types for vertex attributes.
    /// </summary>
    public enum VertexAttributeType {
        /// <summary>
        /// One-byte unsigned integer.
        /// </summary>
        UInt8,

        /// <summary>
        /// 10-bit unsigned integer.
        /// </summary>
        /// <remarks>
        /// Availability depends on Caps flags.
        /// </remarks>
        UInt10,

        /// <summary>
        /// Two-byte signed integer.
        /// </summary>
        Int16,

        /// <summary>
        /// Two-byte float.
        /// </summary>
        /// <remarks>
        /// Availability depends on Caps flags.
        /// </remarks>
        Half,

        /// <summary>
        /// Four-byte float.
        /// </summary>
        Float
    }

    /// <summary>
    /// Specifies vertex attribute usages.
    /// </summary>
    public enum VertexAttributeUsage {
        /// <summary>
        /// Position data.
        /// </summary>
        Position,

        /// <summary>
        /// Normals.
        /// </summary>
        Normal,

        /// <summary>
        /// Tangents.
        /// </summary>
        Tangent,

        /// <summary>
        /// Bitangents.
        /// </summary>
        Bitangent,

        /// <summary>
        /// First color channel.
        /// </summary>
        Color0,

        /// <summary>
        /// Second color channel.
        /// </summary>
        Color1,

        /// <summary>
        /// Indices.
        /// </summary>
        Indices,

        /// <summary>
        /// Animation weights.
        /// </summary>
        Weight,

        /// <summary>
        /// First texture coordinate channel (arbitrary data).
        /// </summary>
        TexCoord0,

        /// <summary>
        /// Second texture coordinate channel (arbitrary data).
        /// </summary>
        TexCoord1,

        /// <summary>
        /// Third texture coordinate channel (arbitrary data).
        /// </summary>
        TexCoord2,

        /// <summary>
        /// Fourth texture coordinate channel (arbitrary data).
        /// </summary>
        TexCoord3,

        /// <summary>
        /// Fifth texture coordinate channel (arbitrary data).
        /// </summary>
        TexCoord4,

        /// <summary>
        /// Sixth texture coordinate channel (arbitrary data).
        /// </summary>
        TexCoord5,

        /// <summary>
        /// Seventh texture coordinate channel (arbitrary data).
        /// </summary>
        TexCoord6,

        /// <summary>
        /// Eighth texture coordinate channel (arbitrary data).
        /// </summary>
        TexCoord7
    }

    /// <summary>
    /// Delegate type for callback functions.
    /// </summary>
    /// <param name="userData">User-provided data to the original allocation call.</param>
    [SuppressUnmanagedCodeSecurity]
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void ReleaseCallback (IntPtr userData);

    [SuppressUnmanagedCodeSecurity]
    unsafe static class NativeMethods {
        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_update_texture_2d (ushort handle, byte mip, ushort x, ushort y, ushort width, ushort height, MemoryBlock.DataPtr* memory, ushort pitch);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_update_texture_3d (ushort handle, byte mip, ushort x, ushort y, ushort z, ushort width, ushort height, ushort depth, MemoryBlock.DataPtr* memory);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_update_texture_cube (ushort handle, CubeMapFace side, byte mip, ushort x, ushort y, ushort width, ushort height, MemoryBlock.DataPtr* memory, ushort pitch);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool bgfx_check_avail_transient_index_buffer (int num);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool bgfx_check_avail_transient_vertex_buffer (int num, ref VertexLayout.Data decl);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool bgfx_check_avail_instance_data_buffer (int num, ushort stride);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool bgfx_check_avail_transient_buffers (int numVertices, ref VertexLayout.Data decl, int numIndices);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_alloc_transient_index_buffer (out TransientIndexBuffer tib, int num);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_alloc_transient_vertex_buffer (out TransientVertexBuffer tvb, int num, ref VertexLayout.Data decl);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool bgfx_alloc_transient_buffers (out TransientVertexBuffer tvb, ref VertexLayout.Data decl, ushort numVertices, out TransientIndexBuffer tib, ushort numIndices);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern InstanceDataBuffer.NativeStruct* bgfx_alloc_instance_data_buffer (int num, ushort stride);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bgfx_dispatch (byte id, ushort program, ushort numX, ushort numY, ushort numZ, byte flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bgfx_dispatch_indirect (byte id, ushort program, ushort indirectHandle, ushort start, ushort num, byte flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_texture (byte stage, ushort sampler, ushort texture, uint flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_texture_from_frame_buffer (byte stage, ushort sampler, ushort frameBuffer, byte attachment, uint flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_image (byte stage, ushort sampler, ushort texture, byte mip, TextureFormat format, ComputeBufferAccess access);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_image_from_frame_buffer (byte stage, ushort sampler, ushort frameBuffer, byte attachment, TextureFormat format, ComputeBufferAccess access);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_compute_index_buffer (byte stage, ushort handle, ComputeBufferAccess access);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_compute_vertex_buffer (byte stage, ushort handle, ComputeBufferAccess access);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_compute_dynamic_index_buffer (byte stage, ushort handle, ComputeBufferAccess access);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_compute_dynamic_vertex_buffer (byte stage, ushort handle, ComputeBufferAccess access);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_compute_indirect_buffer (byte stage, ushort handle, ComputeBufferAccess access);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_frame_buffer (ushort width, ushort height, TextureFormat format, TextureFlags flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_frame_buffer_scaled (BackbufferRatio ratio, TextureFormat format, TextureFlags flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_frame_buffer_from_handles (byte count, ushort* handles, [MarshalAs(UnmanagedType.U1)] bool destroyTextures);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_frame_buffer_from_nwh (IntPtr nwh, ushort width, ushort height, TextureFormat depthFormat);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_destroy_frame_buffer (ushort handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_view_frame_buffer (byte id, ushort handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_program (ushort vsh, ushort fsh, [MarshalAs(UnmanagedType.U1)] bool destroyShaders);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_compute_program (ushort csh, [MarshalAs(UnmanagedType.U1)] bool destroyShaders);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_destroy_program (ushort handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern Capabilities.Caps* bgfx_get_caps ();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_vertex_decl_begin (ref VertexLayout.Data decl, RendererBackend backend);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_vertex_decl_add (ref VertexLayout.Data decl, VertexAttributeUsage attribute, byte count, VertexAttributeType type, [MarshalAs(UnmanagedType.U1)] bool normalized, [MarshalAs(UnmanagedType.U1)] bool asInt);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_vertex_decl_skip (ref VertexLayout.Data decl, byte count);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_vertex_decl_end (ref VertexLayout.Data decl);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_vertex_buffer (MemoryBlock.DataPtr* memory, ref VertexLayout.Data decl, BufferFlags flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_destroy_vertex_buffer (ushort handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_uniform ([MarshalAs(UnmanagedType.LPStr)] string name, UniformType type, ushort arraySize);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_destroy_uniform (ushort handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_texture (MemoryBlock.DataPtr* mem, TextureFlags flags, byte skip, out Texture.TextureInfo info);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_texture_2d (ushort width, ushort _height, byte numMips, TextureFormat format, TextureFlags flags, MemoryBlock.DataPtr* mem);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_texture_2d_scaled (BackbufferRatio ratio, byte numMips, TextureFormat format, TextureFlags flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_texture_3d (ushort width, ushort _height, ushort _depth, byte numMips, TextureFormat format, TextureFlags flags, MemoryBlock.DataPtr* mem);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_texture_cube (ushort size, byte numMips, TextureFormat format, TextureFlags flags, MemoryBlock.DataPtr* mem);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_destroy_texture (ushort handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_calc_texture_size (ref Texture.TextureInfo info, ushort width, ushort height, ushort depth, [MarshalAs(UnmanagedType.U1)] bool cubeMap, byte mipCount, TextureFormat format);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_shader (MemoryBlock.DataPtr* memory);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_get_shader_uniforms (ushort handle, Uniform[] uniforms, ushort max);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_destroy_shader (ushort handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern MemoryBlock.DataPtr* bgfx_alloc (int size);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern MemoryBlock.DataPtr* bgfx_copy (IntPtr data, int size);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern MemoryBlock.DataPtr* bgfx_make_ref_release (IntPtr data, int size, IntPtr releaseFn, IntPtr userData);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_index_buffer (MemoryBlock.DataPtr* memory, BufferFlags flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_destroy_index_buffer (ushort handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_dynamic_index_buffer (int indexCount, BufferFlags flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_dynamic_index_buffer_mem (MemoryBlock.DataPtr* memory, BufferFlags flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_update_dynamic_index_buffer (ushort handle, int startIndex, MemoryBlock.DataPtr* memory);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_destroy_dynamic_index_buffer (ushort handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_dynamic_vertex_buffer (int vertexCount, ref VertexLayout.Data decl, BufferFlags flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_dynamic_vertex_buffer_mem (MemoryBlock.DataPtr* memory, ref VertexLayout.Data decl, BufferFlags flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_update_dynamic_vertex_buffer (ushort handle, int startVertex, MemoryBlock.DataPtr* memory);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_destroy_dynamic_vertex_buffer (ushort handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_indirect_buffer (int size);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_destroy_indirect_buffer (ushort handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_image_swizzle_bgra8 (int width, int height, int pitch, IntPtr src, IntPtr dst);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_image_rgba8_downsample_2x2 (int width, int height, int pitch, IntPtr src, IntPtr dst);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_platform_data (ref PlatformData data);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern PerfStats.Stats* bgfx_get_stats ();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern RendererBackend bgfx_get_renderer_type ();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_shutdown ();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_reset (int width, int height, ResetFlags flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bgfx_frame ();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_debug (DebugFeatures flags);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_marker ([MarshalAs(UnmanagedType.LPStr)] string marker);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_dbg_text_clear (byte color, [MarshalAs(UnmanagedType.U1)] bool smallText);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bgfx_set_transform (float* matrix, ushort count);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_stencil (uint frontFace, uint backFace);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bgfx_touch (byte id);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bgfx_submit (byte id, ushort programHandle, int depth);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bgfx_submit_occlusion_query (byte id, ushort programHandle, ushort queryHandle, int depth);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int bgfx_submit_indirect (byte id, ushort programHandle, ushort indirectHandle, ushort start, ushort num, int depth);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_discard ();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_view_name (byte id, [MarshalAs(UnmanagedType.LPStr)] string name);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_view_rect (byte id, ushort x, ushort y, ushort width, ushort height);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_view_rect_auto (byte id, ushort x, ushort y, BackbufferRatio ratio);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_view_scissor (byte id, ushort x, ushort y, ushort width, ushort height);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_view_clear (byte id, ClearTargets flags, int rgba, float depth, byte stencil);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_view_clear_mrt (byte id, ClearTargets flags, float depth, byte stencil, byte rt0, byte rt1, byte rt2, byte rt3, byte rt4, byte rt5, byte rt6, byte rt7);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_palette_color (byte index, float* color);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_view_seq (byte id, [MarshalAs(UnmanagedType.U1)] bool enabled);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_view_transform (byte id, float* view, float* proj);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_save_screen_shot ([MarshalAs(UnmanagedType.LPStr)] string filePath);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_state (ulong state, int rgba);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_vertex_pack (float* input, [MarshalAs(UnmanagedType.U1)] bool inputNormalized, VertexAttributeUsage attribute, ref VertexLayout.Data decl, IntPtr data, int index);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_vertex_unpack (float* output, VertexAttributeUsage attribute, ref VertexLayout.Data decl, IntPtr data, int index);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_vertex_convert (ref VertexLayout.Data destDecl, IntPtr destData, ref VertexLayout.Data srcDecl, IntPtr srcData, int num);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_weld_vertices (ushort* output, ref VertexLayout.Data decl, IntPtr data, ushort num, float epsilon);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern byte bgfx_get_supported_renderers (RendererBackend[] backends);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern sbyte* bgfx_get_renderer_name (RendererBackend backend);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_init (RendererBackend backend, ushort vendorId, ushort deviceId, IntPtr callbacks, IntPtr allocator);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_dbg_text_printf (ushort x, ushort y, byte color, [MarshalAs(UnmanagedType.LPStr)] string format, [MarshalAs(UnmanagedType.LPStr)] string args);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_dbg_text_printf (ushort x, ushort y, byte color, byte* format, IntPtr args);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_dbg_text_image (ushort x, ushort y, ushort width, ushort height, IntPtr data, ushort pitch);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_index_buffer (ushort handle, int firstIndex, int count);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_dynamic_index_buffer (ushort handle, int firstIndex, int count);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_vertex_buffer (ushort handle, int startVertex, int count);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_dynamic_vertex_buffer (ushort handle, int count);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_uniform (ushort handle, void* value, ushort arraySize);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_transform_cached (int cache, ushort num);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_set_scissor (ushort x, ushort y, ushort width, ushort height);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_scissor_cached (ushort cache);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_transient_vertex_buffer (ref TransientVertexBuffer tvb, int startVertex, int numVertices);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_transient_index_buffer (ref TransientIndexBuffer tib, int startIndex, int numIndices);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_instance_data_buffer (InstanceDataBuffer.NativeStruct* idb, ushort num);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_instance_data_from_vertex_buffer (ushort handle, int startVertex, int count);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_instance_data_from_dynamic_vertex_buffer (ushort handle, int startVertex, int count);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_reset_view (byte id);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_blit (byte id, ushort dst, byte dstMip, ushort dstX, ushort dstY, ushort dstZ, ushort src,
                                             byte srcMip, ushort srcX, ushort srcY, ushort srcZ, ushort width, ushort height, ushort depth);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_blit_frame_buffer (byte id, ushort dst, byte dstMip, ushort dstX, ushort dstY, ushort dstZ, ushort src, byte attachment,
                                                          byte srcMip, ushort srcX, ushort srcY, ushort srcZ, ushort width, ushort height, ushort depth);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_read_texture (ushort handle, IntPtr data);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_read_frame_buffer (ushort handle, byte attachment, IntPtr data);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ushort bgfx_create_occlusion_query ();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_destroy_occlusion_query (ushort handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern OcclusionQueryResult bgfx_get_result (ushort handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern void bgfx_set_condition (ushort handle, [MarshalAs(UnmanagedType.U1)] bool visible);

#if DEBUG
        const string DllName = "bgfx_debug.dll";
#else
        const string DllName = "bgfx.dll";
#endif
    }

    struct CallbackShim {
        IntPtr vtbl;
        IntPtr reportError;
        IntPtr reportDebug;
        IntPtr getCachedSize;
        IntPtr getCacheEntry;
        IntPtr setCacheEntry;
        IntPtr saveScreenShot;
        IntPtr captureStarted;
        IntPtr captureFinished;
        IntPtr captureFrame;

        public static unsafe IntPtr CreateShim (ICallbackHandler handler) {
            if (handler == null)
                return IntPtr.Zero;

            if (savedDelegates != null)
                throw new InvalidOperationException("Callbacks should only be initialized once; bgfx can only deal with one set at a time.");

            var memory = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(CallbackShim)));
            var shim = (CallbackShim*)memory;
            var saver = new DelegateSaver(handler, shim);

            // the shim uses the unnecessary ctor slot to act as a vtbl pointer to itself,
            // so that the same block of memory can act as both bgfx_callback_interface_t and bgfx_callback_vtbl_t
            shim->vtbl = memory + IntPtr.Size;

            // cache the data so we can free it later
            shimMemory = memory;
            savedDelegates = saver;

            return memory;
        }

        public static void FreeShim () {
            if (savedDelegates == null)
                return;

            savedDelegates = null;
            Marshal.FreeHGlobal(shimMemory);
        }

        [SuppressUnmanagedCodeSecurity]
        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        delegate void ReportErrorHandler (IntPtr thisPtr, ErrorType errorType, string message);

        [SuppressUnmanagedCodeSecurity]
        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        delegate void ReportDebugHandler (IntPtr thisPtr, string fileName, ushort line, string format, IntPtr args);

        [SuppressUnmanagedCodeSecurity]
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        delegate int GetCachedSizeHandler (IntPtr thisPtr, long id);

        [SuppressUnmanagedCodeSecurity]
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        delegate bool GetCacheEntryHandler (IntPtr thisPtr, long id, IntPtr data, int size);

        [SuppressUnmanagedCodeSecurity]
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        delegate void SetCacheEntryHandler (IntPtr thisPtr, long id, IntPtr data, int size);

        [SuppressUnmanagedCodeSecurity]
        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        delegate void SaveScreenShotHandler (IntPtr thisPtr, string path, int width, int height, int pitch, IntPtr data, int size, [MarshalAs(UnmanagedType.U1)] bool flipVertical);

        [SuppressUnmanagedCodeSecurity]
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        delegate void CaptureStartedHandler (IntPtr thisPtr, int width, int height, int pitch, TextureFormat format, [MarshalAs(UnmanagedType.U1)] bool flipVertical);

        [SuppressUnmanagedCodeSecurity]
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        delegate void CaptureFinishedHandler (IntPtr thisPtr);

        [SuppressUnmanagedCodeSecurity]
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        delegate void CaptureFrameHandler (IntPtr thisPtr, IntPtr data, int size);

        // We're creating delegates to a user's interface methods; we're then converting those delegates
        // to native pointers and passing them into native code. If we don't save the references to the
        // delegates in managed land somewhere, the GC will think they're unreferenced and clean them
        // up, leaving native holding a bag of pointers into nowhere land.
        class DelegateSaver {
            ICallbackHandler handler;
            ReportErrorHandler reportError;
            ReportDebugHandler reportDebug;
            GetCachedSizeHandler getCachedSize;
            GetCacheEntryHandler getCacheEntry;
            SetCacheEntryHandler setCacheEntry;
            SaveScreenShotHandler saveScreenShot;
            CaptureStartedHandler captureStarted;
            CaptureFinishedHandler captureFinished;
            CaptureFrameHandler captureFrame;

            public unsafe DelegateSaver (ICallbackHandler handler, CallbackShim* shim) {
                this.handler = handler;
                reportError = ReportError;
                reportDebug = ReportDebug;
                getCachedSize = GetCachedSize;
                getCacheEntry = GetCacheEntry;
                setCacheEntry = SetCacheEntry;
                saveScreenShot = SaveScreenShot;
                captureStarted = CaptureStarted;
                captureFinished = CaptureFinished;
                captureFrame = CaptureFrame;

                shim->reportError = Marshal.GetFunctionPointerForDelegate(reportError);
                shim->reportDebug = Marshal.GetFunctionPointerForDelegate(reportDebug);
                shim->getCachedSize = Marshal.GetFunctionPointerForDelegate(getCachedSize);
                shim->getCacheEntry = Marshal.GetFunctionPointerForDelegate(getCacheEntry);
                shim->setCacheEntry = Marshal.GetFunctionPointerForDelegate(setCacheEntry);
                shim->saveScreenShot = Marshal.GetFunctionPointerForDelegate(saveScreenShot);
                shim->captureStarted = Marshal.GetFunctionPointerForDelegate(captureStarted);
                shim->captureFinished = Marshal.GetFunctionPointerForDelegate(captureFinished);
                shim->captureFrame = Marshal.GetFunctionPointerForDelegate(captureFrame);
            }

            void ReportError (IntPtr thisPtr, ErrorType errorType, string message) {
                handler.ReportError(errorType, message);
            }

            void ReportDebug (IntPtr thisPtr, string fileName, ushort line, string format, IntPtr args) {
                handler.ReportDebug(fileName, line, format, args);
            }

            int GetCachedSize (IntPtr thisPtr, long id) {
                return handler.GetCachedSize(id);
            }

            bool GetCacheEntry (IntPtr thisPtr, long id, IntPtr data, int size) {
                return handler.GetCacheEntry(id, data, size);
            }

            void SetCacheEntry (IntPtr thisPtr, long id, IntPtr data, int size) {
                handler.SetCacheEntry(id, data, size);
            }

            void SaveScreenShot (IntPtr thisPtr, string path, int width, int height, int pitch, IntPtr data, int size, bool flipVertical) {
                handler.SaveScreenShot(path, width, height, pitch, data, size, flipVertical);
            }

            void CaptureStarted (IntPtr thisPtr, int width, int height, int pitch, TextureFormat format, bool flipVertical) {
                handler.CaptureStarted(width, height, pitch, format, flipVertical);
            }

            void CaptureFinished (IntPtr thisPtr) {
                handler.CaptureFinished();
            }

            void CaptureFrame (IntPtr thisPtr, IntPtr data, int size) {
                handler.CaptureFrame(data, size);
            }
        }

        static IntPtr shimMemory;
        static DelegateSaver savedDelegates;
    }
}