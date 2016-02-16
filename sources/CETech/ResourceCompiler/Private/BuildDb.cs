using System;
using System.Data.SQLite;
using System.IO;

// TODO: speed?

namespace CETech
{
    public static class BuildDb
    {
        private static readonly string dbPath = string.Format("Data Source={0}; version=3;",
            Path.Combine(FileSystem.RootDir("build"), "build.db"));


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
                        cmd.Parameters.AddWithValue("@mtime", mtime);
                        cmd.ExecuteNonQuery();
                    }
                    transaction.Commit();
                }
                c.Close();
            }
        }

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

                                var actual_mtime = FileSystem.FileMTime(root, rdr.GetString(0));
                                var last_mtime = rdr.GetDateTime(1).ToUniversalTime();

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