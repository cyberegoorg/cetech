using System.Diagnostics;
using System.IO;
using CETech.Develop;
using CETech.Utils;
using MsgPack.Serialization;
using SharpBgfx;
using YamlDotNet.RepresentationModel;

namespace CETech
{
    /// <summary>
    ///     Package resource
    /// </summary>
    public class ShaderResource
    {
        /// <summary>
        ///     Resource type
        /// </summary>
        public static readonly long Type = StringId.FromString("shader");

        /// <summary>
        ///     Resource loader
        /// </summary>
        /// <param name="input">Resource data stream</param>
        /// <returns>Resource data</returns>
        public static object ResourceLoader(Stream input)
        {
            var serializer = MessagePackSerializer.Get<Resource>();
            var resource = serializer.Unpack(input);
            return new ShaderInstance {fs_file = resource.fs_file, vs_file = resource.vs_file};
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
            var resource = (ShaderInstance) data;

            var vs_shader = new Shader(MemoryBlock.FromArray(resource.vs_file));
            var fs_shader = new Shader(MemoryBlock.FromArray(resource.fs_file));
            resource.program = new SharpBgfx.Program(vs_shader, fs_shader, true);
        }

        /// <summary>
        ///     Resource unloader
        /// </summary>
        /// <param name="data">data</param>
        public static void ResourceUnloader(object data)
        {
        }

        public struct Resource
        {
            public byte[] vs_file;
            public byte[] fs_file;
        }

        public class ShaderInstance
        {
            public byte[] fs_file;
            public SharpBgfx.Program program;
            public byte[] vs_file;
        }

#if CETECH_DEVELOP

        private static void shaderc(string input, string output, string include_path, string type, string platform,
            string profile)
        {
            // Prepare the process to run
            var start = new ProcessStartInfo();
            // Enter in the command line arguments, everything you would enter after the executable name itself
            start.Arguments = string.Format("-f {0} -o {1} -i {2} --type {3} --platform {4} --profile {5}", input,
                output, include_path, type, platform, profile);

            Log.Debug("shaderc", "{0}", start.Arguments);

            // Enter the executable to run, including the complete path
            var bin_path = ConfigSystem.GetValueString("resource_compiler.bin");
            start.FileName = Path.Combine(bin_path, "shaderc.exe");
            start.RedirectStandardOutput = true;
            start.RedirectStandardError = true;
            start.UseShellExecute = false;
            // Do you want to show a console window?
            start.WindowStyle = ProcessWindowStyle.Hidden;
            start.CreateNoWindow = true;
            int exitCode;

            // Run the external process & wait for it to finish
            using (var proc = Process.Start(start))
            {
                proc.WaitForExit();

                // Retrieve the app's exit code
                exitCode = proc.ExitCode;

                Log.Debug("shader_compiler", "{0}", proc.StandardOutput.ReadToEnd());

                if (exitCode != 0)
                {
                    Log.Error("shader_compiler", "{0}", proc.StandardError.ReadToEnd());
                }
            }
        }

        /// <summary>
        ///     Resource compiler
        /// </summary>
        /// <param name="capi">Compiler api</param>
        public static void Compile(ResourceCompiler.CompilatorApi capi)
        {
            var core_dir = FileSystem.GetRootDir("core");
            var build_dir = FileSystem.GetRootDir("build");
            var src_dir = FileSystem.GetRootDir("src");

            var include_path = Path.Combine(core_dir, "bgfxshaders");


            TextReader input = new StreamReader(capi.ResourceFile);
            var yaml = new YamlStream();
            yaml.Load(input);

            var rootNode = yaml.Documents[0].RootNode as YamlMappingNode;
            var vs_input = ((YamlScalarNode) rootNode.Children[new YamlScalarNode("vs_input")]).Value;
            var fs_input = ((YamlScalarNode) rootNode.Children[new YamlScalarNode("fs_input")]).Value;

            var input_shader = Path.Combine(src_dir, vs_input);
            var output_vsshader = Path.Combine(build_dir, "tmp", vs_input + ".bin");
            shaderc(input_shader, output_vsshader, include_path, "vertex", "windows", "vs_4_0");

            input_shader = Path.Combine(src_dir, fs_input);
            var output_fsshader = Path.Combine(build_dir, "tmp", fs_input + ".bin");
            shaderc(input_shader, output_fsshader, include_path, "fragment", "windows", "ps_4_0");

            var vs_file = File.ReadAllBytes(output_vsshader);
            var fs_file = File.ReadAllBytes(output_fsshader);


            var serializer = MessagePackSerializer.Get<Resource>();
            serializer.Pack(capi.BuildFile, new Resource {vs_file = vs_file, fs_file = fs_file});

            capi.add_dependency(vs_input);
            capi.add_dependency(fs_input);
        }
#endif
    }
}