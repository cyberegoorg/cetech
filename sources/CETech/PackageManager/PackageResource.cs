using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using CETech.Utils;
using YamlDotNet.RepresentationModel;

namespace CETech
{
    public class PackageResource
    {
        public static readonly StringId64 Type = new StringId64("package");

        public static unsafe void compile(ResourceCompiler.CompilatorAPI capi)
        {
            TextReader input = new StreamReader(capi.ResourceFile);
            var yaml = new YamlStream();
            yaml.Load(input);

            var rootNode = yaml.Documents[0].RootNode as YamlMappingNode;

            var typeHeaders = new List<TypeHeader>();
            var names = new List<long>();

            foreach (var type in rootNode.Children)
            {
                var typestr = type.Key as YamlScalarNode;
                var maping = type.Value as YamlSequenceNode;

                var typeid = new StringId64(typestr.Value);

                typeHeaders.Add(new TypeHeader {type = typeid, count = (ulong) maping.Children.Count, offset = 0});

                foreach (var name in maping.Children)
                {
                    var nameid = new StringId64(((YamlScalarNode) name).Value);
                    names.Add(nameid);
                }
            }

            var b = ByteUtils.ToByteArray(new Header {count = (ulong) typeHeaders.Count});
            capi.BuildFile.Write(b, 0, b.Length);

            var names_offset = (ulong) (sizeof (Header) + sizeof (TypeHeader)*typeHeaders.Count);
            for (var i = 0; i < typeHeaders.Count; i++)
            {
                var th = typeHeaders[i];
                th.offset = names_offset;

                b = ByteUtils.ToByteArray(th);
                capi.BuildFile.Write(b, 0, b.Length);

                names_offset += sizeof (long)*typeHeaders[i].count;
            }

            for (var i = 0; i < names.Count; i++)
            {
                b = BitConverter.GetBytes(names[i]);
                capi.BuildFile.Write(b, 0, b.Length);
            }
        }

        public static byte[] ResourceLoader(Stream input)
        {
            var buffer = new byte[input.Length];
            input.Read(buffer, 0, (int) input.Length);
            return buffer;
        }

        public static void ResourceOffline(byte[] data)
        {
        }

        public static void ResourceOnline(byte[] data)
        {
        }

        public static void ResourceUnloader(byte[] data)
        {
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct Header
        {
            public ulong count;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct TypeHeader
        {
            public long type;
            public ulong count;
            public ulong offset;
        }
    }
}