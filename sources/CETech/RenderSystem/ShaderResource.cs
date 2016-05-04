using System.Diagnostics;
using System.IO;
using System.Yaml;
using CETech.Develop;
using CETech.Resource;
using CETech.Utils;
using MsgPack.Serialization;
using SharpBgfx;

namespace CETech
{
    /// <summary>
    ///     Package resource
    /// </summary>
    public class ShaderResource
    {
        /// <summary>
        ///     ResourceManager type
        /// </summary>
        public static readonly long Type = StringId64.FromString("shader");

        /// <summary>
        ///     ResourceManager loader
        /// </summary>
        /// <param name="input">ResourceManager data stream</param>
        /// <returns>ResourceManager data</returns>
        public static object ResourceLoader(Stream input)
        {
            var serializer = MessagePackSerializer.Get<Resource>();
            var resource = serializer.Unpack(input);
            return new ShaderInstance {fs_file = resource.fs_file, vs_file = resource.vs_file};
        }

        /// <summary>
        ///     ResourceManager offline.
        /// </summary>
        /// <param name="data">Data</param>
        public static void ResourceOffline(object data)
        {
            var resource = (ShaderInstance) data;
            resource.program.Dispose();
        }

        /// <summary>
        ///     ResourceManager online
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
        ///     ResourceManager unloader
        /// </summary>
        /// <param name="data">data</param>
        public static void ResourceUnloader(object data)
        {
        }

        public static object Reloader(long name, object new_data)
        {
            var resource = (ShaderInstance) new_data;
            var old = CETech.Resource.ResourceManager.Get<ShaderInstance>(Type, name);

            var vs_shader = new Shader(MemoryBlock.FromArray(resource.vs_file));
            var fs_shader = new Shader(MemoryBlock.FromArray(resource.fs_file));
            old.program = new SharpBgfx.Program(vs_shader, fs_shader, true);

            return resource;
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
            public Resource resource;
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
            var bin_path = ConfigSystem.String("resource_compiler.bin");

#if PLATFORM_WINDOWS
            start.FileName = Path.Combine(bin_path, "shaderc.exe");
#else
            start.FileName = Path.Combine(bin_path, "shaderc");	
#endif

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
        ///     ResourceManager compiler
        /// </summary>
        /// <param name="capi">Compiler api</param>
        public static void Compile(ResourceCompiler.CompilatorApi capi)
        {
            var core_dir = FileSystem.GetRootDir("core");
            var build_dir = FileSystem.GetRootDir("build");
            var src_dir = FileSystem.GetRootDir("src");

            var include_path = Path.Combine(core_dir, "bgfxshaders");

            TextReader input = new StreamReader(capi.ResourceFile);
            var yaml = YamlNode.FromYaml(input);

            var rootNode = yaml[0] as YamlMapping;
            var vs_input = ((YamlScalar) rootNode["vs_input"]).Value;
            var fs_input = ((YamlScalar) rootNode["fs_input"]).Value;

            var input_shader = Path.Combine(src_dir, vs_input);
            var output_vsshader = Path.Combine(build_dir, "tmp", vs_input + ".bin");

// TODO: platform from capi
#if PLATFORM_LINUX
			shaderc(input_shader, output_vsshader, include_path, "vertex", "linux", "120");
#elif PLATFORM_MACOS
            shaderc(input_shader, output_vsshader, include_path, "vertex", "osx", "120");
#else
            shaderc(input_shader, output_vsshader, include_path, "vertex", "windows", "vs_4_0");
#endif

            input_shader = Path.Combine(src_dir, fs_input);
            var output_fsshader = Path.Combine(build_dir, "tmp", fs_input + ".bin");

#if PLATFORM_LINUX
			shaderc(input_shader, output_fsshader, include_path, "fragment", "linux", "120");
        #elif PLATFORM_MACOS
            shaderc(input_shader, output_fsshader, include_path, "fragment", "osx", "120");
		#else
            shaderc(input_shader, output_fsshader, include_path, "fragment", "windows", "ps_4_0");
#endif

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