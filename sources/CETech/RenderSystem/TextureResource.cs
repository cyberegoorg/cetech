using System.Diagnostics;
using System.IO;
using CETech.Develop;
using CETech.Utils;
using SharpBgfx;
using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;

namespace CETech
{
    /// <summary>
    ///     Package resource
    /// </summary>
    public class TextureResource
    {
        /// <summary>
        ///     Resource type
        /// </summary>
        public static readonly long Type = StringId.FromString("texture");

        /// <summary>
        ///     Resource loader
        /// </summary>
        /// <param name="input">Resource data stream</param>
        /// <returns>Resource data</returns>
        public static object ResourceLoader(Stream input)
        {
            using (var ms = new MemoryStream())
            {
                input.CopyTo(ms);
                return new Resource() {data = ms.ToArray()};
            }
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
            var resource = (Resource) data;
            var mem = MemoryBlock.FromArray(resource.data);
            resource.texture = Texture.FromFile(mem);
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
            var old = ResourceManager.Get<Resource>(Type, name);
            var neww = (Resource) new_data;

            old.data = neww.data;
            var mem = MemoryBlock.FromArray(old.data);
            old.texture = Texture.FromFile(mem);

            return old;
        }

        public class Resource
        {
            public byte[] data;
            public Texture texture;
        }

#if CETECH_DEVELOP

        public class TextureYaml
        {
            public string input { get; set; }

            [YamlAlias("gen_mipmaps")]
            public bool gen_mipmaps { get; set; }

            [YamlAlias("is_normalmap")]
            public bool is_normalmap { get; set; }
        }

        /// <summary>
        ///     Resource compiler
        /// </summary>
        /// <param name="capi">Compiler api</param>
        public static void Compile(ResourceCompiler.CompilatorApi capi)
        {
            var build_dir = FileSystem.GetRootDir("build");

            TextReader input = new StreamReader(capi.ResourceFile);

            var deserializer = new Deserializer(namingConvention: new CamelCaseNamingConvention());
            var texture_yaml = deserializer.Deserialize<TextureYaml>(input);
    
            var input_img = FileSystem.GetFullPath("src", texture_yaml.input);
            var output_img = Path.Combine(build_dir, "tmp", Path.GetFileName(texture_yaml.input) + ".ktx");

            capi.add_dependency(input_img);

            texturec(input_img, output_img, texture_yaml.gen_mipmaps, texture_yaml.is_normalmap);

            using (var output_stream = new FileStream(output_img, FileMode.Open))
            {
                output_stream.CopyTo(capi.BuildFile);
            };            
        }

        private static void texturec(string inputImg, string outputImg, bool gen_mipmaps, bool is_normalmap)
        {
            // Prepare the process to run
            var start = new ProcessStartInfo();
            // Enter in the command line arguments, everything you would enter after the executable name itself

            var args = string.Format("-f {0} -o {1}", inputImg, outputImg);
            if (gen_mipmaps)
            {
                args += " --mips";
            }

            if (is_normalmap)
            {
                args += " --normalmap";
            }

            start.Arguments = args;

            Log.Debug("texturec", "{0}", start.Arguments);

            // Enter the executable to run, including the complete path
            var bin_path = ConfigSystem.GetValueString("resource_compiler.bin");
            start.FileName = Path.Combine(bin_path, "texturec.exe");
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

                Log.Debug("texture_compiler", "{0}", proc.StandardOutput.ReadToEnd());

                if (exitCode != 0)
                {
                    Log.Error("texture_compiler", "{0}", proc.StandardError.ReadToEnd());
                }
            }
        }
#endif
    }
}