using System.Diagnostics;
using System.IO;
using CETech.Develop;
using CETech.Utils;
using Mono.Options;
using SharpBgfx;
using YamlDotNet.RepresentationModel;

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
            resource.texture = Texture.FromFile(mem, TextureFlags.None, 0);
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
            old.texture = Texture.FromFile(mem, TextureFlags.None, 0);

            return old;
        }

        public class Resource
        {
            public byte[] data;
            public Texture texture;
        }

#if CETECH_DEVELOP

        /// <summary>
        ///     Resource compiler
        /// </summary>
        /// <param name="capi">Compiler api</param>
        public static void Compile(ResourceCompiler.CompilatorApi capi)
        {
            var build_dir = FileSystem.GetRootDir("build");

            TextReader input = new StreamReader(capi.ResourceFile);
            var yaml = new YamlStream();
            yaml.Load(input);

            var rootNode = yaml.Documents[0].RootNode as YamlMappingNode;
            var input_path = ((YamlScalarNode) rootNode.Children[new YamlScalarNode("input")]).Value;
            var input_img = FileSystem.GetFullPath("src", input_path);

            var output_img = Path.Combine(build_dir, "tmp", Path.GetFileName(input_path) + ".ktx");

            capi.add_dependency(input_img);

            texturec(input_img, output_img);

            var output_stream = new FileStream(output_img, FileMode.Open);
            output_stream.CopyTo(capi.BuildFile);
        }

        private static void texturec(string inputImg, string outputImg)
        {
            // Prepare the process to run
            var start = new ProcessStartInfo();
            // Enter in the command line arguments, everything you would enter after the executable name itself
            start.Arguments = string.Format("-f {0} -o {1}", inputImg, outputImg);

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