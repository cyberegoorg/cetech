using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Yaml;
using CETech.Develop;
using CETech.Resource;
using MsgPack.Serialization;
using SharpBgfx;

namespace CETech
{
    /// <summary>
    ///     Scene resource
    /// </summary>
    public class SceneResource
    {
        [Flags]
        public enum ChanelName
        {
            Null = 0,
            Position = 1 << 0,
            Normal = 1 << 1,
            Tangent = 1 << 2,
            Bitangent = 1 << 3,
            Color0 = 1 << 4,
            Color1 = 1 << 5,
            Indices = 1 << 6,
            Weight = 1 << 7,
            TexCoord0 = 1 << 8,
            TexCoord1 = 1 << 9,
            TexCoord2 = 1 << 10,
            TexCoord3 = 1 << 11,
            TexCoord4 = 1 << 12,
            TexCoord5 = 1 << 13,
            TexCoord6 = 1 << 14,
            TexCoord7 = 1 << 15
        }

        public enum StreamType
        {
            Null = 0,
            Vec2,
            Vec3
        }

        /// <summary>
        ///     Resource Name
        /// </summary>
        public static readonly long Type = StringId64.FromString("scene");

        private static readonly ChanelDef[] ChanelDefs =
        {
            new ChanelDef("position", ChanelName.Position, VertexAttributeUsage.Position),
            new ChanelDef("normal", ChanelName.Normal, VertexAttributeUsage.Normal),
            new ChanelDef("tangent", ChanelName.Tangent, VertexAttributeUsage.Tangent),
            new ChanelDef("bitangent", ChanelName.Bitangent, VertexAttributeUsage.Bitangent),
            new ChanelDef("color0", ChanelName.Color0, VertexAttributeUsage.Color0),
            new ChanelDef("color1", ChanelName.Color1, VertexAttributeUsage.Color1),
            new ChanelDef("indices", ChanelName.Indices, VertexAttributeUsage.Indices),
            new ChanelDef("weight", ChanelName.Weight, VertexAttributeUsage.Weight),
            new ChanelDef("texcoord0", ChanelName.TexCoord0, VertexAttributeUsage.TexCoord0),
            new ChanelDef("texcoord1", ChanelName.TexCoord1, VertexAttributeUsage.TexCoord1),
            new ChanelDef("texcoord2", ChanelName.TexCoord2, VertexAttributeUsage.TexCoord2),
            new ChanelDef("texcoord3", ChanelName.TexCoord3, VertexAttributeUsage.TexCoord3),
            new ChanelDef("texcoord4", ChanelName.TexCoord4, VertexAttributeUsage.TexCoord4),
            new ChanelDef("texcoord5", ChanelName.TexCoord5, VertexAttributeUsage.TexCoord5),
            new ChanelDef("texcoord6", ChanelName.TexCoord6, VertexAttributeUsage.TexCoord6),
            new ChanelDef("texcoord7", ChanelName.TexCoord7, VertexAttributeUsage.TexCoord7)
        };

        private static readonly StreamDef[] StreamDefs =
        {
            new StreamDef("vec2", StreamType.Vec2, VertexAttributeType.Float, sizeof (float)*2, 2),
            new StreamDef("vec3", StreamType.Vec3, VertexAttributeType.Float, sizeof (float)*3, 3)
        };

        /// <summary>
        ///     Resource loader
        /// </summary>
        /// <param name="input">Resource data stream</param>
        /// <returns>Resource data</returns>
        public static object ResourceLoader(Stream input)
        {
            var serializer = MessagePackSerializer.Get<Resource>();
            var resource = serializer.Unpack(input);
            return new SceneInstance {resource = resource};
        }

        /// <summary>
        ///     Resource offline.
        /// </summary>
        /// <param name="data">Data</param>
        public static void ResourceOffline(object data)
        {
        }

        public static void AddToLayout(VertexLayout layout, VertexAttributeUsage usage, StreamType type)
        {
            var sdef = StreamDefFromType(type);

            layout.Add(usage, sdef.count, sdef.attr_type);
        }

        /// <summary>
        ///     Resource online
        /// </summary>
        /// <param name="data">Data</param>
        public static void ResourceOnline(object data)
        {
            var instance = (SceneInstance) data;
            var resource = instance.resource;
            var mesh_count = resource.geom_name.Length;

            instance.ib = new IndexBuffer[resource.geom_name.Length];
            instance.vb = new VertexBuffer[resource.geom_name.Length];
            instance.vl = new VertexLayout[resource.geom_name.Length];

            for (var i = 0; i < mesh_count; i++)
            {
                instance.vl[i] = new VertexLayout().Begin();
                var layout = instance.vl[i];
                var stypes = resource.stypes[i];
                var stypes_idx = 0;


                for (var j = 0; j < ChanelDefs.Length; j++)
                {
                    var chanel_def = ChanelDefs[j];
                    if (resource.types[i].HasFlag(chanel_def.Name))
                    {
                        var stype = stypes[stypes_idx++];
                        AddToLayout(layout, chanel_def.usage, stype);
                    }
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

        private static StreamDef StreamDefFromString(string str)
        {
            for (var i = 0; i < StreamDefs.Length; i++)
            {
                if (StreamDefs[i].name != str)
                {
                    continue;
                }

                return StreamDefs[i];
            }
            return new StreamDef();
        }

        private static StreamDef StreamDefFromType(StreamType type)
        {
            for (var i = 0; i < StreamDefs.Length; i++)
            {
                if (StreamDefs[i].type != type)
                {
                    continue;
                }

                return StreamDefs[i];
            }
            return new StreamDef();
        }

        public class SceneInstance
        {
            public IndexBuffer[] ib;
            public Resource resource;
            public VertexBuffer[] vb;
            public VertexLayout[] vl;
        }

        public class Resource
        {
            public List<short[]> geom_ib = new List<short[]>();
            public long[] geom_name;
            public List<byte[]> geom_vb = new List<byte[]>();
            public List<float[]> node_local = new List<float[]>();

            public List<long[]> node_names = new List<long[]>();
            public List<int[]> node_parent = new List<int[]>();
            public List<List<StreamType>> stypes = new List<List<StreamType>>();
            public List<ChanelName> types = new List<ChanelName>();
        }

        private struct ChanelDef
        {
            public readonly string name;
            public readonly ChanelName Name;
            public readonly VertexAttributeUsage usage;

            public ChanelDef(string name, ChanelName Name, VertexAttributeUsage usage)
            {
                this.name = name;
                this.Name = Name;
                this.usage = usage;
            }
        }

        private struct StreamDef
        {
            public readonly string name;
            public readonly StreamType type;
            public readonly VertexAttributeType attr_type;
            public readonly int size;
            public readonly int count;

            public StreamDef(string name, StreamType type, VertexAttributeType attr_type, int size, int count)
            {
                this.name = name;
                this.type = type;
                this.attr_type = attr_type;
                this.size = size;
                this.count = count;
            }
        }

#if CETECH_DEVELOP
        private static int WriteFloat(byte[] bytes, int idx, float value)
        {
            var byteArray = BitConverter.GetBytes(value);
            for (var i = 0; i < byteArray.Length; i++)
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
            return float.Parse(((YamlScalar) node).Value, CultureInfo.InvariantCulture);
        }

        private static int WriteChanel(int write_idx, byte[] output, int vidx, string name, YamlMapping geom_data,
            YamlMapping indices, float size)
        {
            var data = (YamlSequence) geom_data[name];
            var indices_data = (YamlSequence) indices[name];

            var idx = (short) (YamlToShort(indices_data[vidx])*size);


            write_idx += WriteFloat(output, write_idx, YamlToFloat(data[idx + 0]));

            if (size > 1)
            {
                write_idx += WriteFloat(output, write_idx, YamlToFloat(data[idx + 1]));
            }

            if (size > 2)
            {
                write_idx += WriteFloat(output, write_idx, YamlToFloat(data[idx + 2]));
            }

            return write_idx;
        }

        public static void compile_node(KeyValuePair<YamlNode, YamlNode> rootNode, ref int node_id, int parent,
            Dictionary<long, long> node_parent, Dictionary<long, float[]> local_pose, Dictionary<long, long> names)
        {
            node_parent[node_id] = parent;

            var name = rootNode.Key as YamlScalar;
            var body = rootNode.Value as YamlMapping;
            var local = body["local"] as YamlSequence;

            var local_mat = new float[16];
            for (var i = 0; i < 16; i++)
            {
                local_mat[i] = YamlToFloat(local[i]);
            }

            var name_id = StringId64.FromString(name.Value);

            names[node_id] = name_id;
            node_parent[node_id] = parent;
            local_pose[node_id] = local_mat;

            if (body.ContainsKey("children"))
            {
                var parent_node = node_id;
                var childrenNode = body["children"] as YamlMapping;

                foreach (var child in childrenNode)
                {
                    node_id += 1;
                    compile_node(child, ref node_id, parent_node,
                        node_parent, local_pose, names);
                }
            }
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
            var geom_idx = 0;
            foreach (var geom in geometries)
            {
                var name = ((YamlScalar) geom.Key).Value;
                resource.geom_name[geom_idx] = StringId64.FromString(name);

                var geom_data = (YamlMapping) geom.Value;

                var indices = (YamlMapping) geom_data["indices"];
                var types = (YamlMapping) geom_data["types"];
                var chanels = (YamlMapping) geom_data["chanels"];
                var vertex_count = int.Parse(((YamlScalar) indices["size"]).Value);

                var vertex_size = 0;
                var enabled = ChanelName.Null;
                var stypes = new List<StreamType>();
                for (var i = 0; i < ChanelDefs.Length; i++)
                {
                    var item = ChanelDefs[i];

                    if (chanels.ContainsKey(item.name))
                    {
                        enabled |= item.Name;

                        var stream_type = ((YamlScalar) types[item.name]).Value;
                        var sdef = StreamDefFromString(stream_type);
                        vertex_size += sdef.size;

                        stypes.Add(sdef.type);
                    }
                }
                resource.stypes.Add(stypes);

                var write_idx = 0;
                var ib = new short[vertex_count];
                var vb = new byte[vertex_size*vertex_count];

                for (var i = 0; i < vertex_count; i++)
                {
                    ib[i] = (short) i;

                    for (var j = 0; j < ChanelDefs.Length; j++)
                    {
                        var item = ChanelDefs[j];

                        if (enabled.HasFlag(item.Name))
                        {
                            var stream_type = ((YamlScalar) types[item.name]).Value;
                            var sdef = StreamDefFromString(stream_type);

                            write_idx = WriteChanel(write_idx, vb, i, item.name, chanels, indices, sdef.count);
                        }
                    }
                }

                resource.geom_ib.Add(ib);
                resource.geom_vb.Add(vb);
                resource.types.Add(enabled);
                ++geom_idx;
            }

            var graphs = (YamlMapping) rootNode["graph"];
            var node_id = 0;
            var node_parent = new Dictionary<long, long>();
            var local_pose = new Dictionary<long, float[]>();
            var names = new Dictionary<long, long>();


            var tmp_node_parent = new List<int>();
            var tmp_local_pose = new List<float>();
            var tmp_names = new List<long>();

            foreach (var graph in graphs)
            {
                node_id = 0;

                node_parent.Clear();
                local_pose.Clear();
                names.Clear();
                tmp_node_parent.Clear();
                tmp_local_pose.Clear();
                tmp_names.Clear();

                compile_node(graph, ref node_id, int.MaxValue, node_parent, local_pose, names);

                var node_count = node_id + 1;

                for (var i = 0; i < node_count; i++)
                {
                    tmp_node_parent.Add((int) node_parent[i]);

                    var pose = local_pose[i];
                    for (var j = 0; j < 16; j++)
                    {
                        tmp_local_pose.Add(pose[j]);
                    }

                    tmp_names.Add(names[i]);
                }

                resource.node_names.Add(tmp_names.ToArray());
                resource.node_local.Add(tmp_local_pose.ToArray());
                resource.node_parent.Add(tmp_node_parent.ToArray());
            }

            var serializer = MessagePackSerializer.Get<Resource>();
            serializer.Pack(capi.BuildFile, resource);
        }

#endif
    }
}