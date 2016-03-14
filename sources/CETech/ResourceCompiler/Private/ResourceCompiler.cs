//#if CETECH_DEVELOP

using System;
using System.Collections.Generic;
using CETech.Develop;
using CETech.Utils;

namespace CETech
{
    /// <summary>
    ///     Resources compiler
    /// </summary>
    public static partial class ResourceCompiler
    {
        private static readonly Dictionary<long, Compiler> _compoilerMap = new Dictionary<long, Compiler>();

        private static void InitImpl()
        {
            ConsoleServer.RegisterCommand("resource_compiler.compile_all", (args, response) => CompileAll());
        }

        private static void ShutdownImpl()
        {
        }


        /// <summary>
        ///     Register compiler for type
        /// </summary>
        /// <param name="type">Type</param>
        /// <param name="compiler">Compiler delegate</param>
        private static void RegisterCompilerImpl(long type, Compiler compiler)
        {
            _compoilerMap[type] = compiler;
        }

        /// <summary>
        ///     Compile all resource
        /// </summary>
        private static void CompileAllImpl()
        {
            FileSystem.CreateDirectory("build", null);

            BuildDb.init_db();

            CompileRoot("src");
            CompileRoot("core");
        }

        private static void CalcHash(string filename, out long type, out long name)
        {
            var last_idx = filename.LastIndexOf(".", StringComparison.Ordinal);

            var namestr = filename.Substring(0, last_idx);
            var typestr = filename.Substring(last_idx + 1);

            type = StringId.FromString(typestr);
            name = StringId.FromString(namestr);
        }


        private static void compile_task(object data)
        {
            var task = (CompileTask) data;

            Log.Info("compile_task", "Compile {0} => {1:x}{2:x}", task.Filename, task.Type, task.Name);

            using (var input = FileSystem.Open(task.SourceFs, task.Filename, FileSystem.OpenMode.Read))
            {
                using (
                    var build = FileSystem.Open("build", string.Format("{0:x}{1:x}", task.Type, task.Name),
                        FileSystem.OpenMode.Write))
                {
                    var capi = new CompilatorApi(task.Filename, input, build, task.SourceFs);

                    task.Compiler(capi);

                    BuildDb.set_file(task.Filename, FileSystem.GetFileMTime(task.SourceFs, task.Filename));
                    BuildDb.set_file_depend(task.Filename, task.Filename);
                }
            }

            Log.Info("compile_task", "{0} compiled", task.Filename);
        }

        private static void CompileRoot(string root)
        {
            string[] files;
            FileSystem.ListDirectory(root, "", out files);

            var topCompileTask = TaskManager.AddNull("compiler");


            int[] tasks = {0};
            for (var i = 0; i < files.Length; i++)
            {
                var filename = files[i].Remove(0, FileSystem.GetRootDir(root).Length + 1);

                long name, type;
                CalcHash(filename, out type, out name);

                Compiler compiler;
                if (!_compoilerMap.TryGetValue(type, out compiler))
                {
                    continue;
                }

                BuildDb.set_file_hash(filename, string.Format("{0:x}{1:x}", type, name));

                if (!BuildDb.need_compile(root, filename))
                {
                    continue;
                }

                var task = new CompileTask
                {
                    Compiler = compiler,
                    Filename = filename,
                    Name = name,
                    Type = type,
                    SourceFs = root
                };

                tasks[0] = TaskManager.AddBegin("compile_task", compile_task, task, parent: topCompileTask);
                TaskManager.AddEnd(tasks);
            }

            tasks[0] = topCompileTask;
            TaskManager.AddEnd(tasks);
            TaskManager.Wait(topCompileTask);
        }

        internal struct CompileTask
        {
            public Compiler Compiler;
            public string SourceFs;

            public string Filename;
            public long Name;
            public long Type;
        }
    }
}

//#endif