#ifndef CETECH_BUILDDB_H
#define CETECH_BUILDDB_H

#include <time.h>
#include "include/sqlite3/sqlite3.h"


static int _step(sqlite3 *db,
                 sqlite3_stmt *stmt) {
    int rc;
    int run = 0;

    do {
        rc = sqlite3_step(stmt);
        switch (rc) {
            case SQLITE_LOCKED:
                sqlite3_reset(stmt);
                run = 1;
                continue;

            case SQLITE_BUSY:
                run = 1;
                continue;

            case SQLITE_ROW:
            case SQLITE_DONE:
                run = 0;
                break;

            default:
                ct_log_a0.error("builddb", "SQL error '%s' (%d): %s",
                                sqlite3_sql(stmt), rc, sqlite3_errmsg(db));

                run = 0;
                break;
        }
    } while (run);

    return rc;
}

static char *_logdb_path = nullptr;

static sqlite3 *_opendb() {
    sqlite3 *_db;
    sqlite3_open_v2(_logdb_path,
                    &_db,
                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                    SQLITE_OPEN_SHAREDCACHE | SQLITE_OPEN_NOMUTEX,
                    NULL);

    // thanks http://stackoverflow.com/questions/1711631/improve-insert-per-second-performance-of-sqlite
    sqlite3_exec(_db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
    sqlite3_exec(_db, "PRAGMA journal_mode = MEMORY", NULL, NULL, NULL);

    return _db;
}


static int _do_sql(const char *sql) {
    sqlite3 *_db = _opendb();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, NULL);

    if (_step(_db, stmt) != SQLITE_DONE) {
        goto error;
    }

    return 1;

    error:
    sqlite3_finalize(stmt);
    sqlite3_close_v2(_db);

    return 0;
}

static int builddb_init_db(const char *build_dir,
                           struct ct_path_a0 *path,
                           struct ct_memory_a0 *memory) {

    path->join(&_logdb_path, memory->main_allocator(), 2, build_dir, "build.db");


    if (!_do_sql("CREATE TABLE IF NOT EXISTS files (\n"
                         "id       INTEGER PRIMARY KEY    AUTOINCREMENT    NOT NULL,\n"
                         "filename TEXT    UNIQUE                          NOT NULL,\n"
                         "mtime    INTEGER                                 NOT NULL\n"
                         ");")) {
        return 0;
    }

    if (!_do_sql("CREATE TABLE IF NOT EXISTS file_dependency ("
                         "id        INTEGER PRIMARY KEY    AUTOINCREMENT    NOT NULL,\n"
                         "filename  TEXT                                    NOT NULL,\n"
                         "depend_on TEXT                                    NOT NULL\n"
                         ");")) {
        return 0;
    }

    if (!_do_sql("CREATE TABLE IF NOT EXISTS file_hash (\n"
                         "filename  TEXT    PRIMARY KEY NOT NULL,\n"
                         "hash      TEXT    UNIQUE NOT NULL\n"
                         ");")) {
        return 0;
    }

    return 1;
}

static void builddb_set_file(const char *filename,
                             time_t mtime) {
    static const char *sql = "INSERT OR REPLACE INTO files VALUES(NULL, ?1, ?2);";

    sqlite3 *_db = _opendb();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, filename, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, mtime);
    _step(_db, stmt);

    sqlite3_finalize(stmt);
    sqlite3_close_v2(_db);
}

static void builddb_set_file_depend(const char *filename,
                                    const char *depend_on) {
    static const char *sql = "INSERT INTO file_dependency (filename, depend_on) SELECT ?1, ?2 WHERE NOT EXISTS(SELECT 1 FROM file_dependency WHERE filename = ?1 AND depend_on = ?2);";

    sqlite3 *_db = _opendb();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, filename, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, depend_on, -1, SQLITE_TRANSIENT);

    _step(_db, stmt);

    sqlite3_finalize(stmt);
    sqlite3_close_v2(_db);
}

static void builddb_set_file_hash(const char *filename,
                                  const char *hash) {
    static const char *sql = "INSERT OR REPLACE INTO file_hash VALUES(?1, ?2);";

    sqlite3 *_db = _opendb();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, filename, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hash, -1, SQLITE_TRANSIENT);

    _step(_db, stmt);

    sqlite3_finalize(stmt);
    sqlite3_close_v2(_db);
}

static int builddb_get_filename_by_hash(char *filename,
                                        size_t max_len,
                                        const char *hash) {
    static const char *sql = "SELECT filename FROM file_hash WHERE hash = ?1;";

    sqlite3 *_db = _opendb();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, hash, -1, SQLITE_TRANSIENT);

    int ok = 0;
    while (_step(_db, stmt) == SQLITE_ROW) {
        const u_char *fn = sqlite3_column_text(stmt, 0);

        snprintf(filename, max_len, "%s", fn);
    }
    ok = 1;

    sqlite3_finalize(stmt);
    sqlite3_close_v2(_db);

    return ok;
}

static int builddb_need_compile(const char *filename,
                                struct ct_fs_a0 *fs) {
    static const char *sql = "SELECT\n"
            "    file_dependency.depend_on, files.mtime\n"
            "FROM\n"
            "    file_dependency\n"
            "JOIN\n"
            "    files on files.filename == file_dependency.depend_on\n"
            "WHERE\n"
            "    file_dependency.filename = ?1\n";

    int compile = 1;

    sqlite3 *_db = _opendb();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, filename, -1, SQLITE_TRANSIENT);

    while (_step(_db, stmt) == SQLITE_ROW) {
        compile = 0;
        const char *dep_file = (const char *) sqlite3_column_text(stmt, 0);

        time_t actual_mtime = fs->file_mtime(
                CT_ID64_0("source"), dep_file);

        time_t last_mtime = sqlite3_column_int64(stmt, 1);

        if (actual_mtime != last_mtime) {
            compile = 1;
            break;
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close_v2(_db);

    return compile;
}


#endif //CETECH_BUILDDB_H
