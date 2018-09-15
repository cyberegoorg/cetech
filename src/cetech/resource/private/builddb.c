#include <time.h>
#include <stdio.h>
#include <inttypes.h>

#include "include/sqlite3/sqlite3.h"

#include <celib/log.h>
#include <celib/os.h>
#include <celib/memory.h>
#include <celib/fs.h>
#include <cetech/kernel/kernel.h>
#include <celib/hashlib.h>
#include <celib/module.h>
#include <celib/api_system.h>
#include <celib/cdb.h>
#include <celib/config.h>
#include <celib/buffer.inl>
#include <cetech/resource/resource.h>
#include <celib/task.h>

#include "cetech/resource/builddb.h"

#define _G BUILDDB_GLOBALS
static struct _G {
    sqlite3 *db[TASK_MAX_WORKERS];
    char *_logdb_path;
} _G = {};

static int _step(sqlite3 *db,
                 sqlite3_stmt *stmt) {
    int rc;
    int run = 0;

    do {
        rc = sqlite3_step(stmt);
        switch (rc) {
            case SQLITE_LOCKED:
                ce_log_a0->warning("builddb", "SQL locked '%s' (%d): %s",
                                   sqlite3_sql(stmt), rc, sqlite3_errmsg(db));

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
                ce_log_a0->error("builddb", "SQL error '%s' (%d): %s",
                                 sqlite3_sql(stmt), rc, sqlite3_errmsg(db));

                run = 0;
                break;
        }
    } while (run);

    return rc;
}

static sqlite3 *_opendb() {
    uint32_t worker_idx = ce_task_a0->worker_id();

    if (_G.db[worker_idx]) {
        return _G.db[worker_idx];
    }

    sqlite3_open_v2(_G._logdb_path,
                    &_G.db[worker_idx],
                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                    SQLITE_OPEN_NOMUTEX,
                    NULL);

    sqlite3_exec(_G.db[worker_idx], "PRAGMA synchronous = NORMAL", NULL, NULL,
                 NULL);
    sqlite3_exec(_G.db[worker_idx], "PRAGMA journal_mode = WAL", NULL, NULL,
                 NULL);

    return _G.db[worker_idx];
}


static int _do_sql(const char *sql) {
    sqlite3 *_db = _opendb();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, NULL);

    if (_step(_db, stmt) != SQLITE_DONE) {
        goto error;
    }

    sqlite3_finalize(stmt);


    return 1;

    error:
    sqlite3_finalize(stmt);


    return 0;
}

const char *CREATE_SQL[] = {
        "CREATE TABLE IF NOT EXISTS files (\n"
        "id       INTEGER PRIMARY KEY    AUTOINCREMENT    NOT NULL,\n"
        "filename TEXT    UNIQUE                          NOT NULL,\n"
        "name     INTEGER                                 NOT NULL,\n"
        "type     INTEGER                                 NOT NULL,\n"
        "mtime    INTEGER                                 NOT NULL,\n"
        "data     BLOB                                            \n"
        ");",

        "CREATE INDEX IF NOT EXISTS files_name_type_index ON files (name, type);",

        "CREATE TABLE IF NOT EXISTS file_dependency ("
        "id        INTEGER PRIMARY KEY    AUTOINCREMENT    NOT NULL,\n"
        "filename  TEXT                                    NOT NULL,\n"
        "depend_on TEXT                                    NOT NULL\n"
        ");"
};

static int builddb_init_db() {
    const char *platform = ce_cdb_a0->read_str(ce_config_a0->obj(),
                                               CONFIG_PLATFORM, "");

    const char *build_dir_str = ce_cdb_a0->read_str(ce_config_a0->obj(),
                                                    CONFIG_BUILD, "");

    char *build_dir_full = NULL;
    ce_os_a0->path->join(&build_dir_full, ce_memory_a0->system, 2,
                         build_dir_str, platform);
    ce_os_a0->path->make_path(build_dir_full);

    ce_os_a0->path->join(&_G._logdb_path, ce_memory_a0->system, 2,
                         build_dir_full,
                         "build.db");

    ce_buffer_free(build_dir_full, ce_memory_a0->system);

    for (int i = 0; i < CE_ARRAY_LEN(CREATE_SQL); ++i) {
        if (!_do_sql(CREATE_SQL[i])) {
            return 0;
        }
    }
    return 1;
}

static void type_name_from_filename(const char *fullname,
                                    struct ct_resource_id *resource_id,
                                    char *short_name) {

    const char *resource_type = ce_os_a0->path->extension(fullname);

    size_t size = strlen(fullname) - strlen(resource_type) - 1;

    char resource_name[128] = {};
    memcpy(resource_name, fullname, size);

    resource_id->name = ce_id_a0->id64(resource_name);
    resource_id->type = ce_id_a0->id64(resource_type);

    if (short_name) {
        memcpy(short_name, fullname, sizeof(char) * size);
        short_name[size] = '\0';
    }
}

static void builddb_set_file(const char *filename,
                             time_t mtime,
                             const char *data,
                             uint64_t size) {
    static const char *sql = "INSERT OR REPLACE INTO files VALUES(NULL, ?1, ?2, ?3, ?4, ?5);";

    sqlite3 *_db = _opendb();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, NULL);


    struct ct_resource_id rid;

    type_name_from_filename(filename, &rid, NULL);

    sqlite3_bind_text(stmt, 1, filename, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, rid.name);
    sqlite3_bind_int64(stmt, 3, rid.type);
    sqlite3_bind_int64(stmt, 4, mtime);
    sqlite3_bind_blob(stmt, 5, data, size, NULL);
    _step(_db, stmt);

    sqlite3_finalize(stmt);

}

bool builddb_load_cdb_file(struct ct_resource_id resource,
                           uint64_t object,
                           struct ce_alloc *allocator) {
    static const char *sql = "SELECT data FROM files WHERE type = ?1 AND name = ?2;";

    sqlite3 *_db = _opendb();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, resource.type);
    sqlite3_bind_int64(stmt, 2, resource.name);

    int ok = _step(_db, stmt) == SQLITE_ROW;
    if (ok) {
        const char *data = sqlite3_column_blob(stmt, 0);
        ce_cdb_a0->load(ce_cdb_a0->db(), data, object, allocator);
    }

    sqlite3_finalize(stmt);


    return ok != 0;
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

}


static int buildb_get_filename_type_name(char *filename,
                                         size_t max_len,
                                         uint64_t type,
                                         uint64_t name) {

    static const char *sql = "SELECT filename FROM files WHERE type = ?1 AND name = ?2;";

    sqlite3 *_db = _opendb();
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(_db, sql, -1, &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, type);
    sqlite3_bind_int64(stmt, 2, name);

    int ok = 0;
    while (_step(_db, stmt) == SQLITE_ROW) {
        const unsigned char *fn = sqlite3_column_text(stmt, 0);

        snprintf(filename, max_len, "%s", fn);
    }
    ok = 1;

    sqlite3_finalize(stmt);


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

        time_t actual_mtime = ce_fs_a0->file_mtime(SOURCE_ROOT, dep_file);

        time_t last_mtime = sqlite3_column_int64(stmt, 1);

        if (actual_mtime != last_mtime) {
            compile = 1;
            break;
        }
    }

    sqlite3_finalize(stmt);


    return compile;
}

void _add_dependency(const char *who_filename,
                     const char *depend_on_filename) {

    builddb_set_file_depend(who_filename, depend_on_filename);
//    builddb_set_file(depend_on_filename,ce_fs_a0->file_mtime(SOURCE_ROOT,
//                                                 depend_on_filename));
}

static struct ct_builddb_a0 build_db_api = {
        .put_file = builddb_set_file,
        .load_cdb_file = builddb_load_cdb_file,
        .set_file_depend = builddb_set_file_depend,
        .get_filename_type_name = buildb_get_filename_type_name,
        .need_compile = builddb_need_compile,
        .add_dependency = _add_dependency,
};

struct ct_builddb_a0 *ct_builddb_a0 = &build_db_api;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {};

    api->register_api("ct_builddb_a0", ct_builddb_a0);

    builddb_init_db();
}

static void _shutdown() {
    for (int i = 0; i < TASK_MAX_WORKERS; ++i) {
        sqlite3_close_v2(_G.db[i]);
    }

    _G = (struct _G) {};
}

CE_MODULE_DEF(
        builddb,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);

        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)