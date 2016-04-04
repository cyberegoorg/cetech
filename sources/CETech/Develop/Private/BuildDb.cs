using System;
using System.IO;
#if PLATFORM_WINDOWS
using System.Data.SQLite;

#elif PLATFORM_LINUX
using Mono.Data.Sqlite;
#endif

// TODO: speed?

namespace CETech.Develop.Private
{
#if PLATFORM_LINUX
	using SQLiteConnection = SqliteConnection;
	using SQLiteCommand = SqliteCommand;
	#endif

    /// <summary>
    ///     Build DB
    /// </summary>
    public static class BuildDb
    {
        private static readonly string dbPath = string.Format("Data Source={0}; version=3;",
            Path.Combine(FileSystem.GetRootDir("build"), "build.db"));


        /// <summary>
        ///     Init db
        /// </summary>
        public static void init_db()
        {
            using (var c = new SQLiteConnection(dbPath))
            {
                c.Open();

                using (var transaction = c.BeginTransaction())
                {
                    using (var cmd = new SQLiteCommand("", c))
                    {
                        cmd.CommandText = @"
                                CREATE TABLE IF NOT EXISTS files (
                                id       INTEGER PRIMARY KEY    AUTOINCREMENT    NOT NULL,
                                filename TEXT    UNIQUE                          NOT NULL,
                                mtime    INTEGER                                 NOT NULL
                            )";

                        cmd.ExecuteNonQuery();

                        cmd.CommandText = @"
                            CREATE TABLE IF NOT EXISTS file_dependency (
                            id        INTEGER PRIMARY KEY    AUTOINCREMENT    NOT NULL,
                            filename  TEXT                                    NOT NULL,
                            depend_on TEXT                                    NOT NULL
                            )";

                        cmd.ExecuteNonQuery();

                        cmd.CommandText = @"
                            CREATE TABLE IF NOT EXISTS file_hash (
                            filename  TEXT    PRIMARY KEY NOT NULL,
                            hash      TEXT    UNIQUE NOT NULL
                            )";

                        cmd.ExecuteNonQuery();
                    }
                    transaction.Commit();
                }
                c.Close();
            }
        }

        /// <summary>
        ///     Set file
        /// </summary>
        /// <param name="filename">filename</param>
        /// <param name="mtime">last modified time</param>
        public static void set_file(string filename, DateTime mtime)
        {
            using (var c = new SQLiteConnection(dbPath))
            {
                c.Open();

                using (var transaction = c.BeginTransaction())
                {
                    using (var cmd = new SQLiteCommand("", c))
                    {
                        cmd.CommandText = @"INSERT OR REPLACE INTO files VALUES(NULL, @filename, @mtime);";

                        cmd.Parameters.AddWithValue("@filename", filename);
						cmd.Parameters.AddWithValue("@mtime", mtime.ToFileTimeUtc());
                        cmd.ExecuteNonQuery();
                    }
                    transaction.Commit();
                }
                c.Close();
            }
        }

        /// <summary>
        ///     Set file depend
        /// </summary>
        /// <param name="filename">filename</param>
        /// <param name="depend_on">depend filename</param>
        public static void set_file_depend(string filename, string depend_on)
        {
            using (var c = new SQLiteConnection(dbPath))
            {
                c.Open();

                using (var transaction = c.BeginTransaction())
                {
                    using (var cmd = new SQLiteCommand("", c))
                    {
                        cmd.CommandText =
                            @"INSERT INTO file_dependency (filename, depend_on) SELECT @filename, @depend_on WHERE NOT EXISTS(SELECT 1 FROM file_dependency WHERE filename = @filename AND depend_on = @depend_on);";

                        cmd.Parameters.AddWithValue("@filename", filename);
                        cmd.Parameters.AddWithValue("@depend_on", depend_on);
                        cmd.ExecuteNonQuery();
                    }
                    transaction.Commit();
                }
                c.Close();
            }
        }

        /// <summary>
        ///     Set file hash.
        /// </summary>
        /// <param name="filename">filename</param>
        /// <param name="hash">hash</param>
        public static void set_file_hash(string filename, string hash)
        {
            using (var c = new SQLiteConnection(dbPath))
            {
                c.Open();

                using (var transaction = c.BeginTransaction())
                {
                    using (var cmd = new SQLiteCommand("", c))
                    {
                        cmd.CommandText = @"INSERT OR REPLACE INTO file_hash VALUES(@filename, @hash);";

                        cmd.Parameters.AddWithValue("@filename", filename);
                        cmd.Parameters.AddWithValue("@hash", hash);
                        cmd.ExecuteNonQuery();
                    }
                    transaction.Commit();
                }
                c.Close();
            }
        }

        /// <summary>
        ///     Need compile resource?
        /// </summary>
        /// <param name="root">filesystem root</param>
        /// <param name="filename">filename</param>
        /// <returns>True if need</returns>
        public static bool need_compile(string root, string filename)
        {
            var compile = true;

            using (var c = new SQLiteConnection(dbPath))
            {
                c.Open();

                using (var transaction = c.BeginTransaction())
                {
                    using (var cmd = new SQLiteCommand("", c))
                    {
                        cmd.CommandText = @"
                        SELECT
                            file_dependency.depend_on, files.mtime
                        FROM
                            file_dependency
                        JOIN
                            files on files.filename == file_dependency.depend_on
                        WHERE
                            file_dependency.filename = @filename
                    ";

                        cmd.Parameters.AddWithValue("@filename", filename);

                        using (var rdr = cmd.ExecuteReader())
                        {
                            while (rdr.Read())
                            {
                                compile = false;

								var actual_mtime = FileSystem.GetFileMTime(root, rdr.GetString(0)).ToFileTimeUtc();
								var last_mtime = rdr.GetInt64 (1);

                                if (actual_mtime != last_mtime)
                                {
                                    compile = true;
                                    break;
                                }
                            }
                        }
                    }
                    transaction.Commit();
                }
                c.Close();
            }

            return compile;
        }
    }
}