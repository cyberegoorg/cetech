using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Yaml;
using CETech.CEMath;
using CETech.Develop;
using CETech.Resource;
using MsgPack.Serialization;
using SharpBgfx;

namespace CETech
{
    /// <summary>
    ///     Package resource
    /// </summary>
    public class MeshResource
    {
        /// <summary>
        ///     Resource type
        /// </summary>
        public static readonly long Type = StringId.FromString("mesh");

        /// <summary>
        ///     Resource loader
        /// </summary>
        /// <param name="input">Resource data stream</param>
        /// <returns>Resource data</returns>
        public static object ResourceLoader(Stream input)
        {
            var serializer = MessagePackSerializer.Get<Resource>();
            var resource = serializer.Unpack(input);
            return new MeshInstance() { resource = resource };
        }

        /// <summary>
        ///     Resource offline.
        /// </summary>
        /// <param name="data">Data</param>
        public static void ResourceOffline(object data)
        {
        }

        /// <summary>
        ///     Resource online
        /// </summary>
        /// <param name="data">Data</param>
        public static void ResourceOnline(object data)
        {
            var instance = (MeshInstance) data;
            var resource = instance.resource;

            instance.ib = new IndexBuffer[resource.geom_name.Length];
            instance.vb = new VertexBuffer[resource.geom_name.Length];
            instance.vl = new VertexLayout[resource.geom_name.Length];

            for (int i = 0; i < resource.geom_name.Length; i++)
            {
                instance.vl[i] = new VertexLayout().Begin();
                var layout = instance.vl[i];
                
                layout.Add(VertexAttributeUsage.Position, 3, VertexAttributeType.Float);

                if (resource.normal_enabled[i])
                {
                    layout.Add(VertexAttributeUsage.Normal, 3, VertexAttributeType.Float, true);
                }


                if (resource.texcoord_enabled[i])
                {
                    layout.Add(VertexAttributeUsage.TexCoord0, 2, VertexAttributeType.Float);
                }

                if (resource.tangent_enable[i])
                {
                    layout.Add(VertexAttributeUsage.Tangent, 3, VertexAttributeType.Float, true);
                }

                if (resource.bitangent_enabled[i])
                {
                    layout.Add(VertexAttributeUsage.Bitangent, 3, VertexAttributeType.Float, true);
                }

                layout.End();

                instance.ib[i] = new IndexBuffer(MemoryBlock.FromArray(resource.geom_ib[i]));
                instance.vb[i] = new VertexBuffer(MemoryBlock.FromArray(resource.geom_vb[i]), layout);

                Debug.Assert(instance.ib[i] != IndexBuffer.Invalid);
                Debug.Assert(instance.vb[i] != VertexBuffer.Invalid);
            }
        }

        /// <summary>
        ///     Resource unloader
        /// </summary>
        /// <param name="data">data</param>
        public static void ResourceUnloader(object data)
        {
        }

        public static object ResourceReloader(long name, object new_data)
        {
            var old = CETech.Resource.Resource.Get<Resource>(Type, name);
            var neww = (Resource) new_data;

            return old;
        }

        public class MeshInstance
        {
            public Resource resource;
            public VertexBuffer[] vb;
            public IndexBuffer[] ib;
            public VertexLayout[] vl;
        }

        public class Resource
        {
            public long[] geom_name;
            public bool[] normal_enabled;
            public bool[] texcoord_enabled;
            public bool[] tangent_enable;
            public bool[] bitangent_enabled;

            public List<byte[]> geom_vb = new List<byte[]>();
            public List<short[]> geom_ib = new List<short[]>();
        }

#if CETECH_DEVELOP

        private static int WriteFloat(byte[] bytes, int idx, float value)
        {
            byte[] byteArray = BitConverter.GetBytes(value);
            for (int i = 0; i < byteArray.Length; i++)
            {
                bytes[idx + i] = byteArray[i];
            }
            return byteArray.Length;
        }
        
        private static short YamlToShort(YamlNode node)
        {
            return short.Parse(((YamlScalar) node).Value);
        }

        private static float YamlToFloat(YamlNode node)
        {
            return float.Parse(((YamlScalar)node).Value, CultureInfo.InvariantCulture);
        }

        /// <summary>
        ///     Resource compiler
        /// </summary>
        /// <param name="capi">Compiler api</param>
        public static void Compile(ResourceCompiler.CompilatorApi capi)
        {
            TextReader input = new StreamReader(capi.ResourceFile);
            var yaml = YamlNode.FromYaml(input);
            var rootNode = yaml[0] as YamlMapping;

            var resource = new Resource();

            var geometries = (YamlMapping) rootNode["geometries"];

            resource.geom_name = new long[geometries.Count];
            resource.normal_enabled = new bool[geometries.Count];
            resource.texcoord_enabled = new bool[geometries.Count];
            resource.tangent_enable = new bool[geometries.Count];
            resource.bitangent_enabled = new bool[geometries.Count];

            var geom_idx = 0;
            foreach (var geom in geometries)
            {
                var name = ((YamlScalar)geom.Key).Value;
                resource.geom_name[geom_idx] = StringId.FromString(name);

                var geom_data = (YamlMapping)geom.Value;

                var indices = (YamlMapping)geom_data["indices"];

                var position = (YamlSequence)geom_data["position"];
                var position_indices = (YamlSequence)indices["position"];
                var vertex_count = position_indices.Count;

                var normal_enabled = geom_data.ContainsKey("normal");
                var texcoord_enabled = geom_data.ContainsKey("texcoord");
                var tangent_enabled = geom_data.ContainsKey("tangent");
                var bitangent_enabled = geom_data.ContainsKey("bitangent");

                resource.normal_enabled[geom_idx] = normal_enabled;
                resource.texcoord_enabled[geom_idx] = texcoord_enabled;
                resource.tangent_enable[geom_idx] = tangent_enabled;
                resource.bitangent_enabled[geom_idx] = bitangent_enabled;

                var float3_size = (sizeof (float)*3);
                var float2_size = (sizeof (float)*2);

                var vertex_size = float3_size + // Position
                                  (normal_enabled ? float3_size : 0)   + // normal
                                  (texcoord_enabled ? float2_size : 0) + // texcoord
                                  (tangent_enabled ? float3_size: 0)   + // tangent
                                  (bitangent_enabled ? float3_size: 0) + // bitangent
                                  0;

                var ib = new short[vertex_count];
                var vb = new byte[vertex_size*vertex_count];
                var write_idx = 0;

                var wm = Quatf.ToMat4F(Quatf.FromEurelAngle(0.0f, 45 * Mathf.ToRad, 0.0f));

                for (int i = 0; i < vertex_count; i++)
                {
                    ib[i] = (short) i;

                    var idx = YamlToShort(position_indices[i]) * 3;

                    var pos = new Vec3f(YamlToFloat(position[idx + 0]),
                                        YamlToFloat(position[idx + 1]),
                                        YamlToFloat(position[idx + 2]));
                    //pos = wm * pos;

                    // position
                    write_idx += WriteFloat(vb, write_idx, pos.X);
                    write_idx += WriteFloat(vb, write_idx, pos.Y);
                    write_idx += WriteFloat(vb, write_idx, pos.Z);

                    // normal
                    if (normal_enabled)
                    {
                        var normal = (YamlSequence)geom_data["normal"];
                        var normal_indices = (YamlSequence)indices["normal"];
                        idx = (short) (YamlToShort(normal_indices[i]) * 3);

                        write_idx += WriteFloat(vb, write_idx, YamlToFloat(normal[idx + 0]));
                        write_idx += WriteFloat(vb, write_idx, YamlToFloat(normal[idx + 1]));
                        write_idx += WriteFloat(vb, write_idx, YamlToFloat(normal[idx + 2]));
                    }

                    // texcoord
                    if (texcoord_enabled)
                    {
                        var texcoord = (YamlSequence)geom_data["texcoord"];
                        var texcoord_indices = (YamlSequence)indices["texcoord"];
                        idx = (short) (YamlToShort(texcoord_indices[i]) * 2);

                        write_idx += WriteFloat(vb, write_idx, YamlToFloat(texcoord[idx + 0]));
                        write_idx += WriteFloat(vb, write_idx, YamlToFloat(texcoord[idx + 1]));
                    }
                    
                    // tangent
                    if (tangent_enabled)
                    {
                        var tangent = (YamlSequence)geom_data["tangent"];
                        var tangent_indices = (YamlSequence)indices["tangent"];
                        idx = (short) (YamlToShort(tangent_indices[i]) * 3);

                        write_idx += WriteFloat(vb, write_idx, YamlToFloat(tangent[idx + 0]));
                        write_idx += WriteFloat(vb, write_idx, YamlToFloat(tangent[idx + 1]));
                    }

                    // bitangent
                    if (bitangent_enabled)
                    {
                        var bitangent = (YamlSequence)geom_data["bitangent"];
                        var bitangent_indices = (YamlSequence)indices["bitangent"];
                        idx = (short) (YamlToShort(bitangent_indices[i]) * 3);

                        write_idx += WriteFloat(vb, write_idx, YamlToFloat(bitangent[idx + 0]));
                        write_idx += WriteFloat(vb, write_idx, YamlToFloat(bitangent[idx + 1]));
                    }
                }

                resource.geom_ib.Add(ib);
                resource.geom_vb.Add(vb);
                ++geom_idx;
            }

            var serializer = MessagePackSerializer.Get<Resource>();
            serializer.Pack(capi.BuildFile, resource);
        }

#endif
    }
}
