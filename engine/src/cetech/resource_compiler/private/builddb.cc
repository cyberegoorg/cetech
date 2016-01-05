#include <cstdint>

#include "cetech/resource_compiler/private/builddb.h"

#include "celib/log/log.h"
#include "celib/defines.h"

#include "sqlite3/sqlite3.h"

#include "cetech/filesystem/filesystem.h"

namespace cetech {
    struct SQLiteSTMT {
        sqlite3* _db;
        sqlite3_stmt* _stmt;

        SQLiteSTMT(sqlite3 * db, const char* sql) : _db(db) {
            _db = db;
            sqlite3_prepare_v2(db, sql, -1, &_stmt, NULL);
        }

        ~SQLiteSTMT() {
            sqlite3_finalize(_stmt);
        }

        SQLiteSTMT& bind_text(int num,
                              const char* text) {
            sqlite3_bind_text(_stmt, num, text, -1, SQLITE_TRANSIENT);
            return *this;
        }

        SQLiteSTMT& bind_int(int num,
                             int value) {
            sqlite3_bind_int(_stmt, num, value);
            return *this;
        }

        SQLiteSTMT& bind_int64(int num,
                               int64_t value) {
            sqlite3_bind_int64(_stmt, num, value);
            return *this;
        }

        const char* get_column_text(int num) {
            return (const char*)sqlite3_column_text(_stmt, num);
        }

        int get_column_int(int num) {
            return sqlite3_column_int(_stmt, num);
        }

        int64_t get_column_int64(int num) {
            return sqlite3_column_int64(_stmt, num);
        }

        int step() {
            int rc;
            bool run = false;

            do {
                rc = sqlite3_step(_stmt);
                switch (rc) {
                case SQLITE_LOCKED:
                    sqlite3_reset(_stmt);
                    run = true;
                    continue;

                case SQLITE_BUSY:
                    run = true;
                    continue;

                case SQLITE_ROW:
                case SQLITE_DONE:
                    run = false;
                    break;

                default:
                    CE_ASSERT_MSG("builddb", false, "SQL error '%s' (%d): %s",
                                  sqlite3_sql(_stmt), rc, sqlite3_errmsg(_db));

                    run = false;
                    break;
                }
            } while (run);

            return rc;
        }


    };

    BuildDB::BuildDB() : _db(0) {}

    BuildDB::~BuildDB() {
        close();
    };

    void BuildDB::open(const char* db_path) {
        sqlite3_open_v2(db_path,
                        &_db,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_SHAREDCACHE | SQLITE_OPEN_NOMUTEX,
                        NULL );
    }

    void BuildDB::close() {
        sqlite3_close(_db);
    }

    void BuildDB::set_file(const char* filename,
                           time_t mtime) {
        SQLiteSTMT(_db,
                   "INSERT OR REPLACE INTO files VALUES(NULL, ?1, ?2);"
                   ).bind_text(1, filename).bind_int64(2, mtime).step();
    }

    void BuildDB::set_file_depend(const char* filename,
                                  const char* depend_on) {
        SQLiteSTMT(_db,
                   "INSERT INTO file_dependency (filename, depend_on) SELECT ?1, ?2 WHERE NOT EXISTS(SELECT 1 FROM file_dependency WHERE filename = ?1 AND depend_on = ?2);"
                   ).bind_text(1, filename).bind_text(2, depend_on).step();
    }

    bool BuildDB::need_compile(StringId64_t root,
                               const char* filename) {
        bool compile = true;

        SQLiteSTMT stmt(_db,
                        "SELECT\n"
                        "    file_dependency.depend_on, files.mtime\n"
                        "FROM\n"
                        "    file_dependency\n"
                        "JOIN\n"
                        "    files on files.filename == file_dependency.depend_on\n"
                        "WHERE\n"
                        "    file_dependency.filename = ?1\n");


        stmt.bind_text(1, filename);

        while (stmt.step() == SQLITE_ROW) {
            compile = false;

            time_t actual_mtime = filesystem::file_mtime(root, stmt.get_column_text(0));
            time_t last_mtime = stmt.get_column_int64(1);

            if (actual_mtime != last_mtime) {
                compile = true;
                break;
            }
        }

        return compile;
    }

    bool BuildDB::init_db() {
        // Create files table
        {
            SQLiteSTMT query(_db,
                             "CREATE TABLE IF NOT EXISTS files (\n"
                             "id       INTEGER PRIMARY KEY    AUTOINCREMENT    NOT NULL,\n"
                             "filename TEXT    UNIQUE                          NOT NULL,\n"
                             "mtime    INTEGER                                 NOT NULL\n"
                             ");");

            if (query.step() != SQLITE_DONE) {
                return false;
            }
        }

        // Create file_dependency table
        {
            SQLiteSTMT query(_db,
                             "CREATE TABLE IF NOT EXISTS file_dependency (\n"
                             "id        INTEGER PRIMARY KEY    AUTOINCREMENT    NOT NULL,\n"
                             "filename  TEXT                                    NOT NULL,\n"
                             "depend_on TEXT                                    NOT NULL\n"
                             ");");

            if (query.step() != SQLITE_DONE) {
                return false;
            }
        }

        return true;
    }
}
