#include <time.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

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

#define LOG_WHERE "builddb"
#define MAX_WORKERS TASK_MAX_WORKERS

#define _G BUILDDB_GLOBALS

struct sqls_s {
    sqlite3_stmt *put_file;
    sqlite3_stmt *put_resource;
    sqlite3_stmt *put_file_blob;
    sqlite3_stmt *load_file_blob;
    sqlite3_stmt *set_file_depend;
    sqlite3_stmt *get_filename;
    sqlite3_stmt *need_compile;
    sqlite3_stmt *get_file_id;
};

static struct _G {
    sqlite3 *db[MAX_WORKERS];
    struct sqls_s sqls[MAX_WORKERS];
    char *_logdb_path;
} _G = {};

const char *CREATE_SQL[] = {
        "CREATE TABLE IF NOT EXISTS files (\n"
        "id       INTEGER PRIMARY KEY                     NOT NULL,\n"
        "filename TEXT    UNIQUE                          NOT NULL,\n"
        "mtime    INTEGER                                 NOT NULL\n"
        ");",

        "CREATE TABLE IF NOT EXISTS resource (\n"
        "type     INTEGER                                 NOT NULL,\n"
        "name     INTEGER                                 NOT NULL,\n"
        "file     INTEGER                                         ,\n"
        "FOREIGN KEY(file) REFERENCES files(id),                   \n"
        "PRIMARY KEY (type, name)"
        ");",

        "CREATE TABLE IF NOT EXISTS file_dependency ("
        "file      INTEGER,\n"
        "depend_on INTEGER,\n"
        "FOREIGN KEY(file) REFERENCES files(id),\n"
        "FOREIGN KEY(depend_on) REFERENCES files(id)\n"
        ");",

        "CREATE TABLE IF NOT EXISTS resource_data (\n"
        "type     INTEGER                                 NOT NULL,\n"
        "name     INTEGER                                 NOT NULL,\n"
        "data     BLOB,                                            \n"
        "PRIMARY KEY (type, name)"
        ");"
        ""
};


#define _STATMENT(name, _sql)  \
        {.offset=offsetof(struct sqls_s, name), .sql=(_sql)}

static struct {
    uint64_t offset;
    const char *sql;
} _queries[] = {
        _STATMENT(put_file,
                  "INSERT OR REPLACE INTO files (id, filename, mtime) VALUES(?1, ?2, ?3);"),

        _STATMENT(put_file_blob,
                  "INSERT OR REPLACE INTO resource_data (type, name, data) VALUES(?1, ?2, ?3);"),

        _STATMENT(put_resource,
                  "INSERT OR REPLACE INTO resource (type, name, file) VALUES(?1, ?2, ?3);"),

        _STATMENT(load_file_blob,
                  "SELECT data FROM resource_data WHERE type = ?1 AND name = ?2;"),

        _STATMENT(set_file_depend,
                  "INSERT INTO file_dependency (file, depend_on) VALUES (?1, ?2);"),

        _STATMENT(get_filename,
                  "SELECT files.filename\n"
                  "FROM resource\n"
                  "JOIN files on files.id == resource.file\n"
                  "WHERE resource.type = ?1 and resource.name = ?2;"),

        _STATMENT(need_compile,
                  "SELECT\n"
                  "     files.filename, files.mtime\n"
                  "FROM\n"
                  "    file_dependency\n"
                  "JOIN\n"
                  "    files on files.id == file_dependency.depend_on\n"
                  "WHERE\n"
                  "    file_dependency.file = ?1\n"),

        _STATMENT(get_file_id,
                  "SELECT id FROM files WHERE filename = ?1")
};

static int _step(sqlite3 *db,
                 sqlite3_stmt *stmt) {
    int rc;
    int run = 0;

    do {
        rc = sqlite3_step(stmt);
        switch (rc) {
            case SQLITE_LOCKED:
                ce_log_a0->warning(LOG_WHERE, "SQL locked '%s' (%d): %s",
                                   sqlite3_sql(stmt), rc, sqlite3_errmsg(db));
                run = 1;
                continue;

            case SQLITE_BUSY:
                run = 1;
                sqlite3_reset(stmt);
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

    if ((rc != SQLITE_ROW)) {
        sqlite3_reset(stmt);
    }


    return rc;
}

static struct sqls_s *_get_sqls() {
    uint32_t worker_idx = ce_task_a0->worker_id();
    struct sqls_s *sqls = &_G.sqls[worker_idx];
    return sqls;
}

static sqlite3 *_opendb() {
    uint32_t worker_idx = ce_task_a0->worker_id();
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

    int worker_n = ce_task_a0->worker_count();
    for (int j = 0; j < worker_n; ++j) {
        sqlite3_open_v2(_G._logdb_path,
                        &_G.db[j],
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                        SQLITE_OPEN_NOMUTEX,
                        NULL);

        sqlite3_exec(_G.db[j], "PRAGMA synchronous = OFF", NULL, NULL,
                     NULL);
        sqlite3_exec(_G.db[j], "PRAGMA journal_mode = OFF", NULL, NULL,
                     NULL);
    }

    for (int i = 0; i < CE_ARRAY_LEN(CREATE_SQL); ++i) {
        if (!_do_sql(CREATE_SQL[i])) {
            return 0;
        }
    }


    for (int j = 0; j < worker_n; ++j) {
        sqlite3 *_db = _G.db[j];

        struct sqls_s *sqls = &_G.sqls[j];

        for (int i = 0; i < CE_ARRAY_LEN(_queries); ++i) {
            sqlite3_stmt **stm = (sqlite3_stmt **) (((char *) sqls) +
                                                    _queries[i].offset);

            int r = sqlite3_prepare_v2(_db, _queries[i].sql,
                                       -1, stm, NULL);

            if (r != SQLITE_OK) {
                ce_log_a0->error("builddb", "SQL error %d '%s' (%d): %s",
                                 i, _queries[i].sql, r,
                                 sqlite3_errmsg(_db));
            }

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

static void builddb_put_file(const char *filename,
                             time_t mtime,
                             const char *data,
                             uint64_t size) {

    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();

    sqlite3_bind_int64(sqls->put_file, 1, ce_id_a0->id64(filename));
    sqlite3_bind_text(sqls->put_file, 2, filename, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(sqls->put_file, 3, mtime);
    _step(_db, sqls->put_file);

    struct ct_resource_id rid;
    type_name_from_filename(filename, &rid, NULL);

    sqlite3_bind_int64(sqls->put_file_blob, 1, rid.type);
    sqlite3_bind_int64(sqls->put_file_blob, 2, rid.name);
    sqlite3_bind_blob(sqls->put_file_blob, 3, data, size, NULL);
    _step(_db, sqls->put_file_blob);
}

void buildb_put_resource(struct ct_resource_id resource,
                         const char *filename) {
    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();

    uint64_t id = ce_id_a0->id64(filename);

    sqlite3_bind_int64(sqls->put_resource, 1, resource.type);
    sqlite3_bind_int64(sqls->put_resource, 2, resource.name);
    sqlite3_bind_int64(sqls->put_resource, 3, id);
    _step(_db, sqls->put_resource);
}

bool builddb_load_cdb_file(struct ct_resource_id resource,
                           uint64_t object,
                           struct ce_alloc *allocator) {
    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();


    sqlite3_bind_int64(sqls->load_file_blob, 1, resource.type);
    sqlite3_bind_int64(sqls->load_file_blob, 2, resource.name);

    int ok = _step(_db, sqls->load_file_blob) == SQLITE_ROW;

    if (ok) {
        const char *data = sqlite3_column_blob(sqls->load_file_blob, 0);
        ce_cdb_a0->load(ce_cdb_a0->db(), data, object, allocator);
        _step(_db, sqls->load_file_blob);
    }

    return ok != 0;
}

static void builddb_set_file_depend(const char *filename,
                                    const char *depend_on) {
    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();

    sqlite3_bind_int64(sqls->set_file_depend, 1, ce_id_a0->id64(filename));
    sqlite3_bind_int64(sqls->set_file_depend, 2, ce_id_a0->id64(depend_on));

    _step(_db, sqls->set_file_depend);
}


static int buildb_get_filename_type_name(char *filename,
                                         size_t max_len,
                                         uint64_t type,
                                         uint64_t name) {

    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();

    sqlite3_bind_int64(sqls->get_filename, 1, type);
    sqlite3_bind_int64(sqls->get_filename, 2, name);

    int ok = _step(_db, sqls->get_filename) == SQLITE_ROW;
    if (ok) {

        const unsigned char *fn = sqlite3_column_text(sqls->get_filename, 0);

        snprintf(filename, max_len, "%s", fn);

        _step(_db, sqls->get_filename);
    }

    return ok;
}

static int builddb_need_compile(const char *filename) {
    int compile = 1;

    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();

    sqlite3_bind_int64(sqls->need_compile, 1, ce_id_a0->id64(filename));

    while (_step(_db, sqls->need_compile) == SQLITE_ROW) {
        compile = 0;
        const char *dep_file = (const char *) sqlite3_column_text(sqls->need_compile, 0);

        time_t actual_mtime = ce_fs_a0->file_mtime(SOURCE_ROOT, dep_file);

        time_t last_mtime = sqlite3_column_int64(sqls->need_compile, 1);

        if (actual_mtime != last_mtime) {
            compile = 1;
            break;
        }
    }

    return compile;
}

void _add_dependency(const char *who_filename,
                     const char *depend_on_filename) {

    builddb_set_file_depend(who_filename, depend_on_filename);
//    builddb_set_file(depend_on_filename,ce_fs_a0->file_mtime(SOURCE_ROOT,
//                                                 depend_on_filename));
}

static struct ct_builddb_a0 build_db_api = {
        .put_file = builddb_put_file,
        .put_resource = buildb_put_resource,
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