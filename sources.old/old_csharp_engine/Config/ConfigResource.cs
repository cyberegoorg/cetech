using System;
using System.IO;
using System.Yaml;
using CETech.Develop;
using CETech.Resource;

namespace CETech
{
    /// <summary>
    ///     Package resource
    /// </summary>
    public class ConfigResource
    {
        /// <summary>
        ///     Resource type
        /// </summary>
        public static readonly long Type = StringId64.FromString("config");

#if CETECH_DEVELOP

        /// <summary>
        ///     Resource compiler
        /// </summary>
        /// <param name="capi">Compiler api</param>
        public static void Compile(ResourceCompiler.CompilatorApi capi)
        {
            capi.ResourceFile.CopyTo(capi.BuildFile);
        }
#endif

        /// <summary>
        ///     Resource loader
        /// </summary>
        /// <param name="input">Resource data stream</param>
        /// <returns>Resource data</returns>
        public static object ResourceLoader(Stream input)
        {
            var ms = new MemoryStream();
            input.CopyTo(ms);
            return ms;
        }

        /// <summary>
        ///     Resource offline.
        /// </summary>
        /// <param name="data">Data</param>
        public static void ResourceOffline(object data)
        {
        }

        private static void _LoadConfig(YamlMapping root_node, string root_string)
        {
            foreach (var type in root_node)
            {
                var typestr = ((YamlScalar) type.Key).Value;

                string new_root_str;
                if (root_string.Length > 0)
                {
                    new_root_str = string.Format("{0}.{1}", root_string, typestr);
                }
                else
                {
                    new_root_str = typestr;
                }

                var t = type.Value.GetType();
                if (t == typeof (YamlMapping))
                {
                    _LoadConfig(type.Value as YamlMapping, new_root_str);
                }
                else if (t == typeof (YamlScalar))
                {
                    var s = type.Value as YamlScalar;

                    switch (ConfigSystem.GetValueType(new_root_str))
                    {
                        case ConfigSystem.ConfigValueType.Int:
                            ConfigSystem.SetValue(new_root_str, int.Parse(s.Value));
                            break;
                        case ConfigSystem.ConfigValueType.Float:
                            ConfigSystem.SetValue(new_root_str, float.Parse(s.Value));
                            break;
                        case ConfigSystem.ConfigValueType.String:
                            ConfigSystem.SetValue(new_root_str, s.Value);
                            break;
                        default:
                            throw new ArgumentOutOfRangeException();
                    }
                }
            }
        }

        /// <summary>
        ///     Resource online
        /// </summary>
        /// <param name="data">Data</param>
        public static void ResourceOnline(object data)
        {
            ((MemoryStream) data).Seek(0, SeekOrigin.Begin);
            TextReader input = new StreamReader((MemoryStream) data);

            var yaml = YamlNode.FromYaml(input);

            var rootNode = yaml[0] as YamlMapping;
            _LoadConfig(rootNode, "");
        }

        /// <summary>
        ///     Resource unloader
        /// </summary>
        /// <param name="data">data</param>
        public static void ResourceUnloader(object data)
        {
        }

        public static object Reloader(long name, object new_data)
        {
            return new_data;
        }
    }
}