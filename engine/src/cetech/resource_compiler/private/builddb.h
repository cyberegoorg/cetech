#pragma once

#include "celib/log/log.h"

#include "sqlite/sqlite3.h"

namespace cetech {
    struct BuildDB {
        BuildDB() {};

        ~BuildDB() {
            close();
        };

        void open(const char* db_path) {
            sqlite3_open(db_path, &db);
        }

        void close() {
            sqlite3_close(db);
        }

        bool _exec(const char* sql, int (* callback)(void*, int, char**, char**), void* data) {
            char* err_msg = 0;
            bool run = true;
            int rc = 0;

            do {
                rc = sqlite3_exec(db, sql, callback, data, &err_msg);

                switch (rc) {
                case SQLITE_BUSY:
                    continue;

                case SQLITE_OK:
                    return true;

                default:
                    log::error("BuildDB", "SQL error (%d): %s", rc, err_msg);
                    sqlite3_free(err_msg);
                    return false;
                }
            } while (run);

            return true;
        }


        void set_file(const char* filename, time_t mtime) {
            char* sql = sqlite3_mprintf("INSERT OR REPLACE INTO files VALUES(NULL, '%q', %d);\n", filename, mtime);

            _exec(sql, 0, 0);
            sqlite3_free(sql);
        }

        void set_file_depend(const char* filename, const char* depend_on) {
            char* sql = sqlite3_mprintf(
                "INSERT INTO file_dependency (filename, depend_on) SELECT '%q', '%q' WHERE NOT EXISTS(SELECT 1 FROM file_dependency WHERE filename = '%q' AND depend_on = '%q')",
                filename,
                depend_on,
                filename,
                depend_on);

            _exec(sql, 0, 0);

            sqlite3_free(sql);
        }

        bool need_compile(const char* filename, FileSystem* source_fs) {
            char* sql = sqlite3_mprintf(
                "SELECT\n"
                "    file_dependency.depend_on, files.mtime\n"
                "FROM\n"
                "    file_dependency\n"
                "JOIN\n"
                "    files on files.filename == file_dependency.depend_on\n"
                "WHERE\n"
                "    file_dependency.filename = '%q'\n", filename);


            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

            int rc = 0;
            bool compile = true;
            while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
                compile = false;

                time_t actual_mtime = source_fs->file_mtime((const char*)sqlite3_column_text(stmt, 0));
                time_t last_mtime = sqlite3_column_int64(stmt, 1);

                if (actual_mtime != last_mtime) {
                    compile = true;
                    break;
                }
            }

            sqlite3_free(sql);
            sqlite3_finalize(stmt);

            return compile;
        }

        bool init_db() {
            const char* sql;

            // Create files table
            sql = "CREATE TABLE IF NOT EXISTS files ("
                  "id       INTEGER PRIMARY KEY    AUTOINCREMENT    NOT NULL,"
                  "filename TEXT    UNIQUE                          NOT NULL,"
                  "mtime    INTEGER                                 NOT NULL"
                  ");";

            if (!_exec(sql, 0, 0)) {
                return false;
            }

            // Create file_dependency table
            sql = "CREATE TABLE IF NOT EXISTS file_dependency ("
                  "id        INTEGER PRIMARY KEY    AUTOINCREMENT    NOT NULL,"
                  "filename  TEXT                                    NOT NULL,"
                  "depend_on TEXT                                    NOT NULL"
                  ");";

            if (!_exec(sql, 0, 0)) {
                return false;
            }

            return true;
        }

        sqlite3* db;
    };
}