using System;
using System.Collections.Generic;
using System.IO;

namespace CETech
{
    public static partial class FileSystem
    {
        private static readonly Dictionary<string, string> RootDirMap = new Dictionary<string, string>();

        private static void MapRootDirImpl(string root, string dir)
        {
            RootDirMap[root] = dir;
        }

        private static string GetRootDirImpl(string root)
        {
            return RootDirMap[root];
        }

        private static Stream OpenImpl(string root, string path, OpenMode mode)
        {
            return new FileStream(GetFullPath(root, path), ToFileFileMode(mode),
                mode == OpenMode.Read ? FileAccess.Read : FileAccess.Write, FileShare.Read);
        }

        private static void ListDirectoryImpl(string root, string path, string filter, out string[] files)
        {
            files = Directory.GetFiles(GetFullPath(root, path), filter, SearchOption.AllDirectories);
        }

        private static void CreateDirectoryImpl(string root, string path)
        {
            Directory.CreateDirectory(GetFullPath(root, path));
        }

        private static DateTime GetFileMTimeImpl(string root, string path)
        {
            return File.GetLastWriteTimeUtc(GetFullPath(root, path));
        }

        private static string GetFullPathImpl(string root, string path)
        {
            return path == null ? RootDirMap[root] : Path.Combine(RootDirMap[root], path);
        }

        private static FileMode ToFileFileMode(OpenMode mode)
        {
            switch (mode)
            {
                case OpenMode.Read:
                    return FileMode.Open;

                case OpenMode.Write:
                    return FileMode.Create;

                default:
                    throw new ArgumentOutOfRangeException("mode", mode, null);
            }
        }
    }
}