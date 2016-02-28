using System;
using System.Collections.Generic;
using System.IO;

namespace CETech
{
    /// <summary>
    ///     File system
    /// </summary>
    public static class FileSystem
    {
        /// <summary>
        ///     File open mode
        /// </summary>
        public enum OpenMode
        {
            /// <summary>
            ///     Read
            /// </summary>
            Read = 0,

            /// <summary>
            ///     Write
            /// </summary>
            Write = 1
        }

        private static readonly Dictionary<string, string> RootDirMap = new Dictionary<string, string>();

        /// <summary>
        ///     Map root to dir
        /// </summary>
        /// <param name="root">root name</param>
        /// <param name="dir">dir path</param>
        public static void MapRootDir(string root, string dir)
        {
            RootDirMap[root] = dir;
        }

        /// <summary>
        ///     Get root directory path
        /// </summary>
        /// <param name="root">root name</param>
        /// <returns></returns>
        public static string GetRootDir(string root)
        {
            return RootDirMap[root];
        }

        /// <summary>
        ///     Open file
        /// </summary>
        /// <param name="root">Root name</param>
        /// <param name="path">File path</param>
        /// <param name="mode">Open mode</param>
        /// <returns>Stream</returns>
        public static Stream Open(string root, string path, OpenMode mode)
        {
            return new FileStream(GetFullPath(root, path), ToFileFileMode(mode));
        }

        /// <summary>
        ///     List directory.
        /// </summary>
        /// <param name="root">Root name</param>
        /// <param name="path">Directory path</param>
        /// <param name="files">Files in directory</param>
        public static void ListDirectory(string root, string path, out string[] files)
        {
            files = Directory.GetFiles(GetFullPath(root, path), "*", SearchOption.AllDirectories);
        }

        /// <summary>
        ///     Create directory
        /// </summary>
        /// <param name="root">Root name</param>
        /// <param name="path">Directory path</param>
        public static void CreateDirectory(string root, string path)
        {
            Directory.CreateDirectory(GetFullPath(root, path));
        }

        /// <summary>
        ///     Get last modified time.
        /// </summary>
        /// <param name="root">Root name</param>
        /// <param name="path">Path</param>
        /// <returns></returns>
        public static DateTime GetFileMTime(string root, string path)
        {
            return File.GetLastWriteTimeUtc(GetFullPath(root, path));
        }

        /// <summary>
        ///     Get full path.
        /// </summary>
        /// <param name="root">Root name</param>
        /// <param name="path">Path</param>
        /// <returns></returns>
        public static string GetFullPath(string root, string path)
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