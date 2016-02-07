using System;
using System.Collections.Generic;
using System.IO;
using SharpBgfx;
using System.IO.MemoryMappedFiles;
using System.Runtime.InteropServices;

namespace Common {
    public static class ResourceLoader {
        static readonly string RootPath = "../../../Assets/";

        static string GetShaderPath () {
            switch (Bgfx.GetCurrentBackend()) {
                case RendererBackend.Direct3D11:
                case RendererBackend.Direct3D12:
                    return Path.Combine(RootPath, "Shaders/bin/dx11/");

                case RendererBackend.OpenGL:
                    return Path.Combine(RootPath, "Shaders/bin/glsl/");

                case RendererBackend.OpenGLES:
                    return Path.Combine(RootPath, "Shaders/bin/gles/");

                case RendererBackend.Direct3D9:
                    return Path.Combine(RootPath, "Shaders/bin/dx9/");

                default:
                    throw new InvalidOperationException("Unknown renderer backend type.");
            }
        }

        public static Shader LoadShader (string name) {
            var path = Path.Combine(GetShaderPath(), name) + ".bin";
            var mem = MemoryBlock.FromArray(File.ReadAllBytes(path));
            return new Shader(mem);
        }

        public static Program LoadProgram (string vsName, string fsName) {
            var vsh = LoadShader(vsName);
            var fsh = LoadShader(fsName);

            return new Program(vsh, fsh, true);
        }

        public static Program LoadProgram (string csName) {
            var csh = LoadShader(csName);
            return new Program(csh, true);
        }

        public static Texture LoadTexture (string name) {
            var path = Path.Combine(RootPath, "textures/", name);
            var mem = MemoryBlock.FromArray(File.ReadAllBytes(path));
            return Texture.FromFile(mem, TextureFlags.None, 0);
        }

        public static Mesh LoadMesh (string fileName) {
            var path = Path.Combine(RootPath, "meshes/", fileName);
            var groups = new List<MeshGroup>();
            var group = new MeshGroup();
            VertexLayout layout = null;

            using (var file = MemoryMappedFile.CreateFromFile(path)) {
                var reader = file.CreateViewAccessor();
                var bytes = new FileInfo(path).Length;
                var index = 0;

                while (index < bytes) {
                    var tag = reader.ReadUInt32(index); index += sizeof(uint);
                    if (tag == ChunkTagVB) {
                        // skip bounding volume info
                        index += BoundingVolumeSize;

                        layout = reader.ReadVertexLayout(ref index);

                        var vertexCount = reader.ReadUInt16(ref index);
                        var vertexData = reader.ReadArray<byte>(vertexCount * layout.Stride, ref index);
                        group.VertexBuffer = new VertexBuffer(MemoryBlock.FromArray(vertexData), layout);
                    }
                    else if (tag == ChunkTagIB) {
                        var indexCount = reader.ReadUInt32(ref index);
                        var indexData = reader.ReadArray<ushort>((int)indexCount, ref index);

                        group.IndexBuffer = new IndexBuffer(MemoryBlock.FromArray(indexData));
                    }
                    else if (tag == ChunkTagPri) {
                        // skip material name
                        var len = reader.ReadUInt16(ref index);
                        index += len;

                        // read primitive data
                        var count = reader.ReadUInt16(ref index);
                        for (int i = 0; i < count; i++) {
                            // skip name
                            len = reader.ReadUInt16(ref index);
                            index += len;

                            var prim = reader.Read<Primitive>(ref index);
                            group.Primitives.Add(prim);

                            // skip bounding volumes
                            index += BoundingVolumeSize;
                        }

                        groups.Add(group);
                        group = new MeshGroup();
                    }
                }
            }

            return new Mesh(layout, groups);
        }

        static uint MakeFourCC (char a, char b, char c, char d) {
            return a | ((uint)b << 8) | ((uint)c << 16) | ((uint)d << 24);
        }

        const int BoundingVolumeSize = 26 * sizeof(float);
        static readonly uint ChunkTagVB = MakeFourCC('V', 'B', ' ', '\x1');
        static readonly uint ChunkTagIB = MakeFourCC('I', 'B', ' ', '\0');
        static readonly uint ChunkTagPri = MakeFourCC('P', 'R', 'I', '\0');
    }

    static class Extensions {
        public static VertexLayout ReadVertexLayout (this UnmanagedMemoryAccessor reader, ref int index) {
            var layout = new VertexLayout();
            layout.Begin();

            var attributeCount = reader.ReadByte(ref index);
            var stride = reader.ReadUInt16(ref index);

            for (int i = 0; i < attributeCount; i++) {
                var offset = reader.ReadUInt16(ref index);
                var attrib = reader.ReadUInt16(ref index);
                var count = reader.ReadByte(ref index);
                var attribType = reader.ReadUInt16(ref index);
                var normalized = reader.ReadBool(ref index);
                var asInt = reader.ReadBool(ref index);

                var usage = attributeUsageMap[attrib];
                layout.Add(usage, count, attributeTypeMap[attribType], normalized, asInt);

                if (layout.GetOffset(usage) != offset)
                    throw new InvalidOperationException("Invalid mesh data; vertex attribute offset mismatch.");
            }

            layout.End();
            if (layout.Stride != stride)
                throw new InvalidOperationException("Invalid mesh data; vertex layout stride mismatch.");

            return layout;
        }

        public static byte ReadByte (this UnmanagedMemoryAccessor reader, ref int index) {
            var result = reader.ReadByte(index);
            index++;
            return result;
        }

        public static bool ReadBool (this UnmanagedMemoryAccessor reader, ref int index) {
            var result = reader.ReadByte(index);
            index++;
            return result != 0;
        }

        public static ushort ReadUInt16 (this UnmanagedMemoryAccessor reader, ref int index) {
            var result = reader.ReadUInt16(index);
            index += sizeof(ushort);
            return result;
        }

        public static uint ReadUInt32 (this UnmanagedMemoryAccessor reader, ref int index) {
            var result = reader.ReadUInt32(index);
            index += sizeof(uint);
            return result;
        }

        public static T Read<T>(this UnmanagedMemoryAccessor reader, ref int index) where T : struct {
            T result;
            reader.Read(index, out result);
            index += Marshal.SizeOf(typeof(T));
            return result;
        }

        public static T[] ReadArray<T>(this UnmanagedMemoryAccessor reader, int count, ref int index) where T : struct {
            var result = new T[count];
            reader.ReadArray(index, result, 0, count);
            index += Marshal.SizeOf(typeof(T)) * count;
            return result;
        }

        static readonly Dictionary<ushort, VertexAttributeType> attributeTypeMap = new Dictionary<ushort, VertexAttributeType> {
            { 0x1, VertexAttributeType.UInt8 },
            { 0x2, VertexAttributeType.Int16 },
            { 0x3, VertexAttributeType.Half },
            { 0x4, VertexAttributeType.Float }
        };

        static readonly Dictionary<ushort, VertexAttributeUsage> attributeUsageMap = new Dictionary<ushort, VertexAttributeUsage> {
            { 0x1, VertexAttributeUsage.Position },
            { 0x2, VertexAttributeUsage.Normal },
            { 0x3, VertexAttributeUsage.Tangent },
            { 0x4, VertexAttributeUsage.Bitangent },
            { 0x5, VertexAttributeUsage.Color0 },
            { 0x6, VertexAttributeUsage.Color1 },
            { 0xe, VertexAttributeUsage.Indices },
            { 0xf, VertexAttributeUsage.Weight },
            { 0x10, VertexAttributeUsage.TexCoord0 },
            { 0x11, VertexAttributeUsage.TexCoord1 },
            { 0x12, VertexAttributeUsage.TexCoord2 },
            { 0x13, VertexAttributeUsage.TexCoord3 },
            { 0x14, VertexAttributeUsage.TexCoord4 },
            { 0x15, VertexAttributeUsage.TexCoord5 },
            { 0x16, VertexAttributeUsage.TexCoord6 },
            { 0x17, VertexAttributeUsage.TexCoord7 }
        };
    }
}
