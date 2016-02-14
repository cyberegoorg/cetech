using System;
using System.Collections.Generic;
using System.IO;
using CETech.Utils;

namespace CETech
{
    public static class ResourceCompiler
    {
        public delegate void Compiler(CompilatorAPI capi);

        private static readonly Dictionary<StringId, Compiler> _compoilerMap = new Dictionary<StringId, Compiler>();

        public static void registerCompiler(StringId type, Compiler compiler)
        {
            _compoilerMap[type] = compiler;
        }

        public static void CompileAll()
        {
            FileSystem.CreateDirectory("build", null);
            Compille("src");
            Compille("core");
        }

        private static void calcHash(string filename, out StringId type, out StringId name)
        {
            var last_idx = filename.LastIndexOf(".", StringComparison.Ordinal);

            var namestr = filename.Substring(0, last_idx);
            var typestr = filename.Substring(last_idx + 1);

            type = new StringId(typestr);
            name = new StringId(namestr);
        }


        private static void compile_task(object data)
        {
            var task = (CompileTask) data;

            Log.Info("compile_task", "Compile {0} => {1}{2}", task.filename, task.type, task.name);

            using (var input = FileSystem.Open(task.source_fs, task.filename, FileSystem.OpenMode.Read))
            {
                using (
                    var build = FileSystem.Open("build", string.Format("{0}{1}", task.type, task.name),
                        FileSystem.OpenMode.Write))
                {
                    var capi = new CompilatorAPI(task.filename, input, build);

                    task.compiler(capi);
                }
            }

            Log.Info("compile_task", "{0} compiled", task.filename);
        }

        private static string ReplaceFirst(string text, string search, string replace)
        {
            var pos = text.IndexOf(search, StringComparison.Ordinal);
            if (pos < 0)
            {
                return text;
            }
            return text.Substring(0, pos) + replace + text.Substring(pos + search.Length);
        }

        private static void Compille(string root)
        {
            string[] files;
            FileSystem.ListDirectory(root, "", out files);

            var topCompileTask = TaskManager.AddNull("compiler");

            int[] tasks = {0};
            for (var i = 0; i < files.Length; i++)
            {
                var filename = files[i].Remove(0, FileSystem.RootDir(root).Length + 1);

                StringId name, type;
                calcHash(filename, out type, out name);

                Compiler compiler;
                if (!_compoilerMap.TryGetValue(type, out compiler))
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

        public class CompilatorAPI
        {
            public Stream BuildFile;
            public string Filename;
            public Stream ResourceFile;

            public CompilatorAPI(string filename, Stream resourceFile, Stream buildFile)
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
            public StringId name;
            public StringId type;
        }
    }
}