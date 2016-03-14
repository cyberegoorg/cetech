//#if CETECH_DEVELOP

using System;
using System.Collections.Generic;
using System.IO;
using CETech.Develop;
using CETech.Utils;

namespace CETech
{
    /// <summary>
    ///     Resources compiler
    /// </summary>
    public static partial class ResourceCompiler
    {
        /// <summary>
        ///     Compiler delegate
        /// </summary>
        /// <param name="capi">Compiler api</param>
        public delegate void Compiler(CompilatorApi capi);


        public static void Init()
        {
            InitImpl();
        }

        public static void Shutdown()
        {
            ShutdownImpl();
        }


        /// <summary>
        ///     Register compiler for type
        /// </summary>
        /// <param name="type">Type</param>
        /// <param name="compiler">Compiler delegate</param>
        public static void RegisterCompiler(long type, Compiler compiler)
        {
            RegisterCompilerImpl(type, compiler);
        }

        /// <summary>
        ///     Compile all resource
        /// </summary>
        public static void CompileAll()
        {
            CompileAllImpl();
        }

        /// <summary>
        ///     Compilator api
        /// </summary>
        public class CompilatorApi
        {
            private readonly string _sourceRoot;

            /// <summary>
            ///     Compiled resource stream
            /// </summary>
            public readonly Stream BuildFile;

            /// <summary>
            ///     Source stream
            /// </summary>
            public readonly Stream ResourceFile;

            /// <summary>
            ///     Source filename
            /// </summary>
            public string Filename;

            /// <summary>
            ///     Create compilator api.
            /// </summary>
            /// <param name="filename">Source filename</param>
            /// <param name="resourceFile">Source stream</param>
            /// <param name="buildFile">Compiled resource stream</param>
            /// <param name="sourceRoot">Source file root</param>
            public CompilatorApi(string filename, Stream resourceFile, Stream buildFile, string sourceRoot)
            {
                Filename = filename;
                ResourceFile = resourceFile;
                BuildFile = buildFile;
                _sourceRoot = sourceRoot;
            }

            /// <summary>
            ///     Add resource dependency
            /// </summary>
            /// <param name="path"></param>
            public void add_dependency(string path)
            {
                BuildDb.set_file(path, FileSystem.GetFileMTime(_sourceRoot, path));
                BuildDb.set_file_depend(Filename, path);
            }
        }
    }
}

//#endif