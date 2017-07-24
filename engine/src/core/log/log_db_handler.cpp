
#include <time.h>


#include <cetech/core/log.h>
#include <cetech/core/os/errors.h>
#include <cetech/machine/machine.h>
#include <cetech/core/api_system.h>

#include <cetech/core/memory.h>
#include <cetech/core/os/path.h>

#include "include/sqlite3/sqlite3.h"

CETECH_DECL_API(ct_log_a0);

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

static char *logdb_path = nullptr;
static time_t _session_id = {0};
static sqlite3 *db[256] = {0};

static sqlite3 *_opendb(char worker_id) {
    if (db[worker_id] != 0) {
        return db[worker_id];
    }

    sqlite3 *_db;
    sqlite3_open_v2(logdb_path,
                    &_db,
                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                    SQLITE_OPEN_SHAREDCACHE,
                    NULL);

    // thanks http://stackoverflow.com/questions/1711631/improve-insert-per-second-performance-of-sqlite
    sqlite3_exec(_db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
    sqlite3_exec(_db, "PRAGMA journal_mode = MEMORY", NULL, NULL, NULL);

    db[worker_id] = _db;

    return _db;
}


static int _do_sql(char worker_id,
                   const char *sql) {
    sqlite3 *_db = _opendb(worker_id);
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


void logdb_log(enum ct_log_level level,
               time_t time,
               char worker_id,
               const char *where,
               const char *msg,
               void *data) {

    static const char *_level_to_str[4] = {
            [LOG_INFO] = "info",
            [LOG_WARNING] = "warning",
            [LOG_ERROR] = "error",
            [LOG_DBG] = "debug"
    };


    static const char *sql = "INSERT INTO log VALUES(NULL, ?1, ?2, ?3, ?4, ?5, ?6);";

    sqlite3 *_db = _opendb(worker_id);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, _session_id);
    sqlite3_bind_int64(stmt, 2, time);
    sqlite3_bind_text(stmt, 3, _level_to_str[level], -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, where, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 5, worker_id);
    sqlite3_bind_text(stmt, 6, msg, -1, SQLITE_TRANSIENT);

    _step(_db, stmt);

    sqlite3_finalize(stmt);
    //sqlite3_closeq_v2(_db);
}

namespace logsystem {
    int logdb_init_db(const char *log_dir,
                      ct_api_a0 *api) {
        CETECH_GET_API(api, ct_log_a0);

        ct_path_a0 *path = (ct_path_a0 *) api->first("ct_path_a0").api;
        ct_memory_a0 *memory = (ct_memory_a0 *) api->first(
                "ct_memory_a0").api;

        logdb_path = path->join(memory->main_allocator(), 2, log_dir, "log.db");

        _session_id = time(NULL);

        if (!_do_sql(0, "CREATE TABLE IF NOT EXISTS log (\n"
                "id         INTEGER PRIMARY KEY    AUTOINCREMENT   NOT NULL,\n"
                "session_id INTEGER                                NOT NULL,\n"
                "time       INTEGER                                NOT NULL,\n"
                "level      TEXT                                   NOT NULL,\n"
                "wheree     TEXT                                   NOT NULL,\n"
                "worker     INTEGER                                NOT NULL,\n"
                "msg        TEXT                                   NOT NULL\n"
                ");")) {
            return 0;
        }

        ct_log_a0.register_handler(logdb_log, NULL);
        return 1;
    }
}