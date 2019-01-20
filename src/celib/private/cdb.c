#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <time.h>

#include <sys/mman.h>

#include <celib/macros.h>
#include <celib/api_system.h>
#include <celib/memory.h>
#include <celib/log.h>
#include <celib/cdb.h>
#include <celib/module.h>
#include <celib/allocator.h>
#include <celib/hash.inl>
#include <celib/os.h>
#include <celib/buffer.inl>
#include <celib/hashlib.h>

#include <sys/time.h>
#include <math.h>

#define _G coredb_global
#define LOG_WHERE "coredb"

#define MAX_OBJECTS 1000000000ULL

// TODO: X( non optimal braindump code
// TODO: remove null element
// TODO: only 64bit values => remove offset
// TODO: remove locks
// TODO: split object_t

struct object_t {
    // prefab
    uint64_t instance_of;
    uint64_t *instances;

    // hiearchy
    uint64_t key;
    uint64_t parent;

    // writer
    uint64_t orig_obj;
    struct ce_cdb_change_ev0 *changed;

    // object
    uint64_t type;
    struct ce_cdb_t db;
    struct ce_hash_t prop_map;

    uint64_t properties_count;

    uint64_t *keys;
    uint8_t *property_type;
    uint64_t *offset;
    uint8_t *values;
};

struct db_t {
    uint32_t used;
    uint32_t idx;

    // changed
    struct ce_spinlock change_lock;
    uint64_t *changed_obj;
    struct ce_hash_t changed_obj_set;

    // id pool
    uint64_t *to_free_objects_id;
    atomic_ullong to_free_objects_id_n;

    // objects
    struct object_t *object_pool;
    struct object_t **free_objects;
    struct object_t **to_free_objects;

    atomic_ullong object_pool_n;
    atomic_ullong free_objects_n;
    atomic_ullong to_free_objects_n;

    struct ce_spinlock id_map_lock;
    struct ce_hash_t id_map;
};

static struct _G {
    struct db_t *dbs;
    uint32_t *to_free_db;

    struct ce_alloc *allocator;
    struct ce_cdb_t global_db;

    ct_cdb_obj_loader loader;
} _G;

void set_loader(ct_cdb_obj_loader loader) {
    _G.loader = loader;
}

uint64_t _get_ms() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);

    time_t  s = spec.tv_sec;
    uint64_t ms = round(spec.tv_nsec / 1.0e6);
    if (ms > 999) {
        s++;
        ms = 0;
    }

    ms += s * 1000;

    return ms;
}

static uint64_t _next_uid(uint64_t epoch_offset,
                          uint16_t session) {
    static atomic_uint g_seq;
    union {
        uint64_t ui;
        struct {
            uint64_t seq:8;
            uint64_t session:12;
            uint64_t timestamp:44;
        };
    } id = {.ui = 0};

    id.timestamp = _get_ms() - epoch_offset;
    id.session = session;
    id.seq = atomic_fetch_add(&g_seq, 1) + 1;

//    ce_log_a0->debug(LOG_WHERE, "New UID 0x%llx", id.ui);

    return id.ui;
}


static uint64_t _gen_uid() {
    return _next_uid(1547316500ULL, 1);
}

static struct db_t *_get_db(struct ce_cdb_t db) {
    return &_G.dbs[db.idx];
}

static struct object_t *_get_object_from_id(struct db_t *db,
                                            uint64_t objid) {
    if (!objid) {
        return NULL;
    }

    ce_os_a0->thread->spin_lock(&db->id_map_lock);
    uint64_t obj = ce_hash_lookup(&db->id_map, objid, 0);
    ce_os_a0->thread->spin_unlock(&db->id_map_lock);

    if (!obj) {
        if (_G.loader(objid)) {
            ce_os_a0->thread->spin_lock(&db->id_map_lock);
            uint64_t obj = ce_hash_lookup(&db->id_map, objid, 0);
            ce_os_a0->thread->spin_unlock(&db->id_map_lock);

            if(obj) {
                return _get_object_from_id(db, objid);
            }

            return NULL;
        }
        return NULL; // and destroy earth.
    }

    return ((struct object_t *) obj);
}

static void _set_uid_obj(struct db_t *db,
                         uint64_t uid,
                         ce_cdb_obj_o *obj) {
    CE_ASSERT(LOG_WHERE, uid != 0);
    CE_ASSERT(LOG_WHERE, obj != 0);

    ce_os_a0->thread->spin_lock(&db->id_map_lock);
    ce_hash_add(&db->id_map, uid, (uint64_t) obj, _G.allocator);
    ce_os_a0->thread->spin_unlock(&db->id_map_lock);
}

void _remove_uid_obj(struct db_t *db,
                     uint64_t uid) {
//    ce_os_a0->thread->spin_lock(&db->id_map_lock);
    ce_hash_remove(&db->id_map, uid);
//    ce_os_a0->thread->spin_unlock(&db->id_map_lock);
}

static struct object_t *_get_object_from_o(const ce_cdb_obj_o *obj_o) {
    return ((struct object_t *) obj_o);
}

static void _add_change(struct object_t *obj,
                        struct ce_cdb_change_ev0 ev) {
    struct db_t *db_inst = &_G.dbs[obj->db.idx];

    if (!ce_hash_contain(&db_inst->changed_obj_set, obj->orig_obj)) {
        ce_os_a0->thread->spin_lock(&db_inst->change_lock);
        ce_array_push(db_inst->changed_obj, obj->orig_obj, _G.allocator);
        ce_hash_add(&db_inst->changed_obj_set, obj->orig_obj, 0, _G.allocator);
        ce_os_a0->thread->spin_unlock(&db_inst->change_lock);
    };

    uint32_t n = ce_array_size(obj->changed);
    for (int i = 0; i < n; ++i) {
        struct ce_cdb_change_ev0 *ev_it = &obj->changed[i];
        if ((ev_it->type == ev.type)
            && (ev_it->prop == ev.prop)) {
            ev_it->new_value = ev.new_value;
            ev_it->old_value = ev.old_value;
            return;
        }
    }

    ce_os_a0->thread->spin_lock(&db_inst->change_lock);
    ce_array_push(obj->changed, ev, _G.allocator);
    ce_os_a0->thread->spin_unlock(&db_inst->change_lock);
}

static uint64_t _object_new_property(struct object_t *obj,
                                     uint64_t key,
                                     enum ce_cdb_type type,
                                     const void *value,
                                     size_t size,
                                     const struct ce_alloc *alloc) {
    const uint64_t prop_count = obj->properties_count;
    const uint64_t values_size = ce_array_size(obj->values);

    ce_array_push(obj->keys, key, alloc);
    ce_array_push(obj->property_type, type, alloc);
    ce_array_push(obj->offset, values_size, alloc);

    if (size) {
        ce_array_resize(obj->values, values_size + size, alloc);
    }

    ce_hash_add(&obj->prop_map, key, prop_count, alloc);

    obj->properties_count = obj->properties_count + 1;
    return prop_count;
}

struct object_t *_new_object(struct db_t *db,
                             const struct ce_alloc *a) {

    struct object_t *obj;
    if (db->free_objects_n) {
        atomic_fetch_sub(&db->free_objects_n, 1);
        obj = db->free_objects[db->free_objects_n];
    } else {
        uint64_t idx = atomic_fetch_add(&db->object_pool_n, 1);

        obj = &db->object_pool[idx];

        *obj = (struct object_t) {};
    }

    obj->db.idx = db->idx;
    obj->key = 0;

    _object_new_property(obj, 0, CDB_TYPE_NONE, NULL, 0, a);
    return obj;
}

static struct object_t *_object_clone(struct db_t *db,
                                      struct object_t *obj,
                                      const struct ce_alloc *alloc) {
    const uint64_t properties_count = obj->properties_count;
    const uint64_t values_size = ce_array_size(obj->values);

    struct object_t *new_obj = _new_object(db, alloc);

    new_obj->instance_of = obj->instance_of;
    new_obj->parent = obj->parent;
    new_obj->key = obj->key;

    new_obj->type = obj->type;
    new_obj->properties_count = properties_count;

    ce_array_push_n(new_obj->changed, obj->changed,
                    ce_array_size(obj->changed), alloc);

    ce_array_push_n(new_obj->keys, obj->keys + 1,
                    ce_array_size(obj->keys) - 1, alloc);

    ce_array_push_n(new_obj->property_type, obj->property_type + 1,
                    ce_array_size(obj->property_type) - 1, alloc);

    ce_array_push_n(new_obj->offset, obj->offset + 1,
                    ce_array_size(obj->offset) - 1, alloc);

    if (values_size)
        ce_array_push_n(new_obj->values, obj->values, values_size, alloc);


    ce_hash_clone(&obj->prop_map, &new_obj->prop_map, alloc);

    uint32_t n = ce_array_size(obj->instances);
    if (n) {
        ce_array_push_n(new_obj->instances, obj->instances, n, alloc);
    }

    return new_obj;
}

static uint64_t _find_prop_index(const struct object_t *obj,
                                 uint64_t key) {
    return ce_hash_lookup(&obj->prop_map, key, 0);
}

static void _destroy_object(struct db_t *db_inst,
                            struct object_t *obj) {
    uint64_t idx = atomic_fetch_add(&db_inst->to_free_objects_n, 1);
    db_inst->to_free_objects[idx] = obj;
}


static void *virt_alloc(uint64_t size) {
    return mmap(NULL,
                size,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS |
                MAP_NORESERVE,
                -1, 0);
}

static int virt_free(void *addr,
                     uint64_t size) {
    return munmap(addr, size);
}

static struct ce_cdb_t create_db() {
    uint64_t n = ce_array_size(_G.dbs);

    uint32_t idx = UINT32_MAX;
    for (uint32_t i = 0; i < n; ++i) {
        if (!_G.dbs[i].used) {
            idx = i;
            break;
        }
    }

    if (UINT32_MAX == idx) {
        idx = ce_array_size(_G.dbs);
        ce_array_push(_G.dbs, (struct db_t) {}, _G.allocator);
    }

    _G.dbs[idx] = (struct db_t) {
            .used = true,
            .idx = idx,

            .to_free_objects_id = (uint64_t *) virt_alloc(MAX_OBJECTS *
                                                          sizeof(struct object_t **)),

            .object_pool = (struct object_t *) virt_alloc(MAX_OBJECTS *
                                                          sizeof(struct object_t)),

            .free_objects = (struct object_t **) virt_alloc(MAX_OBJECTS *
                                                            sizeof(struct object_t *)),

            .to_free_objects = (struct object_t **) virt_alloc(MAX_OBJECTS *
                                                               sizeof(struct object_t *))
    };

    return (struct ce_cdb_t) {.idx = idx};
};

static void create_object_uid(struct ce_cdb_t db,
                              uint64_t uid,
                              uint64_t type) {
    struct db_t *db_inst = &_G.dbs[db.idx];
    struct object_t *obj = _new_object(db_inst, _G.allocator);

    _set_uid_obj(db_inst, uid, obj);

    obj->db = db;
    obj->type = type;
    obj->orig_obj = uid;
}


static uint64_t create_object(struct ce_cdb_t db,
                              uint64_t type) {
    uint64_t uid = _gen_uid();
    create_object_uid(db, uid, type);
    return uid;
}

static ce_cdb_obj_o *write_begin(struct ce_cdb_t db,
                                 uint64_t _obj);

static void write_commit(ce_cdb_obj_o *_writer);

void set_subobject(ce_cdb_obj_o *_writer,
                   uint64_t property,
                   uint64_t subobject);

static bool prop_exist(const ce_cdb_obj_o *reader,
                       uint64_t key) {

    if(!reader) {
        return false;
    }

    struct object_t *obj = _get_object_from_o(reader);

    uint32_t idx = _find_prop_index(obj, key);

    if (idx) {
        return true;
    }

    return false;
}

void prop_copy(const ce_cdb_obj_o *from,
               ce_cdb_obj_o *to,
               uint64_t prorp);

static void set_from(struct ce_cdb_t _db,
                     uint64_t from,
                     uint64_t to) {
    struct db_t *db = _get_db(_db);

    struct object_t *inst = _get_object_from_id(db, to);
    inst->instance_of = from;

}

static uint64_t create_from(struct ce_cdb_t db,
                            uint64_t from);

uint64_t create_from_uid(struct ce_cdb_t db,
                         uint64_t from,
                         uint64_t uid) {
    struct db_t *db_inst = &_G.dbs[db.idx];


    struct object_t *from_obj = _get_object_from_id(db_inst, from);

    struct object_t *inst = _new_object(db_inst, _G.allocator);

    _set_uid_obj(db_inst, uid, inst);

    inst->db = db;

    inst->instance_of = from;
    inst->type = from_obj->type;
    inst->orig_obj = uid;

    ce_array_push(from_obj->instances, uid, _G.allocator);

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(db, from);
    struct object_t *wr = inst;
    for (int i = 1; i < from_obj->properties_count; ++i) {
        uint64_t key = from_obj->keys[i];
        if (from_obj->property_type[i] == CDB_TYPE_SUBOBJECT) {
            union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
                    from_obj->values +
                    from_obj->offset[i]);

            uint64_t old_subobj = value_ptr->subobj;
            uint64_t new_subobj = create_from(db, old_subobj);
            set_subobject(wr, from_obj->keys[i], new_subobj);
        } else {
            if (!prop_exist(inst, key)) {
                prop_copy(reader, inst, key);
            }
        }
    }

    return (uint64_t) uid;
}

static uint64_t create_from(struct ce_cdb_t db,
                            uint64_t from) {
    uint64_t uid = _gen_uid();
    return create_from_uid(db, from, uid);
}

static uint64_t clone(struct ce_cdb_t db,
                      uint64_t from) {
    struct db_t *db_inst = _get_db(db);
    struct object_t *from_obj = _get_object_from_id(db_inst, from);
    struct object_t *new_obj = _object_clone(db_inst, from_obj, _G.allocator);

    uint64_t uid = _gen_uid();

    _set_uid_obj(db_inst, uid, new_obj);

    new_obj->db = db;
    new_obj->orig_obj = uid;


    struct object_t *wr = new_obj;
    for (int i = 1; i < from_obj->properties_count; ++i) {

        if (from_obj->property_type[i] != CDB_TYPE_SUBOBJECT) {
            continue;
        }

        uint64_t key = from_obj->keys[i];

        union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
                from_obj->values +
                from_obj->offset[i]);

        uint64_t from_subobj = value_ptr->subobj;
        uint64_t new_subobj = clone(db, from_subobj);
        set_subobject(wr, key, new_subobj);
    }

    return uid;
}

static void destroy_db(struct ce_cdb_t db) {
    ce_array_push(_G.to_free_db, db.idx, _G.allocator);
}

static void _add_obj_to_destroy_list(struct db_t *db_inst,
                                     uint64_t _obj) {
    bool contain = false;
    const uint64_t n = db_inst->to_free_objects_id_n;
    for (int i = 0; i < n; ++i) {
        if (db_inst->to_free_objects_id[i] != _obj) {
            continue;
        }

        contain = true;
        break;
    }

    if (contain) {
        return;
    }
//    CE_ASSERT(LOG_WHERE, !contain);

    uint64_t idx = atomic_fetch_add(&db_inst->to_free_objects_id_n, 1);
    db_inst->to_free_objects_id[idx] = _obj;
}

static void destroy_object(struct ce_cdb_t db,
                           uint64_t _obj) {
    struct db_t *db_inst = _get_db(db);

    struct object_t *obj = _get_object_from_id(db_inst, _obj);

    CE_ASSERT(LOG_WHERE, obj);

    _add_obj_to_destroy_list(db_inst, _obj);

    for (int i = 1; i < obj->properties_count; ++i) {
        switch (obj->property_type[i]) {
            case CDB_TYPE_SUBOBJECT: {
                union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
                        obj->values +
                        obj->offset[i]);

                uint64_t old_subobj = value_ptr->subobj;
                destroy_object(db, old_subobj);
            }

                break;

            default:
                break;
        }
    }
}

static void gc() {
    const uint32_t fdb_n = ce_array_size(_G.to_free_db);
    for (int i = 0; i < fdb_n; ++i) {
        uint32_t idx = _G.to_free_db[i];
        struct db_t *db_inst = &_G.dbs[idx];

        virt_free(db_inst->to_free_objects_id,
                  MAX_OBJECTS * sizeof(struct object_t **));

        virt_free(db_inst->object_pool,
                  MAX_OBJECTS * sizeof(struct object_t));


        virt_free(db_inst->free_objects,
                  MAX_OBJECTS * sizeof(struct object_t *));

        virt_free(db_inst->to_free_objects,
                  MAX_OBJECTS * sizeof(struct object_t *));

        db_inst->used = false;
    }
    ce_array_clean(_G.to_free_db);

    const uint32_t db_n = ce_array_size(_G.dbs);
    for (int i = 0; i < db_n; ++i) {
        struct db_t *db_inst = &_G.dbs[i];

        if (!db_inst->used) {
            continue;
        }

        uint32_t changed_obj_n = ce_array_size(db_inst->changed_obj);
        for (int j = 0; j < changed_obj_n; ++j) {
            struct object_t *obj = _get_object_from_id(db_inst,
                                                       db_inst->changed_obj[j]);

            if (obj) {
                ce_array_clean(obj->changed);
            } else {
                ce_log_a0->warning(LOG_WHERE, "invalid change obj %llx",
                                   db_inst->changed_obj[j]);
            }

        }
        ce_array_clean(db_inst->changed_obj);
        ce_hash_clean(&db_inst->changed_obj_set);

        const uint32_t to_free_objects_id_n = db_inst->to_free_objects_id_n;

        for (int j = 0; j < to_free_objects_id_n; ++j) {
            uint64_t objid = db_inst->to_free_objects_id[j];

            struct object_t *obj = _get_object_from_id(db_inst, objid);

            // remove from instance from parent instance
            if (obj->instance_of) {
                struct object_t *prefab_obj = \
                    _get_object_from_id(db_inst, obj->instance_of);

                const uint32_t instances_n = \
                    ce_array_size(prefab_obj->instances);

                const uint32_t last_idx = instances_n - 1;
                for (int k = 0; k < instances_n; ++k) {
                    if (prefab_obj->instances[k] != obj->orig_obj) {
                        continue;
                    }

                    prefab_obj->instances[k] = prefab_obj->instances[last_idx];
                    ce_array_pop_back(prefab_obj->instances);

                    break;
                }
            }

            _destroy_object(db_inst, obj);
        }

        for (int j = 0; j < to_free_objects_id_n; ++j) {
            uint64_t objid = db_inst->to_free_objects_id[j];
            _remove_uid_obj(db_inst, objid);

        }

        db_inst->to_free_objects_id_n = 0;

        atomic_ullong to_free_object_n = db_inst->to_free_objects_n;
        for (int j = 0; j < to_free_object_n; ++j) {
            struct object_t *obj = db_inst->to_free_objects[j];

            ce_array_clean(obj->instances);

            ce_array_clean(obj->property_type);
            ce_array_clean(obj->keys);
            ce_array_clean(obj->values);
            ce_array_clean(obj->offset);

            ce_hash_clean(&obj->prop_map);

            ce_array_clean(obj->changed);

            *obj = (struct object_t) {
                    .instances = obj->instances,
                    .property_type = obj->property_type,
                    .keys = obj->keys,
                    .values = obj->values,
                    .offset = obj->offset,
                    .prop_map = obj->prop_map,
                    .changed = obj->changed
            };


            uint64_t fidx = atomic_fetch_add(&db_inst->free_objects_n, 1);
            db_inst->free_objects[fidx] = obj;
        }

        db_inst->to_free_objects_n = 0;
    }

}

struct cdb_binobj_header {
    uint64_t version;
    uint64_t type;
    uint64_t parent;
    uint64_t instance_of;
    uint64_t properties_count;
    uint64_t values_size;
    uint64_t string_buffer_size;
    uint64_t blob_buffer_size;
};

static void dump(struct ce_cdb_t db,
                 uint64_t _obj,
                 char **output,
                 struct ce_alloc *allocator) {
    struct db_t *dbi = _get_db(db);

    struct object_t *obj = _get_object_from_id(dbi, _obj);

    uint64_t *keys = obj->keys;
    uint8_t *type = obj->property_type;
    uint64_t *offset = obj->offset;
    uint8_t *values = obj->values;

    uint8_t *values_copy = CE_ALLOC(allocator, uint8_t, ce_array_size(values));
    memcpy(values_copy, values, ce_array_size(values));

    char *str_buffer = NULL;
    char *blob_buffer = NULL;
    for (int i = 1; i < obj->properties_count; ++i) {
        switch (obj->property_type[i]) {
            case CDB_TYPE_STR: {
                uint64_t stroffset = ce_array_size(str_buffer);
                char *str = *(char **) (values_copy + offset[i]);
                ce_array_push_n(str_buffer, str, strlen(str) + 1, allocator);

                uint64_t *strptr = (uint64_t *) (values_copy + offset[i]);
                *strptr = stroffset;
            }
                break;

            case CDB_TYPE_BLOB: {
                uint64_t bloboffset = ce_array_size(blob_buffer);

                struct ce_cdb_blob_t0 *blob;
                blob = (struct ce_cdb_blob_t0 *) (values_copy + offset[i]);

                ce_array_push_n(blob_buffer, &blob->size, sizeof(uint64_t),
                                allocator);

                ce_array_push_n(blob_buffer, blob->data, blob->size, allocator);

                uint64_t *blobptr = (uint64_t *) (values_copy + offset[i]);
                *blobptr = bloboffset;
            }
                break;
            default:
                break;
        }
    }

    struct cdb_binobj_header header = {
            .type = obj->type,
            .parent = obj->parent,
            .instance_of = obj->instance_of,
            .properties_count = obj->properties_count - 1,
            .values_size = ce_array_size(values),
            .string_buffer_size = ce_array_size(str_buffer),
            .blob_buffer_size = ce_array_size(blob_buffer),
    };

    ce_array_push_n(*output, (char *) &header,
                    sizeof(struct cdb_binobj_header),
                    allocator);

    ce_array_push_n(*output, (char *) (keys + 1),
                    sizeof(uint64_t) * (ce_array_size(keys) - 1),
                    allocator);

    ce_array_push_n(*output, (char *) (type + 1),
                    sizeof(uint8_t) * (ce_array_size(type) - 1),
                    allocator);

    ce_array_push_n(*output, (char *) (offset + 1),
                    sizeof(uint64_t) * (ce_array_size(offset) - 1),
                    allocator);

    ce_array_push_n(*output, (char *) values_copy,
                    ce_array_size(values),
                    allocator);

    ce_array_push_n(*output, (char *) str_buffer,
                    header.string_buffer_size,
                    allocator);

    ce_array_push_n(*output, (char *) blob_buffer,
                    header.blob_buffer_size,
                    allocator);

    CE_FREE(allocator, values_copy);
}


static ce_cdb_obj_o *write_begin(struct ce_cdb_t db,
                                 uint64_t _obj) {
    struct db_t *db_inst = _get_db(db);
    struct object_t *obj = _get_object_from_id(db_inst, _obj);

    struct object_t *new_obj = _object_clone(db_inst, obj, _G.allocator);

    new_obj->orig_obj = _obj;

    return new_obj;
}

static enum ce_cdb_type prop_type(const ce_cdb_obj_o *reader,
                                  uint64_t key) {
    struct object_t *obj = _get_object_from_o(reader);
    uint64_t idx = _find_prop_index(obj, key);

    if (idx) {
        return (enum ce_cdb_type) obj->property_type[idx];
    }

    return CDB_TYPE_NONE;
}

static void _dispatch_instances(struct ce_cdb_t db,
                                struct object_t *orig_obj);

static void write_commit(ce_cdb_obj_o *_writer) {
    struct object_t *writer = _get_object_from_o(_writer);

    struct db_t *db = _get_db(writer->db);

    struct object_t *orig_obj = _get_object_from_id(db, writer->orig_obj);

    _dispatch_instances(writer->db, writer);

    _set_uid_obj(db, writer->orig_obj, writer);
    _destroy_object(db, orig_obj);
}

//static bool write_try_commit(ce_cdb_obj_o *_writer) {
//    struct object_t *writer = _get_object_from_o(_writer);
//    struct object_t *orig_obj = _get_object_from_id(writer->orig_obj);
//
//    struct object_t **obj_addr = (struct object_t **) writer->orig_obj;
//
//    bool ok = atomic_compare_exchange_weak((atomic_ullong *) obj_addr,
//                                           ((uint64_t *) &orig_obj),
//                                           ((uint64_t) writer));
//
//    _destroy_object(orig_obj);
//    return ok;
//}

static void set_float(ce_cdb_obj_o *_writer,
                      uint64_t property,
                      float value) {
    struct object_t *writer = _get_object_from_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_FLOAT, &value,
                                   sizeof(float),
                                   _G.allocator);
    }

    union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
            writer->values +
            writer->offset[idx]);

    _add_change(writer, (struct ce_cdb_change_ev0) {
            .obj = writer->orig_obj,
            .prop=property,
            .type =CE_CDB_CHANGE,
            .prop_type = writer->property_type[idx],
            .old_value = *value_ptr,
            .new_value.f = value,
    });


    value_ptr->f = value;
}

static void set_bool(ce_cdb_obj_o *_writer,
                     uint64_t property,
                     bool value) {
    struct object_t *writer = _get_object_from_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_BOOL, &value,
                                   sizeof(bool),
                                   _G.allocator);
    }

    union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
            writer->values +
            writer->offset[idx]);

    _add_change(writer, (struct ce_cdb_change_ev0) {
            .obj = writer->orig_obj,
            .prop=property,
            .type = CE_CDB_CHANGE,
            .prop_type = writer->property_type[idx],
            .old_value = *value_ptr,
            .new_value.b = value,
    });

    value_ptr->b = value;
}

static void set_string(ce_cdb_obj_o *_writer,
                       uint64_t property,
                       const char *value) {
    struct ce_alloc *a = _G.allocator;

    struct object_t *writer = _get_object_from_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);

    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_STR,
                                   &value, sizeof(char *),
                                   _G.allocator);
    }
//    else {
//        union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
//                writer->values +
//                writer->offset[idx]);
//        CE_FREE(a, value_ptr->str);
//    }

    union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
            writer->values +
            writer->offset[idx]);

    char *value_clone = ce_memory_a0->str_dup(value, a);

    _add_change(writer, (struct ce_cdb_change_ev0) {
            .obj = writer->orig_obj,
            .prop=property,
            .type =CE_CDB_CHANGE,
            .prop_type = writer->property_type[idx],
            .old_value = *value_ptr,
            .new_value.str = value_clone,
    });

    value_ptr->str = value_clone;
}

static void set_uint64(ce_cdb_obj_o *_writer,
                       uint64_t property,
                       uint64_t value) {
    struct object_t *writer = _get_object_from_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property, CDB_TYPE_UINT64,
                                   &value, sizeof(uint64_t), _G.allocator);
    }

    union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
            writer->values +
            writer->offset[idx]);

    _add_change(writer, (struct ce_cdb_change_ev0) {
            .obj = writer->orig_obj,
            .prop=property,
            .type =CE_CDB_CHANGE,
            .prop_type = writer->property_type[idx],
            .old_value = *value_ptr,
            .new_value.uint64 = value,
    });

    value_ptr->uint64 = value;
}

static void set_ptr(ce_cdb_obj_o *_writer,
                    uint64_t property,
                    const void *value) {
    struct object_t *writer = _get_object_from_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property, CDB_TYPE_PTR, &value,
                                   sizeof(void *),
                                   _G.allocator);
    }

    union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
            writer->values +
            writer->offset[idx]);

    _add_change(writer, (struct ce_cdb_change_ev0) {
            .obj = writer->orig_obj,
            .prop=property,
            .type =CE_CDB_CHANGE,
            .prop_type = writer->property_type[idx],
            .old_value = *value_ptr,
            .new_value.ptr = (void *) value,
    });

    value_ptr->ptr = (void *) value;
}

static void set_ref(ce_cdb_obj_o *_writer,
                    uint64_t property,
                    uint64_t ref) {

    struct object_t *writer = _get_object_from_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_REF, &ref,
                                   sizeof(uint64_t),
                                   _G.allocator);
    }

    writer->key = property;


    union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
            writer->values +
            writer->offset[idx]);

    _add_change(writer, (struct ce_cdb_change_ev0) {
            .obj = writer->orig_obj,
            .prop=property,
            .type =CE_CDB_CHANGE,
            .prop_type = writer->property_type[idx],
            .old_value = *value_ptr,
            .new_value.ref = ref,
    });

    value_ptr->ref = ref;
}

uint64_t parent(const ce_cdb_obj_o *reader) {
    struct object_t *obj = _get_object_from_o(reader);
    return obj->parent;
}

void set_subobjectw(ce_cdb_obj_o *_writer,
                    uint64_t property,
                    ce_cdb_obj_o *_subwriter) {
    struct object_t *writer = _get_object_from_o(_writer);
    struct object_t *subwriter = _get_object_from_o(_subwriter);

    subwriter->key = property;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_SUBOBJECT, &subwriter->orig_obj,
                                   sizeof(uint64_t),
                                   _G.allocator);
    }


    union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
            writer->values +
            writer->offset[idx]);

    _add_change(writer, (struct ce_cdb_change_ev0) {
            .obj = writer->orig_obj,
            .prop=property,
            .type =CE_CDB_CHANGE,
            .prop_type = writer->property_type[idx],
            .old_value = *value_ptr,
            .new_value.subobj = subwriter->orig_obj,
    });


    value_ptr->subobj = subwriter->orig_obj;
    subwriter->parent = writer->orig_obj;
}

void set_subobject(ce_cdb_obj_o *_writer,
                   uint64_t property,
                   uint64_t subobject) {
    struct object_t *writer = _get_object_from_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_SUBOBJECT, &subobject,
                                   sizeof(uint64_t),
                                   _G.allocator);
    }


    union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
            writer->values +
            writer->offset[idx]);

    _add_change(writer, (struct ce_cdb_change_ev0) {
            .obj = writer->orig_obj,
            .prop=property,
            .type =CE_CDB_CHANGE,
            .prop_type = writer->property_type[idx],
            .old_value = *value_ptr,
            .new_value.subobj = subobject,
    });

    value_ptr->subobj = subobject;

    if (subobject) {
        struct db_t *db = _get_db(writer->db);

        struct object_t *subobj = _get_object_from_id(db, subobject);
        subobj->parent = writer->orig_obj;
        subobj->key = property;
    }
}

void set_blob(ce_cdb_obj_o *_writer,
              uint64_t property,
              void *blob_data,
              uint64_t blob_size) {
    struct ce_alloc *a = _G.allocator;

    struct object_t *writer = _get_object_from_o(_writer);

    void *new_blob = CE_ALLOC(a, char, blob_size);

    memcpy(new_blob, blob_data, blob_size);

    struct ce_cdb_blob_t0 blob = {
            .size = blob_size,
            .data = new_blob,
    };


    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_BLOB,
                                   &blob, sizeof(struct ce_cdb_blob_t0),
                                   _G.allocator);
    } else {
        union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
                writer->values +
                writer->offset[idx]);
        CE_FREE(a, value_ptr->blob.data);
    }


    union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
            writer->values +
            writer->offset[idx]);

    _add_change(writer, (struct ce_cdb_change_ev0) {
            .obj = writer->orig_obj,
            .prop=property,
            .type =CE_CDB_CHANGE,
            .prop_type = writer->property_type[idx],
            .old_value = *value_ptr,
            .new_value.blob = blob,
    });

    value_ptr->blob = blob;
}

static bool prop_exist(const ce_cdb_obj_o *reader,
                       uint64_t key);

void remove_property(ce_cdb_obj_o *_writer,
                     uint64_t property) {
    struct object_t *writer = _get_object_from_o(_writer);

    if (!prop_exist(writer, property)) {
        return;
    }

    uint64_t idx = _find_prop_index(writer, property);

    if (idx) {
        union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
                writer->values +
                writer->offset[idx]);


        _add_change(writer, (struct ce_cdb_change_ev0) {
                .obj = writer->orig_obj,
                .prop=property,
                .type =CE_CDB_REMOVE,
                .prop_type = writer->property_type[idx],
                .old_value = *value_ptr,
        });

        uint64_t last_idx = --writer->properties_count;
        ce_hash_add(&writer->prop_map, writer->keys[last_idx], idx,
                    _G.allocator);

        writer->keys[idx] = writer->keys[last_idx];
        writer->property_type[idx] = writer->property_type[last_idx];
        writer->offset[idx] = writer->offset[last_idx];

        ce_array_pop_back(writer->keys);
        ce_array_pop_back(writer->property_type);
        ce_array_pop_back(writer->offset);

        if (writer->property_type[idx] == CDB_TYPE_SUBOBJECT) {
            destroy_object(writer->db, value_ptr->subobj);
        }

        ce_hash_remove(&writer->prop_map, property);
    }
}

const ce_cdb_obj_o *read(struct ce_cdb_t _db,
                         uint64_t object) {
    if (!object) {
        return NULL;
    }

    struct db_t *db = _get_db(_db);
    struct object_t *obj = _get_object_from_id(db, object);

    return obj;
}

static float read_float(const ce_cdb_obj_o *reader,
                        uint64_t property,
                        float defaultt) {
    if (!reader) {
        return defaultt;
    }

    struct object_t *obj = _get_object_from_o(reader);

    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(float *) (obj->values + obj->offset[idx]);
    }

    return defaultt;
}

static bool read_bool(const ce_cdb_obj_o *reader,
                      uint64_t property,
                      bool defaultt) {
    if (!reader) {
        return defaultt;
    }

    struct object_t *obj = _get_object_from_o(reader);

    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(bool *) (obj->values + obj->offset[idx]);
    }

    return defaultt;
}

static const char *read_string(const ce_cdb_obj_o *reader,
                               uint64_t property,
                               const char *defaultt) {
    if (!reader) {
        return defaultt;
    }

    struct object_t *obj = _get_object_from_o(reader);

    uint64_t idx = _find_prop_index(obj, property);
    if (idx) {
        return *(const char **) (obj->values + obj->offset[idx]);
    }

    return defaultt;
}


static uint64_t read_uint64(const ce_cdb_obj_o *reader,
                            uint64_t property,
                            uint64_t defaultt) {
    if (!reader) {
        return defaultt;
    }

    struct object_t *obj = _get_object_from_o(reader);

    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(uint64_t *) (obj->values + obj->offset[idx]);
    }

    return defaultt;
}

static void *read_ptr(const ce_cdb_obj_o *reader,
                      uint64_t property,
                      void *defaultt) {
    if (!reader) {
        return defaultt;
    }

    struct object_t *obj = _get_object_from_o(reader);

    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(void **) (obj->values + obj->offset[idx]);
    }


    return defaultt;
}

static uint64_t read_ref(const ce_cdb_obj_o *reader,
                         uint64_t property,
                         uint64_t defaultt) {
    if (!reader) {
        return defaultt;
    }

    struct object_t *obj = _get_object_from_o(reader);

    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(uint64_t *) (obj->values + obj->offset[idx]);
    }

    return defaultt;
}

static uint64_t read_subobject(const ce_cdb_obj_o *reader,
                               uint64_t property,
                               uint64_t defaultt) {
    if (!reader) {
        return defaultt;
    }

    struct object_t *obj = _get_object_from_o(reader);

    uint64_t idx = _find_prop_index(obj, property);

    uint64_t subobj = 0;

    if (idx) {
        subobj = *(uint64_t *) (obj->values + obj->offset[idx]);
        return subobj;
    }

    return defaultt;
}

void *read_blob(const ce_cdb_obj_o *reader,
                uint64_t property,
                uint64_t *size,
                void *defaultt) {
    if (!reader) {
        return defaultt;
    }

    struct object_t *obj = _get_object_from_o(reader);

    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        void *data = (obj->values + obj->offset[idx]);

        struct ce_cdb_blob_t0 *blob = data;

        if (size) {
            *size = blob->size;
        }

        return blob->data;
    }

    return defaultt;
}

static const uint64_t *prop_keys(const ce_cdb_obj_o *reader) {
    struct object_t *obj = _get_object_from_o(reader);

    if (!obj) {
        return NULL;
    }

    return obj->keys + 1;
}

static uint64_t prop_count(const ce_cdb_obj_o *reader) {
    struct object_t *obj = _get_object_from_o(reader);

    if (!obj) {
        return 0;
    }

    uint64_t count = obj->properties_count - 1;

    return count;
}


bool prop_equal(const ce_cdb_obj_o *r1,
                const ce_cdb_obj_o *r2,
                uint64_t prorp) {
    enum ce_cdb_type t = prop_type(r1, prorp);

    if (!prop_exist(r1, prorp) || !prop_exist(r2, prorp)) {
        return false;
    }

    switch (t) {
        case CDB_TYPE_NONE:
            break;
        case CDB_TYPE_UINT64:
            return read_uint64(r1, prorp, 0) == read_uint64(r2, prorp, 0);
            break;
        case CDB_TYPE_PTR:
            return read_ptr(r1, prorp, 0) == read_ptr(r2, prorp, 0);
            break;
        case CDB_TYPE_REF:
            return read_ref(r1, prorp, 0) == read_ref(r2, prorp, 0);
            break;
        case CDB_TYPE_FLOAT:
            return read_float(r1, prorp, 0) == read_float(r2, prorp, 0);
            break;
        case CDB_TYPE_BOOL:
            return read_bool(r1, prorp, 0) == read_bool(r2, prorp, 0);
            break;

        case CDB_TYPE_STR:
            return !strcmp(read_string(r1, prorp, 0),
                           read_string(r2, prorp, 0));
            break;
        case CDB_TYPE_SUBOBJECT:
            return read_subobject(r1, prorp, 0) == read_subobject(r2, prorp, 0);
            break;
        case CDB_TYPE_BLOB:
            break;
    }

    return false;
}

void prop_copy(const ce_cdb_obj_o *from,
               ce_cdb_obj_o *to,
               uint64_t prorp) {
    enum ce_cdb_type t = prop_type(from, prorp);
    union ce_cdb_value_u0 v;
    switch (t) {
        case CDB_TYPE_NONE:
            break;

        case CDB_TYPE_UINT64:
            v.uint64 = read_uint64(from, prorp, 0);
            set_uint64(to, prorp, v.uint64);
            break;
        case CDB_TYPE_PTR:
            v.ptr = read_ptr(from, prorp, 0);
            set_ptr(to, prorp, v.ptr);
            break;
        case CDB_TYPE_REF:
            v.ref = read_ref(from, prorp, 0);
            set_ref(to, prorp, v.ref);
            break;
        case CDB_TYPE_FLOAT:
            v.f = read_float(from, prorp, 0);
            set_float(to, prorp, v.f);
            break;
        case CDB_TYPE_BOOL:
            v.b = read_bool(from, prorp, 0);
            set_bool(to, prorp, v.b);
            break;

        case CDB_TYPE_STR:
            v.str = (char *) read_string(from, prorp, 0);
            set_string(to, prorp, v.str);
            break;

        case CDB_TYPE_SUBOBJECT:
            v.subobj = read_subobject(from, prorp, 0);
            set_uint64(to, prorp, v.subobj);
            break;

        case CDB_TYPE_BLOB:
            break;
    }
}

static struct ce_cdb_t global_db() {
    return _G.global_db;
}

static uint64_t type(const ce_cdb_obj_o *reader) {
    struct object_t *obj = _get_object_from_o(reader);

    return obj->type;
}

void set_type(struct ce_cdb_t _db,
              uint64_t _obj,
              uint64_t type) {
    if (!_obj) {
        return;
    }

    struct db_t *db = _get_db(_db);

    struct object_t *obj = _get_object_from_id(db, _obj);
    obj->type = type;
}


uint64_t key(struct ce_cdb_t _db,
             uint64_t _obj) {
    struct db_t *db = _get_db(_db);

    struct object_t *obj = _get_object_from_id(db, _obj);
    return obj->key;
}

void move(struct ce_cdb_t _db,
          uint64_t _from_obj,
          uint64_t _to) {
    struct db_t *db = _get_db(_db);

    struct object_t *from = _get_object_from_id(db, _from_obj);

    struct object_t **obj_addr = (struct object_t **) _to;

    *obj_addr = from;
}


static void _push_space(char **buffer,
                        uint32_t level) {
    for (int j = 0; j < level; ++j) {
        ce_buffer_printf(buffer, _G.allocator, "  ");
    }
}

static void dump_str(struct ce_cdb_t _db,
                     char **buffer,
                     uint64_t from,
                     uint32_t level) {
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(_db, from);

    const uint32_t prop_count = ce_cdb_a0->prop_count(reader);
    const uint64_t *keys = ce_cdb_a0->prop_keys(reader);

    _push_space(buffer, level);
    ce_buffer_printf(buffer, _G.allocator, "cdb_uid: 0x%llx\n", from);

    uint64_t type = ce_cdb_a0->obj_type(reader);
    const char *type_str = ce_id_a0->str_from_id64(type);
    if (type_str) {
        _push_space(buffer, level);
        ce_buffer_printf(buffer, _G.allocator, "cdb_type: %s\n", type_str);
    } else {
        if (type) {
            _push_space(buffer, level);
            ce_buffer_printf(buffer, _G.allocator, "cdb_type: 0x%llx\n", type);
        }
    }

    uint64_t instance_of = ce_cdb_a0->read_instance_of(reader);
    if (instance_of) {
        _push_space(buffer, level);
        ce_buffer_printf(buffer, _G.allocator, "cdb_instance: 0x%llx\n",
                         instance_of);
    }

    for (int i = 0; i < prop_count; ++i) {
        uint64_t key = keys[i];

        if (key == CDB_INSTANCE_PROP) {
            continue;
        }

        if (instance_of) {
            const ce_cdb_obj_o *ir = ce_cdb_a0->read(_db, instance_of);
            if (prop_equal(ir, reader, key)) {
                continue;
            }
        }

        const char *k = ce_id_a0->str_from_id64(key);

        enum ce_cdb_type type = ce_cdb_a0->prop_type(reader, key);


        if ((type == CDB_TYPE_BLOB) || (type == CDB_TYPE_PTR)) {
            continue;
        }


        _push_space(buffer, level);

        if (k) {
            ce_buffer_printf(buffer, _G.allocator, "%s:", k);
        } else {
            ce_buffer_printf(buffer, _G.allocator, "0x%llx:", key);
        }


        switch (type) {
            case CDB_TYPE_SUBOBJECT: {
                uint64_t s = ce_cdb_a0->read_subobject(reader, key, 0);
                ce_buffer_printf(buffer, _G.allocator, "\n");
                dump_str(_db, buffer, s, level + 1);
            }
                break;

            case CDB_TYPE_FLOAT: {
                float f = ce_cdb_a0->read_float(reader, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " %f\n", f);
            }
                break;

            case CDB_TYPE_STR: {
                const char *s = ce_cdb_a0->read_str(reader, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " %s\n", s);
            }
                break;

            case CDB_TYPE_BOOL: {
                bool b = ce_cdb_a0->read_bool(reader, key, 0);
                if (b) {
                    ce_buffer_printf(buffer, _G.allocator, " true\n");
                } else {
                    ce_buffer_printf(buffer, _G.allocator, " false\n");
                }
            }
                break;

            case CDB_TYPE_NONE: {
                ce_buffer_printf(buffer, _G.allocator, " none\n");
            }
                break;

            case CDB_TYPE_UINT64: {
                uint64_t i = ce_cdb_a0->read_uint64(reader, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " %llu\n", i);
            }
                break;

            case CDB_TYPE_REF: {
                uint64_t ref = ce_cdb_a0->read_ref(reader, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " 0x%llx\n", ref);
            }
                break;

            default:
                break;
        }
    }

}

static uint64_t find_root(struct ce_cdb_t _db,
                          uint64_t _obj) {
    struct db_t *db = _get_db(_db);

    struct object_t *obj = _get_object_from_id(db, _obj);

    if (!obj->parent) {
        return _obj;
    }

    return find_root(_db, obj->parent);
}

static void _dispatch_instances(struct ce_cdb_t db,
                                struct object_t *orig_obj) {
    const int changed_prop_n = ce_array_size(orig_obj->changed);

    if (!changed_prop_n) {
        return;
    }

    const int instances_n = ce_array_size(orig_obj->instances);
    for (int i = 0; i < instances_n; ++i) {
        uint64_t inst_obj = orig_obj->instances[i];

        ce_cdb_obj_o *w = write_begin(db, inst_obj);
        const uint32_t chn = ce_array_size(orig_obj->changed);
        for (int j = 0; j < chn; ++j) {
            struct ce_cdb_change_ev0 *ev = &orig_obj->changed[j];
            enum ce_cdb_type t = prop_type(orig_obj, ev->prop);

            if (ev->type == CE_CDB_CHANGE) {
                switch (t) {
                    case CDB_TYPE_NONE:
                        break;

                    case CDB_TYPE_UINT64: {
                        uint64_t u = read_uint64(w, ev->prop, 0);
                        if (u == ev->old_value.uint64) {
                            set_uint64(w, ev->prop, ev->new_value.uint64);
                        }
                    }
                        break;

                    case CDB_TYPE_FLOAT: {
                        float f = read_float(w, ev->prop, 0);
                        if (f == ev->old_value.f) {
                            set_float(w, ev->prop, ev->new_value.f);
                        }
                    }
                        break;

                    case CDB_TYPE_BOOL: {
                        bool b = read_bool(w, ev->prop, 0);
                        if (b == ev->old_value.b) {
                            set_bool(w, ev->prop, ev->new_value.b);
                        }
                    }
                        break;

                    case CDB_TYPE_STR: {
                        const char *str = read_string(w, ev->prop, 0);
                        if (!str || !strcmp(str, ev->old_value.str)) {
                            set_string(w, ev->prop, ev->new_value.str);
                        }
                    }
                        break;

                    case CDB_TYPE_PTR: {
                        void *ptr = read_ptr(w, ev->prop, 0);
                        if (ptr == ev->old_value.ptr) {
                            set_ptr(w, ev->prop, ev->new_value.ptr);
                        }
                    }
                        break;

                    case CDB_TYPE_SUBOBJECT: {
                        uint64_t new_so = create_from(db,
                                                      ev->new_value.subobj);
                        set_subobject(w, ev->prop, new_so);
                    }

                        break;

                    case CDB_TYPE_REF: {
                        uint64_t ref = read_ref(w, ev->prop, 0);
                        if (ref == ev->old_value.ref) {
                            set_ref(w, ev->prop, ev->new_value.ref);
                        }
                    }
                        break;

                    case CDB_TYPE_BLOB:
                        break;

                    default:
                        break;
                }
            } else if (ev->type == CE_CDB_REMOVE) {
                remove_property(w, ev->prop);
            }

        }
        write_commit(w);
    }
}

const struct ce_cdb_change_ev0 *changed(const ce_cdb_obj_o *reader,
                                        uint32_t *n) {
    struct object_t *obj = (struct object_t *) reader;
    *n = ce_array_size(obj->changed);
    if (*n) {
        return obj->changed;
    }
    return NULL;
}

const uint64_t * changed_objects(struct ce_cdb_t _db, uint32_t* n) {
    struct db_t *db = _get_db(_db);


    uint32_t ch_n = ce_array_size(db->changed_obj);
    if(!ch_n) {
        *n = 0;
        return NULL;
    }

    *n = ch_n;
    return db->changed_obj;
}

void set_parent(struct ce_cdb_t _db,
                uint64_t object,
                uint64_t parent) {
    struct db_t *db = _get_db(_db);

    struct object_t *from = _get_object_from_id(db, object);
    from->parent = parent;
}

uint64_t read_instance_of(const ce_cdb_obj_o *reader) {
    struct object_t *obj = _get_object_from_o(reader);
    return obj->instance_of;
}

static void load(struct ce_cdb_t db,
                 const char *input,
                 uint64_t _obj,
                 struct ce_alloc *allocator) {
    struct db_t *db_inst = _get_db(db);


    const struct cdb_binobj_header *header;
    header = (const struct cdb_binobj_header *) input;

    uint64_t instanceof = header->instance_of;

    if (instanceof) {
        create_from_uid(db, instanceof, _obj);

        struct object_t *instance_obj = _get_object_from_id(db_inst,
                                                            instanceof);
        if (instance_obj) {
            ce_array_push(instance_obj->instances, _obj, _G.allocator);
        } else {
            ce_log_a0->warning(LOG_WHERE,
                               "Unresolved instance of 0x%llx for obj 0x%llx",
                               instanceof, _obj);
        }
    } else {
        create_object_uid(db, _obj, header->type);
    }

    struct object_t *obj = _get_object_from_id(db_inst, _obj);
    obj->parent = header->parent;


    uint64_t *keys = (uint64_t *) (header + 1);
    uint8_t *ptype = (uint8_t *) (keys + header->properties_count);
    uint64_t *offset = (uint64_t *) (ptype + header->properties_count);
    uint8_t *values = (uint8_t *) (offset + header->properties_count);
    const char *strbuffer = (char *) (values + header->values_size);
    const char *blob_buffer = (char *) (strbuffer +
                                        header->string_buffer_size);
    if (!header->properties_count) {
        return;
    }

    if (!instanceof) {
        ce_array_push_n(obj->keys, keys,
                        header->properties_count,
                        allocator);

        ce_array_push_n(obj->property_type, ptype,
                        header->properties_count,
                        allocator);

        ce_array_push_n(obj->offset, offset,
                        header->properties_count,
                        allocator);

        ce_array_push_n(obj->values, values,
                        header->values_size,
                        allocator);

        obj->properties_count += header->properties_count;

        for (int i = 1; i < obj->properties_count; ++i) {
            ce_hash_add(&obj->prop_map, obj->keys[i], i, allocator);
        }

        for (int i = 1; i < obj->properties_count; ++i) {
            switch (obj->property_type[i]) {
                case CDB_TYPE_STR: {
                    uint64_t str_offset = *(uint64_t *) (obj->values +
                                                         obj->offset[i]);

                    char *dup_str = ce_memory_a0->str_dup(
                            strbuffer + str_offset,
                            allocator);

                    union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
                            obj->values +
                            obj->offset[i]);
                    value_ptr->str = dup_str;
                }

                    break;

                case CDB_TYPE_BLOB: {
                    uint64_t blob_offset = *(uint64_t *) (obj->values +
                                                          obj->offset[i]);

                    uint64_t size = *((uint64_t *) (blob_buffer + blob_offset));
                    const char *blob_data = ((blob_buffer +
                                              blob_offset + sizeof(uint64_t)));

                    char *copy_blob_data = CE_ALLOC(allocator, char, size);
                    memcpy(copy_blob_data, blob_data, size);

                    union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
                            obj->values +
                            obj->offset[i]);
                    value_ptr->blob.size = size;
                    value_ptr->blob.data = copy_blob_data;
                }
                    break;
                default:
                    break;
            }
        }
    } else {

        struct object_t fakeo = {
                .properties_count  = header->properties_count,
                .keys = keys,
                .property_type = ptype,
                .values = values,
                .offset = offset,
        };

        struct object_t *fakeop = &fakeo;

        uint32_t prop_n = header->properties_count;
        for (int i = 0; i < prop_n; ++i) {
            switch (ptype[i]) {
                case CDB_TYPE_STR: {
                    uint64_t str_offset = *(uint64_t *) (fakeop->values +
                                                         fakeop->offset[i]);

                    char *dup_str = ce_memory_a0->str_dup(
                            strbuffer + str_offset,
                            allocator);

                    union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
                            fakeop->values +
                            fakeop->offset[i]);
                    value_ptr->str = dup_str;
                }

                    break;

                case CDB_TYPE_BLOB: {
                    uint64_t blob_offset = *(uint64_t *) (fakeop->values +
                                                          fakeop->offset[i]);

                    uint64_t size = *((uint64_t *) (blob_buffer + blob_offset));
                    const char *blob_data = ((blob_buffer +
                                              blob_offset + sizeof(uint64_t)));

                    char *copy_blob_data = CE_ALLOC(allocator, char, size);
                    memcpy(copy_blob_data, blob_data, size);

                    union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
                            fakeop->values +
                            fakeop->offset[i]);

                    value_ptr->blob.size = size;
                    value_ptr->blob.data = copy_blob_data;
                }
                    break;
            }
        }

        for (int i = 0; i < prop_n; ++i) {
            uint64_t name = keys[i];

            enum ce_cdb_type t = ptype[i];

            union ce_cdb_value_u0 *value_ptr = (union ce_cdb_value_u0 *) (
                    fakeop->values +
                    fakeop->offset[i]);

            switch (t) {
                case CDB_TYPE_UINT64:
                    set_uint64(obj, name, value_ptr->uint64);
                    break;
                case CDB_TYPE_PTR:
                    set_ptr(obj, name, value_ptr->ptr);
                    break;
                case CDB_TYPE_REF:
                    set_ref(obj, name, value_ptr->ref);
                    break;
                case CDB_TYPE_FLOAT:
                    set_float(obj, name, value_ptr->f);
                    break;
                case CDB_TYPE_BOOL:
                    set_bool(obj, name, value_ptr->b);
                    break;
                case CDB_TYPE_STR:
                    set_string(obj, name, value_ptr->str);
                    break;
                case CDB_TYPE_SUBOBJECT:
                    set_uint64(obj, name, value_ptr->subobj);
                    break;
                case CDB_TYPE_BLOB:
                    break;
                case CDB_TYPE_NONE:
                    break;
                default:
                    break;
            }
        }
    }
}

static struct ce_cdb_a0 cdb_api = {
        .create_db = create_db,
        .destroy_db = destroy_db,

        .set_loader = set_loader,
        .db  = global_db,
        .obj_type = type,
        .set_type = set_type,
        .obj_key= key,
        .move = move,

        .create_object = create_object,
        .create_object_uid = create_object_uid,
        .create_from = create_from,
        .clone = clone,
        .set_from = set_from,
        .destroy_object = destroy_object,

        .gc = gc,

        .dump_str = dump_str,
        .dump = dump,
        .load = load,

        .find_root = find_root,
        .prop_exist = prop_exist,
        .prop_type = prop_type,
        .prop_keys = prop_keys,
        .prop_count = prop_count,
        .prop_equal = prop_equal,
        .prop_copy = prop_copy,

        .parent = parent,
        .set_parent = set_parent,

        .read = read,
        .read_instance_of = read_instance_of,
        .changed = changed,
        .changed_objects = changed_objects,
        .read_float = read_float,
        .read_bool = read_bool,
        .read_str = read_string,
        .read_uint64 = read_uint64,
        .read_ptr = read_ptr,
        .read_ref = read_ref,
        .read_blob = read_blob,
        .read_subobject = read_subobject,

        .write_begin = write_begin,
        .write_commit = write_commit,
//        .write_try_commit = write_try_commit,

        .set_float = set_float,
        .set_bool = set_bool,
        .set_str = set_string,
        .set_uint64 = set_uint64,
        .set_ptr = set_ptr,
        .set_ref = set_ref,
        .set_subobject = set_subobject,
        .set_subobjectw = set_subobjectw,
        .set_blob = set_blob,

        .remove_property = remove_property,
};

struct ce_cdb_a0 *ce_cdb_a0 = &cdb_api;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

//    _G.id_map.n = MAX_OBJECTS;
//    _G.id_map.values = (uint64_t *) virt_alloc(MAX_OBJECTS * sizeof(uint64_t));
//    _G.id_map.keys = (uint64_t *) virt_alloc(MAX_OBJECTS * sizeof(uint64_t));

    _G.global_db = create_db();

    api->register_api(CE_CDB_API, &cdb_api);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CE_MODULE_DEF(
        cdb,
        {

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