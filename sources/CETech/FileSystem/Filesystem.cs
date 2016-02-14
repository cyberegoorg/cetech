using System;
using System.Collections.Generic;
using System.IO;

namespace CETech
{
    public static class FileSystem
    {
        public enum OpenMode
        {
            Read = 0,
            Write = 1
        }

        private static readonly Dictionary<string, string> _dir_map = new Dictionary<string, string>();

        public static void MapRootDir(string root, string dir)
        {
            _dir_map[root] = dir;
        }

        public static string RootDir(string root)
        {
            return _dir_map[root];
        }

        public static Stream Open(string root, string path, OpenMode mode)
        {
            return new FileStream(get_full_path(root, path), toFileFileMode(mode));
        }

        public static void ListDirectory(string root, string path, out string[] files)
        {
            files = Directory.GetFiles(get_full_path(root, path), "*", SearchOption.AllDirectories);
        }

        public static void CreateDirectory(string root, string path)
        {
            Directory.CreateDirectory(get_full_path(root, path));
        }

        private static FileMode toFileFileMode(OpenMode mode)
        {
            switch (mode)
            {
                case OpenMode.Read:
                    return FileMode.Open;

                case OpenMode.Write:
                    return FileMode.OpenOrCreate;

                default:
                    throw new ArgumentOutOfRangeException("mode", mode, null);
            }
        }

        private static string get_full_path(string root, string path)
        {
            if (path == null)
            {
                return _dir_map[root];
            }

            return Path.Combine(_dir_map[root], path);
        }
    }
}