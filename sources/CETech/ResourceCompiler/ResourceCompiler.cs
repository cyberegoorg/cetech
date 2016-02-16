#if CETECH_DEVELOP

using System;
using System.Collections.Generic;
using System.IO;
using CETech.Utils;

namespace CETech
{
    public static class ResourceCompiler
    {
        public delegate void Compiler(CompilatorApi capi);

        private static readonly Dictionary<long, Compiler> _compoilerMap = new Dictionary<long, Compiler>();

        public static void RegisterCompiler(long type, Compiler compiler)
        {
            _compoilerMap[type] = compiler;
        }

        public static void CompileAll()
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

            Log.Info("compile_task", "Compile {0} => {1:x}{2:x}", task.filename, task.type, task.name);

            using (var input = FileSystem.Open(task.source_fs, task.filename, FileSystem.OpenMode.Read))
            {
                using (
                    var build = FileSystem.Open("build", string.Format("{0:x}{1:x}", task.type, task.name),
                        FileSystem.OpenMode.Write))
                {
                    var capi = new CompilatorApi(task.filename, input, build);

                    task.compiler(capi);

                    BuildDb.set_file(task.filename, FileSystem.FileMTime(task.source_fs, task.filename));
                    BuildDb.set_file_depend(task.filename, task.filename);

                }
            }

            Log.Info("compile_task", "{0} compiled", task.filename);
        }

        private static void CompileRoot(string root)
        {
            string[] files;
            FileSystem.ListDirectory(root, "", out files);

            var topCompileTask = TaskManager.AddNull("compiler");
           

            int[] tasks = {0};
            for (var i = 0; i < files.Length; i++)
            {
                var filename = files[i].Remove(0, FileSystem.RootDir(root).Length + 1);

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
                    compiler = compiler,
                    filename = filename,
                    name = name,
                    type = type,
                    source_fs = root
                };

                tasks[0] = TaskManager.AddBegin("compile_task", compile_task, task, parent: topCompileTask);
                TaskManager.AddEnd(tasks);
            }

            tasks[0] = topCompileTask;
            TaskManager.AddEnd(tasks);
            TaskManager.Wait(topCompileTask);
        }

        public class CompilatorApi
        {
            public Stream BuildFile;
            public string Filename;
            public Stream ResourceFile;

            public CompilatorApi(string filename, Stream resourceFile, Stream buildFile)
            {
                Filename = filename;
                ResourceFile = resourceFile;
                BuildFile = buildFile;
            }
        }

        internal struct CompileTask
        {
            public Compiler compiler;
            public string source_fs;

            public string filename;
            public long name;
            public long type;
        }
    }
}

#endif