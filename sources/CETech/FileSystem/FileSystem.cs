using System;
using System.IO;

namespace CETech
{
    /// <summary>
    ///     File system
    /// </summary>
    public static partial class FileSystem
    {
        /// <summary>
        ///     File open mode
        /// </summary>
        public enum OpenMode
        {
            /// <summary>
            ///     Read
            /// </summary>
            /// d
            Read = 0,

            /// <summary>
            ///     Write
            /// </summary>
            Write = 1
        }

        /// <summary>
        ///     Map root to dir
        /// </summary>
        /// <param name="root">root name</param>
        /// <param name="dir">dir path</param>
        public static void MapRootDir(string root, string dir)
        {
            MapRootDirImpl(root, dir);
        }

        /// <summary>
        ///     Get root directory path
        /// </summary>
        /// <param name="root">root name</param>
        /// <returns></returns>
        public static string GetRootDir(string root)
        {
            return GetRootDirImpl(root);
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
            return OpenImpl(root, path, mode);
        }

        /// <summary>
        ///     List directory.
        /// </summary>
        /// <param name="root">Root name</param>
        /// <param name="path">Directory path</param>
        /// <param name="files">Files in directory</param>
        public static void ListDirectory(string root, string path, out string[] files)
        {
            ListDirectoryImpl(root, path, out files);
        }

        /// <summary>
        ///     Create directory
        /// </summary>
        /// <param name="root">Root name</param>
        /// <param name="path">Directory path</param>
        public static void CreateDirectory(string root, string path)
        {
            CreateDirectoryImpl(root, path);
        }

        /// <summary>
        ///     Get last modified time.
        /// </summary>
        /// <param name="root">Root name</param>
        /// <param name="path">Path</param>
        /// <returns></returns>
        public static DateTime GetFileMTime(string root, string path)
        {
            return GetFileMTimeImpl(root, path);
        }

        /// <summary>
        ///     Get full path.
        /// </summary>
        /// <param name="root">Root name</param>
        /// <param name="path">Path</param>
        /// <returns></returns>
        public static string GetFullPath(string root, string path)
        {
            return GetFullPathImpl(root, path);
        }
    }
}