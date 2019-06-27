#include <time.h>
#include <stdio.h>
#include <stdbool.h>

#include "include/sqlite3/sqlite3.h"

#include <celib/memory/allocator.h>
#include <celib/log.h>

#include <celib/memory/memory.h>
#include <cetech/kernel/kernel.h>
#include <celib/id.h>
#include <celib/module.h>
#include <celib/api.h>
#include <celib/cdb.h>
#include <celib/config.h>
#include <celib/containers/buffer.h>
#include <cetech/resource/resource.h>
#include <celib/task.h>
#include <celib/containers/hash.h>
#include <celib/os/path.h>
#include <celib/os/thread.h>

#include "cetech/resource/resourcedb.h"

#define LOG_WHERE "resourcedb"
#define MAX_WORKERS TASK_MAX_WORKERS

#define _G resourcedb_globals

struct sqls_s {
    sqlite3_stmt *put_file;
    sqlite3_stmt *put_resource;
    sqlite3_stmt *put_file_blob;
    sqlite3_stmt *load_file_blob;
    sqlite3_stmt *get_filename;
    sqlite3_stmt *get_uid_by_filename;

    sqlite3_stmt *get_resource_by_type;
    sqlite3_stmt *get_resource_from_dirs;
    sqlite3_stmt *get_resource_type;
    sqlite3_stmt *resource_exist;
    sqlite3_stmt *set_file_resource;
};

static struct _G {
    sqlite3 *db[MAX_WORKERS];
    struct sqls_s sqls[MAX_WORKERS];

    ce_spinlock_t0 type_cache_lock;
    ce_hash_t type_cache;

    ce_spinlock_t0 uid_cache_lock;
    ce_hash_t uid_cache;

    char *_logdb_path;
    ce_alloc_t0 *alloc;
} _G = {};

const char *CREATE_SQL[] = {
        "CREATE TABLE IF NOT EXISTS files (\n"
        "id       INTEGER PRIMARY KEY                     NOT NULL,\n"
        "filename TEXT    UNIQUE                          NOT NULL,\n"
        "mtime    INTEGER                                 NOT NULL\n"
        ");",

        "CREATE TABLE IF NOT EXISTS resource (\n"
        "uid      INTEGER                                 NOT NULL,\n"
        "type     TEXT                                    ,\n"
        "file     INTEGER                                         ,\n"
        "FOREIGN KEY(file) REFERENCES files(id),                   \n"
        "PRIMARY KEY (uid)"
        ");",

        "CREATE TABLE IF NOT EXISTS resource_data (\n"
        "uid      INTEGER                                 NOT NULL,\n"
        "data     BLOB,                                            \n"
        "FOREIGN KEY(uid) REFERENCES resource(uid),                \n"
        "PRIMARY KEY (uid)                                         \n"
        ");"
        "",

        "CREATE TABLE IF NOT EXISTS file_resource (\n"
        "uid      INTEGER                                 NOT NULL,\n"
        "file     INTEGER                                 NOT NULL,\n"
        "FOREIGN KEY(file) REFERENCES files(id),                   \n"
        "FOREIGN KEY(uid) REFERENCES resource(uid),                \n"
        "PRIMARY KEY (uid)                                         \n"
        ");"
        ""
};


#define _STATMENT(name, _sql)  \
        {.offset=offsetof(struct sqls_s, name), .sql=(_sql)}

static struct {
    uint64_t offset;
    const char *sql;
} _queries[] = {

        _STATMENT(get_filename,
                  "SELECT files.filename\n"
                  "FROM resource\n"
                  "JOIN files on files.id == resource.file\n"
                  "WHERE resource.uid = ?1;"),

        _STATMENT(resource_exist,
                  "SELECT 1\n"
                  "FROM resource\n"
                  "WHERE resource.uid = ?1;"),

        _STATMENT(get_resource_type,
                  "SELECT resource.type\n"
                  "FROM resource\n"
                  "WHERE resource.uid = ?1;"),

        _STATMENT(put_file,
                  "INSERT OR REPLACE INTO files (id, filename, mtime) VALUES(?1, ?2, ?3);"),

        _STATMENT(put_file_blob,
                  "INSERT OR REPLACE INTO resource_data (uid, data) VALUES(?1, ?2);"),

        _STATMENT(put_resource,
                  "INSERT OR REPLACE INTO resource (uid, type, file) VALUES(?1, ?2, ?3);"),

        _STATMENT(load_file_blob,
                  "SELECT data FROM resource_data WHERE uid = ?1"),

        _STATMENT(set_file_resource,
                  "INSERT OR REPLACE INTO file_resource (uid, file) VALUES(?1, ?2);"),

        _STATMENT(get_uid_by_filename,
                  "SELECT file_resource.uid\n"
                  "FROM file_resource\n"
                  "WHERE file_resource.file = ?1;"),

        _STATMENT(get_resource_by_type,
                  "select files.filename\n"
                  "from files\n"
                  "join resource r on files.id = r.file\n"
                  "join file_resource rr on r.uid = rr.uid\n"
                  "where r.type = ?1 and files.filename like ?2;"),

        _STATMENT(get_resource_from_dirs,
                  "select files.filename\n"
                  "from files\n"
                  "where instr(files.filename, ?1);"),
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
                ce_log_a0->error("resourcedb", "SQL error '%s' (%d): %s",
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

static int resourcedb_init_db() {
    const char *platform = ce_config_a0->read_str(CONFIG_PLATFORM, "");
    const char *build_dir_str = ce_config_a0->read_str(CONFIG_BUILD, "");

    char *build_dir_full = NULL;
    ce_os_path_a0->join(&build_dir_full, ce_memory_a0->system, 2,
                        build_dir_str, platform);
    ce_os_path_a0->make_path(build_dir_full);

    ce_os_path_a0->join(&_G._logdb_path, ce_memory_a0->system, 2,
                        build_dir_full,
                        "resource.db");

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
            sqlite3_stmt **stm = (sqlite3_stmt * *)(((char *) sqls) +
                                                    _queries[i].offset);

            int r = sqlite3_prepare_v2(_db, _queries[i].sql,
                                       -1, stm, NULL);

            if (r != SQLITE_OK) {
                ce_log_a0->error("resourcedb", "SQL error %d '%s' (%d): %s",
                                 i, _queries[i].sql, r,
                                 sqlite3_errmsg(_db));
            }

        }
    }

    return 1;
}

static void put_file(const char *filename,
                     time_t mtime) {

    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();

    sqlite3_bind_int64(sqls->put_file, 1, ce_id_a0->id64(filename));
    sqlite3_bind_text(sqls->put_file, 2, filename, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(sqls->put_file, 3, mtime);
    _step(_db, sqls->put_file);
}

static void put_resource_blob(ct_resource_id_t0 rid,
                              const char *data,
                              uint64_t size) {
    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();

    sqlite3_bind_int64(sqls->put_file_blob, 1, rid.uid);
    sqlite3_bind_blob(sqls->put_file_blob, 2, data, size, NULL);
    _step(_db, sqls->put_file_blob);
}

static void put_resource(ct_resource_id_t0 rid,
                         const char *type,
                         const char *filename,
                         bool root) {
    sqlite3 *_db = _opendb();

    struct sqls_s *sqls = _get_sqls();
    uint64_t id = ce_id_a0->id64(filename);

    if (root) {
        sqlite3_bind_int64(sqls->set_file_resource, 1, rid.uid);
        sqlite3_bind_int64(sqls->set_file_resource, 2, id);
        _step(_db, sqls->set_file_resource);

    } else {
        sqlite3_bind_text(sqls->put_resource, 3, NULL, -1, SQLITE_TRANSIENT);
    }

    sqlite3_bind_int64(sqls->put_resource, 1, rid.uid);
    sqlite3_bind_text(sqls->put_resource, 2, type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(sqls->put_resource, 3, id);
    _step(_db, sqls->put_resource);
}

bool load_cdb_file(ce_cdb_t0 db,
                   ct_resource_id_t0 resource,
                   uint64_t object,
                   struct ce_alloc_t0 *allocator) {
    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();


    sqlite3_bind_int64(sqls->load_file_blob, 1, resource.uid);

    int ok = _step(_db, sqls->load_file_blob) == SQLITE_ROW;

    char *buff = NULL;

    if (ok) {
        const char *data = sqlite3_column_blob(sqls->load_file_blob, 0);
        int s = sqlite3_column_bytes(sqls->load_file_blob, 0);
        ce_array_push_n(buff, data, s, allocator);

        _step(_db, sqls->load_file_blob);

        ce_cdb_a0->load(db, buff, object, allocator);

        ce_array_free(buff, allocator);
    }

    return ok != 0;
}

static int get_resource_from_dirs(const char *dir,
                                  char ***filename,
                                  struct ce_alloc_t0 *alloc) {
//    if (!strlen(dir)) {
//        return 0;
//    }

    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();

    sqlite3_bind_text(sqls->get_resource_from_dirs, 1, dir, -1, SQLITE_TRANSIENT);

    while (_step(_db, sqls->get_resource_from_dirs) == SQLITE_ROW) {
        const char *name;
        name = (const char *) sqlite3_column_text(sqls->get_resource_from_dirs,
                                                  0);

        if (name && name[0]) {
            const size_t len = strlen(name);
            char *fn = CE_ALLOC(alloc, char, len + 1);
            snprintf(fn, len + 1, "%s", name);

            ce_array_push(*filename, fn, alloc);
        }
    }

    return 1;
}


static bool obj_exist(ct_resource_id_t0 resource) {
    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();

    sqlite3_bind_int64(sqls->resource_exist, 1, resource.uid);

    int ok = _step(_db, sqls->resource_exist) == SQLITE_ROW;
    if (ok) {
        _step(_db, sqls->resource_exist);
        return true;
    }

    return false;
}

uint64_t get_resource_type(ct_resource_id_t0 resource) {
    ce_os_thread_a0->spin_lock(&_G.type_cache_lock);
    uint64_t type = ce_hash_lookup(&_G.type_cache, resource.uid, 0);
    ce_os_thread_a0->spin_unlock(&_G.type_cache_lock);

    if (type) {
        return type;
    }

    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();

    sqlite3_bind_int64(sqls->get_resource_type, 1, resource.uid);

    int ok = _step(_db, sqls->get_resource_type) == SQLITE_ROW;
    if (ok) {
        const char *type_str = (const char *) sqlite3_column_text(sqls->get_resource_type, 0);
        type = ce_id_a0->id64(type_str);
        _step(_db, sqls->get_resource_type);
    }

    ce_os_thread_a0->spin_lock(&_G.type_cache_lock);
    ce_hash_add(&_G.type_cache, resource.uid, type, _G.alloc);
    ce_os_thread_a0->spin_unlock(&_G.type_cache_lock);

    return type;
}

bool get_resource_filename(ct_resource_id_t0 resource,
                           char *filename,
                           size_t max_len) {
    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();

    sqlite3_bind_int64(sqls->get_filename, 1, resource.uid);

    int ok = _step(_db, sqls->get_filename) == SQLITE_ROW;
    if (ok) {
        const unsigned char *fn = sqlite3_column_text(sqls->get_filename, 0);
        snprintf(filename, max_len, "%s", fn);

        _step(_db, sqls->get_filename);
    }

    return ok;
}

ct_resource_id_t0 get_file_resource(const char *fullname) {
    uint64_t fullname_hash = ce_id_a0->id64(fullname);

    ce_os_thread_a0->spin_lock(&_G.uid_cache_lock);
    uint64_t uid = ce_hash_lookup(&_G.uid_cache, fullname_hash, 0);
    ce_os_thread_a0->spin_unlock(&_G.uid_cache_lock);

    if (uid) {
        return (ct_resource_id_t0) {.uid = uid};
    }

    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();

    sqlite3_bind_int64(sqls->get_uid_by_filename, 1, fullname_hash);

    int ok = _step(_db, sqls->get_uid_by_filename) == SQLITE_ROW;
    if (ok) {
        uid = sqlite3_column_int64(sqls->get_uid_by_filename, 0);

        _step(_db, sqls->get_uid_by_filename);
    }

    if (uid) {
        ce_os_thread_a0->spin_lock(&_G.uid_cache_lock);
        ce_hash_add(&_G.uid_cache, fullname_hash, uid, _G.alloc);
        ce_os_thread_a0->spin_unlock(&_G.uid_cache_lock);
    }

    return (ct_resource_id_t0) {.uid = uid};
}

int get_resource_by_type(const char *name,
                         const char *type,
                         char ***filename,
                         struct ce_alloc_t0 *alloc) {
    sqlite3 *_db = _opendb();
    struct sqls_s *sqls = _get_sqls();

    char tmp_name[256] = {};
    snprintf(tmp_name, CE_ARRAY_LEN(tmp_name), "%%%s%%", name);

    sqlite3_bind_text(sqls->get_resource_by_type, 1, type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(sqls->get_resource_by_type, 2, tmp_name, -1, SQLITE_TRANSIENT);

    while (_step(_db, sqls->get_resource_by_type) == SQLITE_ROW) {
        const unsigned char *fn = sqlite3_column_text(sqls->get_resource_by_type, 0);

        if (fn) {
            char *dup_str = ce_memory_a0->str_dup((const char *) fn, alloc);
            ce_array_push(*filename, dup_str, alloc);
        }

        //_step(_db, sqls->list_resource_by_type);
    }

    return 1;
}

void clean_resource_list(char **filename,
                         struct ce_alloc_t0 *alloc) {
    const uint32_t n = ce_array_size(filename);
    for (int i = 0; i < n; ++i) {
        CE_FREE(alloc, filename[i]);
    }
}

static struct ct_resourcedb_a0 ct_resourcedb_api = {
        .put_file = put_file,
        .put_resource_blob = put_resource_blob,
        .put_resource = put_resource,
        .load_cdb_file = load_cdb_file,
        .obj_exist = obj_exist,
        .get_resource_type = get_resource_type,
        .get_resource_filename = get_resource_filename,
        .get_file_resource = get_file_resource,
        .list_resource_from_dirs = get_resource_from_dirs,
        .list_resource_by_type = get_resource_by_type,
        .clean_resource_list = clean_resource_list,
};

struct ct_resourcedb_a0 *ct_resourcedb_a0 = &ct_resourcedb_api;

void CE_MODULE_LOAD(resourcedb)(struct ce_api_a0 *api,
                                int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);

    _G = (struct _G) {
            .alloc = ce_memory_a0->system,
    };

    api->add_api(CT_BUILDDB_API, ct_resourcedb_a0, sizeof(ct_resourcedb_api));

    resourcedb_init_db();
}

void CE_MODULE_UNLOAD(resourcedb)(struct ce_api_a0 *api,
                                  int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    for (int i = 0; i < TASK_MAX_WORKERS; ++i) {
        sqlite3_close_v2(_G.db[i]);
    }

    _G = (struct _G) {};
}
