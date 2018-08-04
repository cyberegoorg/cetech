#include <time.h>
#include <stdio.h>
#include <inttypes.h>

#include "include/sqlite3/sqlite3.h"

#include <corelib/log.h>
#include <corelib/os.h>
#include <corelib/memory.h>
#include <corelib/fs.h>
#include <cetech/kernel/kernel.h>
#include <corelib/hashlib.h>
#include <corelib/module.h>
#include <corelib/api_system.h>
#include <corelib/cdb.h>
#include <corelib/config.h>
#include <corelib/buffer.inl>
#include <cetech/resource/resource.h>

#include "cetech/resource/builddb.h"

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
                ct_log_a0->error("builddb", "SQL error '%s' (%d): %s",
                                 sqlite3_sql(stmt), rc, sqlite3_errmsg(db));

                run = 0;
                break;
        }
    } while (run);

    return rc;
}



static char *_logdb_path = NULL;

static sqlite3 *_opendb() {
    sqlite3 *_db;
    sqlite3_open_v2(_logdb_path,
                    &_db,
                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                    SQLITE_OPEN_SHAREDCACHE | SQLITE_OPEN_FULLMUTEX,
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

    sqlite3_finalize(stmt);
    sqlite3_close_v2(_db);

    return 1;

    error:
    sqlite3_finalize(stmt);
    sqlite3_close_v2(_db);

    return 0;
}

static int builddb_init_db() {
    const char *platform = ct_cdb_a0->read_str(ct_config_a0->obj(),
                                               CONFIG_PLATFORM, "");

    const char *build_dir_str = ct_cdb_a0->read_str(ct_config_a0->obj(),
                                                    CONFIG_BUILD, "");

    char *build_dir_full = NULL;
    ct_os_a0->path->join(&build_dir_full, ct_memory_a0->system, 2, build_dir_str, platform);
    ct_os_a0->path->make_path(build_dir_full);

    ct_os_a0->path->join(&_logdb_path, ct_memory_a0->system, 2, build_dir_full, "build.db");

    ct_buffer_free(build_dir_full, ct_memory_a0->system);

    if (!_do_sql("CREATE TABLE IF NOT EXISTS files (\n"
                 "id       INTEGER PRIMARY KEY    AUTOINCREMENT    NOT NULL,\n"
                 "filename TEXT    UNIQUE                          NOT NULL,\n"
                 "hash     TEXT    UNIQUE                          NOT NULL,\n"
                 "name     INTEGER                                 NOT NULL\n,"
                 "type     INTEGER                                 NOT NULL\n,"
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

    return 1;
}

static void type_name_from_filename(const char *fullname,
                             struct ct_resource_id *resource_id,
                             char *short_name) {

    const char *resource_type = ct_os_a0->path->extension(fullname);

    size_t size = strlen(fullname) - strlen(resource_type) - 1;

    char resource_name[128] = {};
    memcpy(resource_name, fullname, size);

    resource_id->name = ct_hashlib_a0->id64(resource_name);
    resource_id->type = ct_hashlib_a0->id64(resource_type);

    if (short_name) {
        memcpy(short_name, fullname, sizeof(char) * size);
        short_name[size] = '\0';
    }
}

static void builddb_set_file(const char *filename,
                             time_t mtime) {
    static const char *sql = "INSERT OR REPLACE INTO files VALUES(NULL, ?1, ?2, ?3, ?4, ?5);";

    sqlite3 *_db = _opendb();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, NULL);


    struct ct_resource_id rid;

    type_name_from_filename(filename, &rid, NULL);

    char build_name[128] = {};
    snprintf(build_name, CT_ARRAY_LEN(build_name), "%" PRIx64 "%" PRIx64, rid.type, rid.name);

    sqlite3_bind_text(stmt, 1, filename, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, build_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, rid.name);
    sqlite3_bind_int64(stmt, 4, rid.type);
    sqlite3_bind_int64(stmt, 5, mtime);
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

static int builddb_get_filename_by_hash(char *filename,
                                        size_t max_len,
                                        const char *hash) {
    static const char *sql = "SELECT filename FROM files WHERE hash = ?1;";

    sqlite3 *_db = _opendb();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, hash, -1, SQLITE_TRANSIENT);

    int ok = 0;
    while (_step(_db, stmt) == SQLITE_ROW) {
        const unsigned char*fn = sqlite3_column_text(stmt, 0);

        snprintf(filename, max_len, "%s", fn);
    }
    ok = 1;

    sqlite3_finalize(stmt);
    sqlite3_close_v2(_db);

    return ok;
}

static int buildb_get_filename_type_name(char *filename,
                              size_t max_len,
                              uint64_t type,
                              uint64_t  name) {

    static const char *sql = "SELECT filename FROM files WHERE type = ?1 AND name = ?2;";

    sqlite3 *_db = _opendb();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, type);
    sqlite3_bind_int64(stmt, 2, name);

    int ok = 0;
    while (_step(_db, stmt) == SQLITE_ROW) {
        const unsigned char*fn = sqlite3_column_text(stmt, 0);

        snprintf(filename, max_len, "%s", fn);
    }
    ok = 1;

    sqlite3_finalize(stmt);
    sqlite3_close_v2(_db);

    return ok;
}

static int builddb_need_compile(const char *filename) {
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

        time_t actual_mtime = ct_fs_a0->file_mtime(SOURCE_ROOT, dep_file);

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

void _add_dependency(const char *who_filename,
                     const char *depend_on_filename) {

    builddb_set_file_depend(who_filename, depend_on_filename);
    builddb_set_file(depend_on_filename,ct_fs_a0->file_mtime(SOURCE_ROOT,
                                                 depend_on_filename));
}

static struct ct_builddb_a0 build_db_api = {
    .put_file = builddb_set_file,
    .set_file_depend = builddb_set_file_depend,
    .get_filename_by_hash = builddb_get_filename_by_hash,
    .get_filename_type_name = buildb_get_filename_type_name,
    .need_compile = builddb_need_compile,
    .add_dependency = _add_dependency,
};

struct ct_builddb_a0 *ct_builddb_a0 =  &build_db_api;

static void _init(struct ct_api_a0 *api) {
    api->register_api("ct_builddb_a0", ct_builddb_a0);

    builddb_init_db();
}

static void _shutdown() {
}

CETECH_MODULE_DEF(
        builddb,
        {
            CT_INIT_API(api, ct_memory_a0);
            CT_INIT_API(api, ct_hashlib_a0);

        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
            _shutdown();
        }
)