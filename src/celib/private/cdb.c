// TODO: X( non optimal braindump code
// TODO: remove locks

#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <time.h>
#include <math.h>

#include <sys/mman.h>

#include <celib/macros.h>
#include <celib/api.h>
#include <celib/memory/allocator.h>
#include <celib/memory/memory.h>
#include <celib/log.h>
#include <celib/cdb.h>
#include <celib/module.h>
#include <celib/containers/hash.h>
#include <celib/containers/array.h>
#include <celib/containers/bitset.h>
#include <celib/containers/spsc.h>
#include <celib/containers/mpmc.h>

#include <celib/os/thread.h>
#include <celib/containers/buffer.h>
#include <celib/id.h>

#include <sys/time.h>

#include <celib/os/time.h>

#define _G coredb_global
#define LOG_WHERE "cdb"

#define MAX_OBJECTS 1000000000ULL
#define MAX_OBJECTS_PER_TYPE 1000000ULL
#define MAX_EVENTS_LISTENER 1024
#define MAX_QUEUE_SIZE 1024 * 64

#define UID_HASHMAP

typedef struct type_info_t {
    size_t size;
    size_t align;
} type_info_t;

static type_info_t _TYPE_INFO[] = {
        [CE_CDB_TYPE_NONE] = {},
        [CE_CDB_TYPE_UINT64] = {.size = sizeof(uint64_t), .align = CE_ALIGNOF(uint64_t)},
        [CE_CDB_TYPE_PTR] = {.size = sizeof(void *), .align = CE_ALIGNOF(void *)},
        [CE_CDB_TYPE_REF] = {.size = sizeof(uint64_t), .align = CE_ALIGNOF(uint64_t)},
        [CE_CDB_TYPE_FLOAT] = {.size = sizeof(float), .align = CE_ALIGNOF(float)},
        [CE_CDB_TYPE_BOOL] = {.size = sizeof(bool), .align = CE_ALIGNOF(bool)},
        [CE_CDB_TYPE_STR] = {.size = sizeof(char *), .align = CE_ALIGNOF(char *)},
        [CE_CDB_TYPE_SUBOBJECT] = {.size = sizeof(uint64_t), .align = CE_ALIGNOF(uint64_t)},
        [CE_CDB_TYPE_BLOB] = {.size = sizeof(void *), .align = CE_ALIGNOF(void *)},
        [CE_CDB_TYPE_SET_SUBOBJECT] = {.size = sizeof(uint64_t), .align = CE_ALIGNOF(uint64_t)},
};


static void *virt_alloc(uint64_t size) {
    return CE_REALLOC(ce_memory_a0->virt_system, void, NULL, size, 0);
}

static int virt_free(void *addr,
                     uint64_t size) {
    return CE_REALLOC(ce_memory_a0->virt_system, void, addr, 0, size) != NULL;
}

typedef struct type_storage_t {
    uint64_t type;
    uint64_t type_size;
    uint8_t *pool;
    uint64_t flags;
    ce_mpmc_queue_t0 free_idx;
    ce_mpmc_queue_t0 to_free_idx;

    ce_hash_t prop_idx;
    ce_cdb_type_def_t0 *prop_def;
    uint64_t *prop_name;
    uint8_t *prop_type;
    uint32_t *prop_offset;

    atomic_uint_fast32_t pool_n;
} type_storage_t;

typedef struct listener_pack_t {
    ce_mpmc_queue_t0 *queues;
    atomic_uint_fast16_t n;
} listener_pack_t;

typedef struct object_t {
    ce_cdb_t0 db;
    struct object_t **id;
    uint64_t type;
    uint64_t flags;

    // prefab
    uint64_t instance_of;
    uint64_t *instances;

    // hierarchy
    uint64_t key;
    uint64_t parent;

    // writer
    uint64_t orig_obj;
    ce_cdb_prop_ev_t0 *changed;

    // Typed
    uint32_t typed_obj_idx;
    uint32_t typed_writer_idx;
    type_storage_t *storage;

    //events
    listener_pack_t obj_listeners;
} object_t;


typedef struct set_t {
    ce_hash_t set;
    uint64_t *objs;
} set_t;


typedef struct db_t {
    uint32_t used;
    uint32_t idx;
    uint64_t max_objects;

    // changed
    ce_spinlock_t0 change_lock;
    ce_hash_t changed_obj_set;

    uint64_t *changed_obj;

    // to free uid
    uint64_t *to_free_objects_uid;
    atomic_ullong to_free_objects_uid_n;

    // objects
    object_t *object_pool;
    ce_mpmc_queue_t0 free_objects;
    ce_mpmc_queue_t0 to_free_objects;

    atomic_ullong object_pool_n;

    // objects id
    object_t **object_id_pool;
    object_t ***free_objects_id;
    atomic_ullong object_id_pool_n;
    atomic_ullong free_objects_id_n;

#ifdef UID_HASHMAP
    ce_hash_t uid_hash;
    ce_spinlock_t0 uid_hash_lock;
#else
#endif

    // blobs
    ce_cdb_blob_t0 *blobs;

    // sets
    set_t *sets;

    // Typed object
    ce_hash_t type_map;
    atomic_uint_fast32_t type_n;
    type_storage_t *type_storage;

    // chnaged_queues
    listener_pack_t chnaged_objs;
    listener_pack_t obj_listeners;
} db_t;

typedef struct type_defs_t {
    ce_hash_t def_map;
    ce_cdb_type_def_t0 *defs;
} type_defs_t;

static struct _G {
    db_t *dbs;
    uint32_t *to_free_db;

    ce_alloc_t0 *allocator;
    ce_cdb_t0 global_db;

    ct_cdb_obj_loader_t0 loader;
    type_defs_t type_defs;
} _G;

static object_t **_get_uid_objid(db_t *db,
                                 uint64_t uid) {
    CE_ASSERT(LOG_WHERE, uid != 0);

#ifdef UID_HASHMAP
    ce_os_thread_a0->spin_lock(&db->uid_hash_lock);
    object_t **obj = (object_t **) ce_hash_lookup(&db->uid_hash, uid, 0);
    ce_os_thread_a0->spin_unlock(&db->uid_hash_lock);
    return obj;
#else
#endif
}


// U/ID

static uint64_t _next_uid(uint64_t epoch_offset,
                          uint16_t session) {
    static atomic_ushort g_seq;
    union {
        uint64_t ui;
        struct {
            uint64_t seq:16;
            uint64_t session:8; //8
            uint64_t timestamp:40;
        };
    } id = {.ui = 0};

    id.timestamp = ce_os_time_a0->timestamp_ms() - epoch_offset;
    id.session = session;
    id.seq = atomic_fetch_add(&g_seq, 1) + 1;

//    ce_log_a0->debug(LOG_WHERE, "New UID 0x%llx", id.ui);

    return id.ui * 11400714819323198549UL;
}

static uint64_t _gen_uid() {
    return _next_uid(1547316500ULL, 1);
}

static uint64_t gen_uid(ce_cdb_t0 db) {
    return _gen_uid();
}

static void _set_uid_objid(db_t *db,
                           uint64_t uid,
                           object_t **obj) {
    CE_ASSERT(LOG_WHERE, uid != 0);
    CE_ASSERT(LOG_WHERE, obj != 0);

#ifdef UID_HASHMAP
    ce_os_thread_a0->spin_lock(&db->uid_hash_lock);
    ce_hash_add(&db->uid_hash, uid, (uint64_t) obj, _G.allocator);
    ce_os_thread_a0->spin_unlock(&db->uid_hash_lock);
#else
#endif
}

void _remove_uid_obj(db_t *db,
                     uint64_t uid) {
#ifdef UID_HASHMAP
    ce_os_thread_a0->spin_lock(&db->uid_hash_lock);
    ce_hash_remove(&db->uid_hash, uid);
    ce_os_thread_a0->spin_unlock(&db->uid_hash_lock);
#else
#endif
}

bool _uid_loaded(db_t *db,
                 uint64_t uid) {

#ifdef UID_HASHMAP
    ce_os_thread_a0->spin_lock(&db->uid_hash_lock);
    bool contain = ce_hash_contain(&db->uid_hash, uid);
    ce_os_thread_a0->spin_unlock(&db->uid_hash_lock);
    return contain;
#else
#endif
}

struct object_t **_get_objectid_from_uid(db_t *db,
                                         uint64_t uid) {
    if (!uid) {
        return NULL;
    }

    if (!_uid_loaded(db, uid)) {
        return NULL;
    }

    return _get_uid_objid(db, uid);
}

static struct object_t *_get_object_from_uid(db_t *db,
                                             uint64_t uid) {
    if (!uid) {
        return NULL;
    }

    object_t **objid = _get_objectid_from_uid(db, uid);

    if (!objid) {
        if (!_G.loader((ce_cdb_t0) {db->idx}, uid)) {
            return NULL;
        }

        object_t **objid = _get_objectid_from_uid(db, uid);
        if (!objid) {
            return NULL;
        }

        return *objid;
    }

    return *objid;
}

// events
void _init_listener_pack(listener_pack_t *pack) {
    pack->queues = virt_alloc(sizeof(ce_mpmc_queue_t0) * MAX_EVENTS_LISTENER);
}

ce_mpmc_queue_t0 *_new_listener(listener_pack_t *pack,
                                uint32_t queue_size,
                                size_t item_size) {
    uint32_t idx = atomic_fetch_add(&pack->n, 1);
    ce_mpmc_queue_t0 *q = &pack->queues[idx];
    ce_mpmc_init(q, queue_size, item_size, _G.allocator);
    return q;
}

static void _push_event(listener_pack_t *pack,
                        void *event) {
    uint32_t n = pack->n;
    for (int i = 0; i < n; ++i) {
        ce_mpmc_queue_t0 *q = &pack->queues[i];
        ce_mpmc_enqueue(q, event);
    }
}

ce_mpmc_queue_t0 *_new_changed_obj_events_listener(db_t *db) {
    return _new_listener(&db->chnaged_objs, MAX_QUEUE_SIZE, sizeof(ce_cdb_ev_t0));
}


ce_mpmc_queue_t0 *_new_obj_events_listener(db_t *db) {
    return _new_listener(&db->obj_listeners, MAX_QUEUE_SIZE, sizeof(ce_cdb_prop_ev_t0));
}

ce_mpmc_queue_t0 *_new_obj_events_listener2(db_t *db,
                                            uint64_t _obj) {
    object_t *obj = _get_object_from_uid(db, _obj);
    return _new_listener(&obj->obj_listeners, 64, sizeof(ce_cdb_prop_ev_t0));
}


// objid
object_t **_new_obj_id(db_t *db) {
    uint32_t idx = 0;
    if (db->free_objects_id_n) {
        idx = atomic_fetch_sub(&db->free_objects_id_n, 1) - 1;
        return db->free_objects_id[idx];

    } else {
        idx = atomic_fetch_add(&db->object_id_pool_n, 1);
    }

    return &db->object_id_pool[idx];
}

void _free_obj_id(db_t *db,
                  object_t **obj) {
    uint64_t idx = atomic_fetch_add(&db->free_objects_id_n, 1);
    db->free_objects_id[idx] = obj;
}

// blob
uint32_t _new_blob(db_t *db) {
    uint32_t idx = ce_array_size(db->blobs);
    ce_array_push(db->blobs, (ce_cdb_blob_t0) {}, _G.allocator);
    return idx;
}

ce_cdb_blob_t0 *_get_blob(db_t *db,
                          uint32_t idx) {
    return &db->blobs[idx];
}

// sets
uint32_t _new_set(db_t *db) {
    uint32_t idx = ce_array_size(db->sets);
    ce_array_push(db->sets, (set_t) {}, _G.allocator);
    return idx;
}

set_t *_get_set(db_t *db,
                uint32_t idx) {
    if (!idx) {
        return NULL;
    }

    return &db->sets[idx];
}

void _add_to_set(db_t *db,
                 uint32_t idx,
                 uint64_t obj) {
    set_t *set = _get_set(db, idx);

    if (!set) {
        return;
    }

    if (ce_hash_contain(&set->set, obj)) {
        return;
    }

    uint64_t obj_idx = ce_array_size(set->objs);
    ce_array_push(set->objs, obj, _G.allocator);
    ce_hash_add(&set->set, obj, obj_idx, _G.allocator);
}

void _remove_from_set(db_t *db,
                      uint32_t idx,
                      uint64_t obj) {
    set_t *set = _get_set(db, idx);

    uint64_t obj_idx = ce_hash_lookup(&set->set, obj, UINT64_MAX);

    if (obj_idx == UINT64_MAX) {
        return;
    }

    uint64_t obj_n = ce_array_size(set->objs);
    if (!obj_n) {
        return;
    }

    uint64_t last_idx = obj_n - 1;
    uint64_t last_obj = set->objs[last_idx];

    set->objs[obj_idx] = last_obj;

    ce_array_pop_back(set->objs);

    ce_hash_add(&set->set, last_obj, idx, _G.allocator);
    ce_hash_remove(&set->set, obj);
}


// type storage
#define MAX_TYPES 4096
#define MAX_TYPES_FREE_IDX MAX_OBJECTS


#define CE_ALIGN_PADDING(addr, align) (-((uint64_t)addr) & ((align) - 1))

size_t _type_obj_size(ce_cdb_type_def_t0 *defs) {
    size_t size = 0;
    for (int i = 0; i < defs->num; ++i) {
        type_info_t ti = _TYPE_INFO[defs->defs[i].type];

        uint32_t padding = CE_ALIGN_PADDING(size, ti.align);
        size += ti.size + padding;
    }
    return size;
}


type_storage_t *_get_or_create_storage(db_t *db,
                                       uint64_t type) {
#ifdef _FORCE_DYNAMINC_OBJECT
    return NULL;
#endif

    uint64_t idx = ce_hash_lookup(&db->type_map, type, UINT64_MAX);

    if (!type) {
        return NULL;
    }

    if (idx == UINT64_MAX) {
        uint32_t typedef_idx = ce_hash_lookup(&_G.type_defs.def_map, type, UINT32_MAX);

        if (typedef_idx == UINT32_MAX) {
            return NULL;
        }

        idx = atomic_fetch_add(&db->type_n, 1);
        ce_hash_add(&db->type_map, type, idx, _G.allocator);
        type_storage_t *storage = &db->type_storage[idx];

        ce_cdb_type_def_t0 *defs = &_G.type_defs.defs[typedef_idx];

        uint32_t type_size = _type_obj_size(defs);


        *storage = (type_storage_t) {
                .type =  type,
                .pool = virt_alloc(type_size * MAX_OBJECTS_PER_TYPE),
                .type_size = type_size,
                .pool_n = 1, // NULL element;
                .prop_def = defs,
        };

        ce_mpmc_init(&storage->free_idx, 4096, sizeof(uint64_t), _G.allocator);
        ce_mpmc_init(&storage->to_free_idx, 4096, sizeof(uint64_t), _G.allocator);

        uint32_t bytes = 0;
        for (int i = 0; i < defs->num; ++i) {
            ce_cdb_prop_def_t0 def = defs->defs[i];

            type_info_t ti = _TYPE_INFO[def.type];

            size_t padding = CE_ALIGN_PADDING(bytes, ti.align);
            size_t offset = bytes + padding;

            uint64_t k = ce_id_a0->id64(def.name);
            ce_array_push(storage->prop_type, def.type, _G.allocator);
            ce_array_push(storage->prop_name, k, _G.allocator);
            ce_array_push(storage->prop_offset, offset, _G.allocator);
            ce_hash_add(&storage->prop_idx, k, i, _G.allocator);

            bytes += (ti.size + padding);
        }
    }

    return &db->type_storage[idx];
}

type_storage_t *_get_storage(db_t *db,
                             uint64_t type) {
    uint64_t idx = ce_hash_lookup(&db->type_map, type, UINT64_MAX);

    if (idx == UINT64_MAX) {
        return NULL;
    }

    return &db->type_storage[idx];
}

uint64_t _new_typed_object(type_storage_t *storage,
                           uint64_t type) {
    if (!storage) {
        return 0;
    }

    uint64_t free_idx = 0;
    if (ce_mpmc_dequeue(&storage->free_idx, &free_idx)) {
        return free_idx;
    }

    uint64_t value_idx = atomic_fetch_add(&storage->pool_n, 1);
    return value_idx;
}

void _free_typed_object(db_t *db,
                        uint64_t type,
                        uint64_t idx) {
    type_storage_t *storage = _get_storage(db, type);

    if (!storage) {
        return;
    }

    ce_mpmc_enqueue(&storage->to_free_idx, &idx);
}

uint64_t _clone_typed_object(type_storage_t *storage,
                             uint64_t type,
                             uint64_t from_idx) {
    uint64_t clone_idx = _new_typed_object(storage, type);

    memcpy(&storage->pool[storage->type_size * clone_idx],
           &storage->pool[storage->type_size * from_idx],
           storage->type_size);

    return clone_idx;
}

ce_cdb_value_u0 *_get_prop_value_ptr(type_storage_t *storage,
                                     uint64_t obj_idx,
                                     uint64_t prop) {
    uint64_t prop_idx = ce_hash_lookup(&storage->prop_idx, prop, UINT64_MAX);

    if (prop_idx == UINT64_MAX) {
        return NULL;
    }

    uint64_t prop_offset = storage->prop_offset[prop_idx];

    uint64_t idx = obj_idx * storage->type_size;
    ce_cdb_value_u0 *val = (ce_cdb_value_u0 *) (&storage->pool[idx] + prop_offset);
    return val;
}

///
static struct object_t *_get_object_from_o(const ce_cdb_obj_o0 *obj_o) {
    return ((object_t *) obj_o);
}


void reg_obj_type(uint64_t type,
                  const ce_cdb_prop_def_t0 *prop_def,
                  uint32_t n) {
    for (int i = 0; i < n; ++i) {
        const ce_cdb_prop_def_t0 *def = &prop_def[i];

        if (def->type == CE_CDB_TYPE_SUBOBJECT) {
//            if (!def->obj_type) {
//                ce_log_a0->error(LOG_WHERE, "Property %s has not set obj_type", def->name);
//                return;
//            }
        }
    }

    uint32_t idx = ce_hash_lookup(&_G.type_defs.def_map, type, UINT32_MAX);

    if (idx == UINT32_MAX) {
        idx = ce_array_size(_G.type_defs.defs);
        ce_array_push(_G.type_defs.defs, (ce_cdb_type_def_t0) {}, _G.allocator);

        ce_hash_add(&_G.type_defs.def_map, type, idx, _G.allocator);
    }

    ce_cdb_type_def_t0 *def = &_G.type_defs.defs[idx];

    def->num = n;

    ce_array_clean(def->defs);
    ce_array_push_n(def->defs, prop_def, sizeof(ce_cdb_prop_def_t0) * n, _G.allocator);
}

const ce_cdb_type_def_t0 *_get_prop_def(uint64_t type) {
    uint32_t idx = ce_hash_lookup(&_G.type_defs.def_map, type, UINT32_MAX);

    if (idx == UINT32_MAX) {
        return NULL;
    }

    ce_cdb_type_def_t0 *def = &_G.type_defs.defs[idx];

    return def;
}

void set_loader(ct_cdb_obj_loader_t0 loader) {
    _G.loader = loader;
}


static struct db_t *_get_db(ce_cdb_t0 db) {
    return &_G.dbs[db.idx];
}

static void _add_instance(object_t *obj,
                          uint64_t instance) {
    ce_array_push(obj->instances, instance, _G.allocator);
}

static void _add_changed_obj(db_t *db_inst,
                             object_t *obj) {
    uint64_t k = ce_hash_murmur2_64(&obj->orig_obj, sizeof(uint64_t), 0);

    if (!ce_hash_contain(&db_inst->changed_obj_set, k)) {
        ce_os_thread_a0->spin_lock(&db_inst->change_lock);
        ce_array_push(db_inst->changed_obj, obj->orig_obj, _G.allocator);
        ce_hash_add(&db_inst->changed_obj_set, k, 0, _G.allocator);
        ce_os_thread_a0->spin_unlock(&db_inst->change_lock);

        ce_cdb_ev_t0 ev = {
                .ev_type = CE_CDB_OBJ_CHANGE_EVENT,
                .obj =  obj->orig_obj,
                .obj_type  = obj->type,
        };

        _push_event(&db_inst->chnaged_objs, &ev);
    };
}

static void _add_change(object_t *obj,
                        ce_cdb_prop_ev_t0 ev) {
    ce_array_push(obj->changed, ev, _G.allocator);
}

struct object_t *_new_object(db_t *db,
                             const struct ce_alloc_t0 *a) {

    object_t *obj;

    if (!ce_mpmc_dequeue(&db->free_objects, &obj)) {
        uint64_t idx = atomic_fetch_add(&db->object_pool_n, 1);

        obj = &db->object_pool[idx];
        *obj = (object_t) {};

        _init_listener_pack(&obj->obj_listeners);
    }

    obj->db.idx = db->idx;
    obj->key = 0;
    return obj;
}

static struct object_t *_object_clone(db_t *db,
                                      struct object_t *obj,
                                      const struct ce_alloc_t0 *alloc) {
    object_t *new_obj = _new_object(db, alloc);

    new_obj->instance_of = obj->instance_of;
    new_obj->parent = obj->parent;
    new_obj->key = obj->key;
    new_obj->id = obj->id;
    new_obj->flags = obj->flags;
    new_obj->type = obj->type;
    new_obj->obj_listeners = obj->obj_listeners;

    new_obj->storage = obj->storage;

    return new_obj;
}

static void _destroy_object(db_t *db_inst,
                            object_t *obj) {
    ce_mpmc_enqueue(&db_inst->to_free_objects, &obj);
}

static struct ce_cdb_t0 create_db(uint64_t max_objects) {
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
        ce_array_push(_G.dbs, (db_t) {}, _G.allocator);
    }

    _G.dbs[idx] = (db_t) {
            .used = true,
            .idx = idx,
            .max_objects = max_objects,

            .to_free_objects_uid = (uint64_t *) virt_alloc(max_objects * sizeof(uint64_t)),

            //
            .object_pool = (object_t *) virt_alloc(max_objects * sizeof(object_t)),

            //
            .object_id_pool = (object_t **) virt_alloc(max_objects * sizeof(object_t **)),
            .free_objects_id = (object_t ***) virt_alloc(max_objects * sizeof(object_t ***)),
            .type_storage = (type_storage_t *) virt_alloc(MAX_TYPES * sizeof(type_storage_t)),

#ifndef UID_HASHMAP
#endif
    };

#ifndef UID_HASHMAP
#endif

    struct db_t *db = &_G.dbs[idx];


    _init_listener_pack(&db->obj_listeners);
    _init_listener_pack(&db->chnaged_objs);

    ce_mpmc_init(&db->free_objects, 4096, sizeof(object_t *), _G.allocator);
    ce_mpmc_init(&db->to_free_objects, 4096, sizeof(object_t *), _G.allocator);

    // create set with idx == 0
    _new_set(&_G.dbs[idx]);

    // create blob with idx == 0
    _new_blob(&_G.dbs[idx]);

    return (ce_cdb_t0) {.idx = idx};
};


static void _init_from_defs(ce_cdb_t0 db,
                            object_t *obj,
                            const ce_cdb_type_def_t0 *def);

static uint64_t create_object_uid(ce_cdb_t0 db,
                                  uint64_t uid,
                                  uint64_t type,
                                  bool init) {
    if (!type) {
        ce_log_a0->error(LOG_WHERE, "Could not create object without type");
        return 0;
    }

    db_t *db_inst = &_G.dbs[db.idx];
    object_t *obj = _new_object(db_inst, _G.allocator);

    object_t **objid = _new_obj_id(db_inst);
    *objid = obj;
    _set_uid_objid(db_inst, uid, objid);

    obj->id = objid;
    obj->db = db;
    obj->type = type;
    obj->orig_obj = uid;

    type_storage_t *storage = _get_or_create_storage(db_inst, obj->type);

    if (!storage) {
        const char *type_str = ce_id_a0->str_from_id64(obj->type);
        if (type_str) {
            ce_log_a0->error(LOG_WHERE, "Type %s is not registered", type_str);
        } else {
            ce_log_a0->error(LOG_WHERE, "Type 0x%llx is not registered", obj->type);
        }
        return 0;
    }

    if (storage) {
        obj->storage = storage;
        obj->typed_obj_idx = _new_typed_object(storage, type);

        const ce_cdb_type_def_t0 *def = _get_prop_def(type);
        if (def && init) {
            _init_from_defs(db, obj, def);
        }
    }

    ce_array_clean(obj->changed);

    return uid;
}


static uint64_t create_object(ce_cdb_t0 db,
                              uint64_t type) {
    if (!type) {
        ce_log_a0->error(LOG_WHERE, "NO TYPE");
        return 0;
    }

    uint64_t uid = _gen_uid();
    create_object_uid(db, uid, type, true);
    return uid;
}

static ce_cdb_obj_o0 *write_begin(ce_cdb_t0 db,
                                  uint64_t _obj);

static void write_commit(ce_cdb_obj_o0 *_writer);

void set_subobject(ce_cdb_obj_o0 *_writer,
                   uint64_t property,
                   uint64_t subobject);

static ce_cdb_value_u0 *_get_value_ptr_generic(object_t *obj,
                                               uint64_t property) {

    uint64_t o = obj->typed_writer_idx ? obj->typed_writer_idx : obj->typed_obj_idx;
    ce_cdb_value_u0 *v = _get_prop_value_ptr(obj->storage, o, property);
    return v;

}

static bool prop_exist(const ce_cdb_obj_o0 *reader,
                       uint64_t key) {

    if (!reader) {
        return false;
    }

    object_t *obj = _get_object_from_o(reader);
    ce_cdb_value_u0 *v = _get_value_ptr_generic(obj, key);
    return v != NULL;
}

void prop_copy(const ce_cdb_obj_o0 *from,
               ce_cdb_obj_o0 *to,
               uint64_t prop);

static void set_instance_of(ce_cdb_t0 _db,
                            uint64_t from,
                            uint64_t to) {
    db_t *db = _get_db(_db);

    object_t *inst = _get_object_from_uid(db, to);
    inst->instance_of = from;

}

static uint64_t create_from(ce_cdb_t0 db,
                            uint64_t from);

uint64_t read_objset_count(const ce_cdb_obj_o0 *reader,
                           uint64_t property);

void add_obj(ce_cdb_obj_o0 *_writer,
             uint64_t property,
             uint64_t obj);

uint64_t _create_from_uid(ce_cdb_t0 db,
                          uint64_t from,
                          uint64_t uid,
                          bool load) {
    db_t *db_inst = &_G.dbs[db.idx];

    object_t *from_obj = _get_object_from_uid(db_inst, from);

    if (!from_obj) {
        return 0;
    }

    object_t *inst = _new_object(db_inst, _G.allocator);

    object_t **objid = _new_obj_id(db_inst);
    *objid = inst;
    _set_uid_objid(db_inst, uid, objid);

    inst->db = db;
    inst->id = objid;
    inst->instance_of = from;
    inst->type = from_obj->type;
    inst->orig_obj = uid;

    _add_instance(from_obj, uid);

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(db, from);

    type_storage_t *storage = _get_or_create_storage(db_inst, inst->type);
    if (storage) {
        inst->storage = storage;


//            inst->typed_obj_idx = _clone_typed_object(storage, inst->type, from_obj->typed_obj_idx);
        inst->typed_obj_idx = _new_typed_object(storage, inst->type);

        object_t *wr = inst;
        for (int i = 0; i < inst->storage->prop_def->num; ++i) {
            uint64_t key = inst->storage->prop_name[i];
            uint64_t type = inst->storage->prop_type[i];

            if (type == CE_CDB_TYPE_SUBOBJECT) {
                union ce_cdb_value_u0 *value_ptr = _get_prop_value_ptr(storage,
                                                                       from_obj->typed_obj_idx,
                                                                       key);

                uint64_t old_subobj = value_ptr->subobj;
                if (old_subobj) {
                    uint64_t new_subobj = create_from(db, old_subobj);
                    set_subobject((ce_cdb_obj_o0 *) wr, key, new_subobj);
                }

            } else if (load && type == CE_CDB_TYPE_SET_SUBOBJECT) {
                continue;
            } else {
                prop_copy(reader, (ce_cdb_obj_o0 *) inst, key);
            }
        }
    }


    ce_array_clean(inst->changed);

    return (uint64_t) uid;
}

uint64_t create_from_uid(ce_cdb_t0 db,
                         uint64_t from,
                         uint64_t uid) {
    return _create_from_uid(db, from, uid, false);
}

static uint64_t create_from(ce_cdb_t0 db,
                            uint64_t from) {
    uint64_t uid = _gen_uid();
    return create_from_uid(db, from, uid);
}

uint64_t read_objset_count(const ce_cdb_obj_o0 *reader,
                           uint64_t property) {
    object_t *obj = _get_object_from_o(reader);

    if (!obj) {
        return 0;
    }

    db_t *db_inst = _get_db(obj->db);


    uint32_t setidx = 0;


    uint64_t o = obj->typed_writer_idx ? obj->typed_writer_idx : obj->typed_obj_idx;
    ce_cdb_value_u0 *v = _get_prop_value_ptr(obj->storage, o, property);

    if (!v) {
        return 0;
    }

    setidx = v->set;

    set_t *set = _get_set(db_inst, setidx);

    if (!set) {
        return 0;
    }

    return ce_array_size(set->objs);
}

static void destroy_db(ce_cdb_t0 db) {
    ce_array_push(_G.to_free_db, db.idx, _G.allocator);
}

static uint64_t type(ce_cdb_t0 _db,
                     uint64_t _obj);

static void _add_obj_to_destroy_list(db_t *db_inst,
                                     uint64_t _obj) {
    const uint64_t n = db_inst->to_free_objects_uid_n;
    for (int i = 0; i < n; ++i) {
        if (db_inst->to_free_objects_uid[i] != _obj) {
            continue;
        }
//    CE_ASSERT(LOG_WHERE, !contain);
        return;
    }

    object_t *obj = _get_object_from_uid(db_inst, _obj);

    if (obj) {
        ce_cdb_ev_t0 ev = {
                .ev_type =  CE_CDB_OBJ_DESTROY_EVENT,
                .obj = _obj,
                .obj_type  = obj->type,
        };

        _push_event(&db_inst->chnaged_objs, &ev);

    }

    uint64_t idx = atomic_fetch_add(&db_inst->to_free_objects_uid_n, 1);
    db_inst->to_free_objects_uid[idx] = _obj;
}

static ce_cdb_type_e0 prop_type(const ce_cdb_obj_o0 *reader,
                                uint64_t key);

void _remove_obj(ce_cdb_obj_o0 *_writer,
                 uint64_t property,
                 uint64_t obj);

static void destroy_object(ce_cdb_t0 db,
                           uint64_t _obj) {
    db_t *db_inst = _get_db(db);

    ce_log_a0->debug(LOG_WHERE, "Destroy obj %llx", _obj);

    object_t *obj = _get_object_from_uid(db_inst, _obj);

    if (!obj) {
        ce_log_a0->warning(LOG_WHERE, "Obj %llx is not alive.", _obj);
        return;
    }


    _free_typed_object(db_inst, obj->type, obj->typed_obj_idx);

    if (obj->storage) {
        for (int i = 0; i < obj->storage->prop_def->num; ++i) {
            uint64_t key = obj->storage->prop_name[i];
            ce_cdb_type_e0 type = obj->storage->prop_type[i];
            switch (type) {
                case CE_CDB_TYPE_SUBOBJECT: {
                    union ce_cdb_value_u0 *value_ptr = _get_value_ptr_generic(obj, key);

                    uint64_t old_subobj = value_ptr->subobj;
                    if (old_subobj) {
                        destroy_object(db, old_subobj);
                    }
                }

                    break;

                case CE_CDB_TYPE_SET_SUBOBJECT: {
                    uint64_t n = read_objset_count((ce_cdb_obj_o0 *) obj, key);
                    uint64_t k[n];
                    ce_cdb_a0->read_objset((ce_cdb_obj_o0 *) obj, key, k);
                    for (int j = 0; j < n; ++j) {
                        uint64_t subobj = k[j];
                        destroy_object(db, subobj);
                    }
                }
                    break;
                default:
                    break;
            }
        }
    }

    // Remove from parent
    if (obj->parent) {
        object_t *parent_obj = _get_object_from_uid(db_inst, obj->parent);
        if (prop_type((ce_cdb_obj_o0 *) parent_obj, obj->key) == CE_CDB_TYPE_SET_SUBOBJECT) {
            _remove_obj((ce_cdb_obj_o0 *) parent_obj, obj->key, _obj);
        }
    }

    // destroy instances
    uint64_t inst_n = ce_array_size(obj->instances);
    for (int l = 0; l < inst_n; ++l) {
        uint64_t inst = obj->instances[l];
        destroy_object(db, inst);
    }

    _add_obj_to_destroy_list(db_inst, _obj);
}

static void _gc_db() {
    const uint32_t fdb_n = ce_array_size(_G.to_free_db);
    for (int i = 0; i < fdb_n; ++i) {
        uint32_t idx = _G.to_free_db[i];
        struct db_t *db_inst = &_G.dbs[idx];

        virt_free(db_inst->to_free_objects_uid, db_inst->max_objects * sizeof(object_t **));
        virt_free(db_inst->object_pool, db_inst->max_objects * sizeof(object_t));

        ce_mpmc_free(&db_inst->free_objects);
        ce_mpmc_free(&db_inst->to_free_objects);

        db_inst->used = false;
    }
    ce_array_clean(_G.to_free_db);
}

static void _typed_gc() {
    const uint32_t db_n = ce_array_size(_G.dbs);
    for (int i = 0; i < db_n; ++i) {
        struct db_t *db_inst = &_G.dbs[i];

        if (!db_inst->used) {
            continue;
        }

        uint32_t n = db_inst->type_n;

        for (int j = 0; j < n; ++j) {
            type_storage_t *storage = &db_inst->type_storage[j];

            uint64_t to_free_idx = 0;
            while (ce_mpmc_dequeue(&storage->to_free_idx, &to_free_idx)) {
                memset(&storage->pool[to_free_idx * storage->type_size], 0, storage->type_size);
                ce_mpmc_enqueue(&storage->free_idx, &to_free_idx);
            }
        }
    }
}

static void gc() {
    _gc_db();

    const uint32_t db_n = ce_array_size(_G.dbs);
    for (int i = 0; i < db_n; ++i) {
        struct db_t *db_inst = &_G.dbs[i];

        if (!db_inst->used) {
            continue;
        }

        ce_array_clean(db_inst->changed_obj);
        ce_hash_clean(&db_inst->changed_obj_set);

        const uint32_t to_free_objects_uid_n = db_inst->to_free_objects_uid_n;

        for (int j = 0; j < to_free_objects_uid_n; ++j) {
            uint64_t objid = db_inst->to_free_objects_uid[j];

            struct object_t *obj = _get_object_from_uid(db_inst, objid);

            // remove from instance from parent instance
            if (obj->instance_of) {
                struct object_t *prefab_obj = _get_object_from_uid(db_inst, obj->instance_of);

                const uint32_t instances_n = ce_array_size(prefab_obj->instances);

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

        for (int j = 0; j < to_free_objects_uid_n; ++j) {
            uint64_t uid = db_inst->to_free_objects_uid[j];

            object_t **objid = _get_objectid_from_uid(db_inst, uid);
            if (objid) {
                _free_obj_id(db_inst, objid);
            }

            _remove_uid_obj(db_inst, uid);
        }

        db_inst->to_free_objects_uid_n = 0;


        struct object_t *to_free_obj = 0;
        while (ce_mpmc_dequeue(&db_inst->to_free_objects, &to_free_obj)) {
            ce_array_clean(to_free_obj->instances);

            ce_array_clean(to_free_obj->changed);

            *to_free_obj = (object_t) {
                    .instances = to_free_obj->instances,
                    .changed = to_free_obj->changed,
                    .obj_listeners = to_free_obj->obj_listeners,
            };

            ce_mpmc_enqueue(&db_inst->free_objects, &to_free_obj);
        }

    }

    _typed_gc();
}


static bool _val_eq(ce_cdb_value_u0 v1,
                    ce_cdb_value_u0 v2,
                    ce_cdb_type_e0 t) {
    if (t == CE_CDB_TYPE_FLOAT) {
        return v1.f == v2.f;
    }

    return v1.uint64 == v2.uint64;
}

void _dump(ce_cdb_t0 db,
           uint64_t _obj,
           uint64_t key,
           char **str_buffer,
           char **blob_buffer,
           cnode_t **nodes,
           struct ce_alloc_t0 *allocator) {
    db_t *dbi = _get_db(db);

    object_t *obj = _get_object_from_uid(dbi, _obj);

    ce_array_push(*nodes, ((cnode_t) {
            .type = CNODE_OBJ_BEGIN,
            .key = key,
            .obj.type = obj->type,
            .obj.uid = _obj,
            .obj.instance_of = obj->instance_of,
    }), allocator);

    ce_cdb_type_def_t0 *defs = obj->storage->prop_def;

    for (int i = 0; i < defs->num; ++i) {
        ce_cdb_prop_def_t0 *def = &defs->defs[i];
        uint64_t k = ce_id_a0->id64(def->name);
        ce_cdb_value_u0 *v = _get_prop_value_ptr(obj->storage, obj->typed_obj_idx, k);

        // skip default
        if (_val_eq(def->value, *v, def->type)) {
            continue;
        }

        switch (def->type) {
            case CE_CDB_TYPE_UINT64: {
                ce_array_push(*nodes, ((cnode_t) {
                        .key = k,
                        .type = CNODE_UINT,
                        .value = *v,
                }), allocator);
            }
                break;

            case CE_CDB_TYPE_REF: {
                ce_array_push(*nodes, ((cnode_t) {
                        .key = k,
                        .type = CNODE_REF,
                        .value = *v,
                }), allocator);
            }
                break;

            case CE_CDB_TYPE_FLOAT: {
                ce_array_push(*nodes, ((cnode_t) {
                        .key = k,
                        .type = CNODE_FLOAT,
                        .value = *v,
                }), allocator);
            }
                break;
            case CE_CDB_TYPE_BOOL: {
                ce_array_push(*nodes, ((cnode_t) {
                        .key = k,
                        .type = CNODE_BOOL,
                        .value = *v,
                }), allocator);
            }
                break;

            case CE_CDB_TYPE_SUBOBJECT: {
                const ce_cdb_obj_o0 *reader = (const ce_cdb_obj_o0 *) obj;

                uint64_t subobj = ce_cdb_a0->read_subobject(reader, k, 0);

                _dump(db, subobj, k, str_buffer, blob_buffer, nodes, allocator);
            }
                break;

            case CE_CDB_TYPE_SET_SUBOBJECT: {
                const ce_cdb_obj_o0 *reader = (const ce_cdb_obj_o0 *) obj;

                uint64_t n = ce_cdb_a0->read_objset_num(reader, k);
                uint64_t objs[n];
                ce_cdb_a0->read_objset(reader, k, objs);

                ce_array_push(*nodes, ((cnode_t) {
                        .type = CNODE_OBJSET,
                        .key = k,
                }), allocator);

                for (int j = 0; j < n; ++j) {
                    _dump(db, objs[j], 0, str_buffer, blob_buffer, nodes, allocator);
                }

                ce_array_push(*nodes, ((cnode_t) {
                        .type = CNODE_OBJSET_END,
                }), allocator);

            }
                break;

            case CE_CDB_TYPE_BLOB: {
                uint64_t bloboffset = ce_array_size(*blob_buffer);

                struct ce_cdb_blob_t0 *blob = _get_blob(dbi, v->blob);

                if (blob->data) {
                    ce_array_push_n(*blob_buffer, blob->data, blob->size, _G.allocator);
                }

                ce_array_push(*nodes, ((cnode_t) {
                        .type = CNODE_BLOB,
                        .key = k,
                        .blob.size = blob->size,
                        .blob.data = (void *) bloboffset,
                }), allocator);

            }
                break;

            case CE_CDB_TYPE_STR: {
                uint64_t stroffset = ce_array_size(*str_buffer);

                if (v->str) {
                    ce_array_push_n(*str_buffer, v->str, strlen(v->str) + 1, allocator);
                }

                ce_array_push(*nodes, ((cnode_t) {
                        .type = CNODE_STRING,
                        .key = k,
                        .value.uint64 = stroffset,
                }), allocator);

            }
                break;

        }
    }

    ce_array_push(*nodes, ((cnode_t) {
            .type = CNODE_OBJ_END,
    }), allocator);
}


static void dump(ce_cdb_t0 db,
                 uint64_t _obj,
                 char **output,
                 struct ce_alloc_t0 *allocator) {
    char *str_buffer = NULL;
    char *blob_buffer = NULL;
    cnode_t *nodes = NULL;

    _dump(db, _obj, 0, &str_buffer, &blob_buffer, &nodes, allocator);

    cdb_binobj_header header = {
            .version = 0,
            .blob_buffer_size = ce_array_size(blob_buffer),
            .string_buffer_size = ce_array_size(str_buffer),
            .node_count = ce_array_size(nodes),
    };

    ce_array_push_n(*output, (char *) &header, sizeof(cdb_binobj_header), _G.allocator);
    ce_array_push_n(*output, (char *) nodes, sizeof(cnode_t) * header.node_count, _G.allocator);
    ce_array_push_n(*output, (char *) str_buffer, header.string_buffer_size, _G.allocator);
    ce_array_push_n(*output, (char *) blob_buffer, header.blob_buffer_size, _G.allocator);
}


static ce_cdb_obj_o0 *write_begin(ce_cdb_t0 db,
                                  uint64_t _obj) {
    db_t *db_inst = _get_db(db);
    object_t *obj = _get_object_from_uid(db_inst, _obj);

    if (!obj) {
        return NULL;
    }

    object_t *new_obj = _object_clone(db_inst, obj, _G.allocator);


    new_obj->typed_obj_idx = obj->typed_obj_idx;
    uint64_t clone_objidx = _clone_typed_object(new_obj->storage,
                                                obj->type,
                                                obj->typed_obj_idx);
    new_obj->typed_writer_idx = clone_objidx;


    uint32_t n = ce_array_size(obj->instances);
    if (n) {
        ce_array_push_n(new_obj->instances, obj->instances, n, _G.allocator);
    }

    new_obj->orig_obj = _obj;

    return (ce_cdb_obj_o0 *) new_obj;
}

static ce_cdb_type_e0 prop_type(const ce_cdb_obj_o0 *reader,
                                uint64_t key) {
    object_t *obj = _get_object_from_o(reader);


    for (int i = 0; i < obj->storage->prop_def->num; ++i) {
        uint64_t name = obj->storage->prop_name[i];
        if (name != key) {
            continue;
        }

        return obj->storage->prop_type[i];
    }

    return CE_CDB_TYPE_NONE;
}

static void _dispatch_instances(ce_cdb_t0 db,
                                struct object_t *orig_obj);

static void write_commit(ce_cdb_obj_o0 *_writer) {
    object_t *writer = _get_object_from_o(_writer);

    if (!writer) {
        return;
    }

    db_t *db = _get_db(writer->db);

    object_t *orig_obj = _get_object_from_uid(db, writer->orig_obj);

    _dispatch_instances(writer->db, writer);

    if (writer->typed_obj_idx) {
        _free_typed_object(db, writer->type, writer->typed_obj_idx);
        writer->typed_obj_idx = writer->typed_writer_idx;
    }

    *writer->id = writer;

    _add_changed_obj(db, writer);

    uint32_t ch_n = ce_array_size(writer->changed);
    for (int i = 0; i < ch_n; ++i) {
        _push_event(&db->obj_listeners, &writer->changed[i]);
        _push_event(&writer->obj_listeners, &writer->changed[i]);
    }

    ce_array_clean(writer->changed);
    _destroy_object(db, orig_obj);
}

static bool write_try_commit(ce_cdb_obj_o0 *_writer) {
    object_t *writer = _get_object_from_o(_writer);
    db_t *db = _get_db(writer->db);

    object_t *orig_obj = _get_object_from_uid(db, writer->orig_obj);

    object_t **obj_addr = orig_obj->id;

    if (writer->typed_obj_idx) {
        _free_typed_object(db, writer->type, writer->typed_obj_idx);
        writer->typed_obj_idx = writer->typed_writer_idx;
    }

    bool ok = atomic_compare_exchange_weak((atomic_ullong *) obj_addr,
                                           ((uint64_t *) &orig_obj),
                                           ((uint64_t) writer));

    if (ok) {
        _add_changed_obj(db, writer);

        uint32_t ch_n = ce_array_size(writer->changed);

        for (int i = 0; i < ch_n; ++i) {
            _push_event(&db->obj_listeners, &writer->changed[i]);
        }
    }

    _destroy_object(db, orig_obj);
    return ok;
}

static ce_cdb_value_u0 *_get_value_ptr(object_t *obj,
                                       uint64_t property,
                                       ce_cdb_type_e0 prop_type) {

    uint64_t o = obj->typed_writer_idx ? obj->typed_writer_idx : obj->typed_obj_idx;
    ce_cdb_value_u0 *v = _get_prop_value_ptr(obj->storage, o, property);
    return v;
}

static void _set_float(ce_cdb_obj_o0 *_writer,
                       uint64_t property,
                       float value,
                       bool log_event) {
    object_t *writer = _get_object_from_o(_writer);

    if (!writer) {
        return;
    }

    ce_cdb_value_u0 *value_ptr = _get_value_ptr(writer, property, CE_CDB_TYPE_FLOAT);

    if (!value_ptr) {
        return;
    }

    if (log_event) {
        _add_change(writer, (ce_cdb_prop_ev_t0) {
                .obj = writer->orig_obj,
                .prop = property,
                .ev_type = CE_CDB_PROP_CHANGE_EVENT,
                .prop_type = CE_CDB_TYPE_FLOAT,
                .old_value = *value_ptr,
                .new_value.f = value,
        });
    }

    value_ptr->f = value;

}

static void _set_bool(ce_cdb_obj_o0 *_writer,
                      uint64_t property,
                      bool value,
                      bool log_event) {
    object_t *writer = _get_object_from_o(_writer);

    if (!writer) {
        return;
    }

    ce_cdb_value_u0 *value_ptr = _get_value_ptr(writer, property,
                                                CE_CDB_TYPE_BOOL);

    if (!value_ptr) {
        return;
    }

    if (log_event) {
        _add_change(writer, (ce_cdb_prop_ev_t0) {
                .obj = writer->orig_obj,
                .prop=property,
                .ev_type = CE_CDB_PROP_CHANGE_EVENT,
                .prop_type = CE_CDB_TYPE_BOOL,
                .old_value = *value_ptr,
                .new_value.b = value,
        });
    }

    value_ptr->b = value;
}

static void _set_string(ce_cdb_obj_o0 *_writer,
                        uint64_t property,
                        const char *value,
                        bool log_event) {
    ce_alloc_t0 *a = _G.allocator;

    object_t *writer = _get_object_from_o(_writer);

    ce_cdb_value_u0 *value_ptr = _get_value_ptr(writer, property,
                                                CE_CDB_TYPE_STR);

    if (!value_ptr) {
        return;
    }

    char *value_clone = NULL;
    if (value) {
        value_clone = ce_memory_a0->str_dup(value, a);
    }

    if (log_event) {
        _add_change(writer, (ce_cdb_prop_ev_t0) {
                .obj = writer->orig_obj,
                .prop=property,
                .ev_type =CE_CDB_PROP_CHANGE_EVENT,
                .prop_type = CE_CDB_TYPE_STR,
                .old_value = *value_ptr,
                .new_value.str = value_clone,
        });
    }

    value_ptr->str = value_clone;
}


static void _set_uint64(ce_cdb_obj_o0 *_writer,
                        uint64_t property,
                        uint64_t value,
                        bool log_event) {
    object_t *writer = _get_object_from_o(_writer);

    ce_cdb_value_u0 *value_ptr = _get_value_ptr(writer, property,
                                                CE_CDB_TYPE_UINT64);

    if (!value_ptr) {
        return;
    }

    if (log_event) {
        _add_change(writer, (ce_cdb_prop_ev_t0) {
                .obj = writer->orig_obj,
                .prop=property,
                .ev_type = CE_CDB_PROP_CHANGE_EVENT,
                .prop_type = CE_CDB_TYPE_UINT64,
                .old_value = *value_ptr,
                .new_value.uint64 = value,
        });
    }

    value_ptr->uint64 = value;
}

static void _set_ptr(ce_cdb_obj_o0 *_writer,
                     uint64_t property,
                     const void *value,
                     bool log_event) {
    object_t *writer = _get_object_from_o(_writer);

    if (!writer) {
        return;
    }

    ce_cdb_value_u0 *value_ptr = _get_value_ptr(writer, property,
                                                CE_CDB_TYPE_PTR);

    if (!value_ptr) {
        return;
    }

    if (log_event) {
        _add_change(writer, (ce_cdb_prop_ev_t0) {
                .obj = writer->orig_obj,
                .prop=property,
                .ev_type =CE_CDB_PROP_CHANGE_EVENT,
                .prop_type = CE_CDB_TYPE_PTR,
                .old_value = *value_ptr,
                .new_value.ptr = (void *) value,
        });
    }

    value_ptr->ptr = (void *) value;
}

static void _set_ref(ce_cdb_obj_o0 *_writer,
                     uint64_t property,
                     uint64_t ref,
                     bool log_event) {


    object_t *writer = _get_object_from_o(_writer);

    if (!writer) {
        return;
    }

    ce_cdb_value_u0 *value_ptr = _get_value_ptr(writer, property,
                                                CE_CDB_TYPE_REF);

    if (!value_ptr) {
        return;
    }

    if (log_event) {
        _add_change(writer, (ce_cdb_prop_ev_t0) {
                .obj = writer->orig_obj,
                .prop=property,
                .ev_type =CE_CDB_PROP_CHANGE_EVENT,
                .prop_type = CE_CDB_TYPE_REF,
                .old_value = *value_ptr,
                .new_value.ref = ref,
        });
    }

    value_ptr->ref = ref;
}

void _set_subobject(ce_cdb_obj_o0 *_writer,
                    uint64_t property,
                    uint64_t subobject,
                    bool log_event) {
    object_t *writer = _get_object_from_o(_writer);

    if (!writer) {
        return;
    }

    ce_cdb_value_u0 *value_ptr = _get_value_ptr(writer, property,
                                                CE_CDB_TYPE_SUBOBJECT);


    if (!value_ptr) {
        return;
    }

    if (value_ptr->subobj) {
        destroy_object(writer->db, value_ptr->subobj);
    }

    value_ptr->subobj = subobject;

    if (log_event) {
        _add_change(writer, (ce_cdb_prop_ev_t0) {
                .obj = writer->orig_obj,
                .prop = property,
                .ev_type = CE_CDB_PROP_CHANGE_EVENT,
                .prop_type = CE_CDB_TYPE_SUBOBJECT,
                .old_value = *value_ptr,
                .new_value.subobj = subobject,
        });
    }

    if (subobject) {
        struct db_t *db = _get_db(writer->db);

        struct object_t *subobj = _get_object_from_uid(db, subobject);

        if (subobj) {
            subobj->parent = writer->orig_obj;
            subobj->key = property;
        }
    }
}


static void set_float(ce_cdb_obj_o0 *_writer,
                      uint64_t property,
                      float value) {
    _set_float(_writer, property, value, true);
}

static void set_bool(ce_cdb_obj_o0 *_writer,
                     uint64_t property,
                     bool value) {
    _set_bool(_writer, property, value, true);
}

static void set_string(ce_cdb_obj_o0 *_writer,
                       uint64_t property,
                       const char *value) {
    _set_string(_writer, property, value, true);
}

static void set_uint64(ce_cdb_obj_o0 *_writer,
                       uint64_t property,
                       uint64_t value) {
    _set_uint64(_writer, property, value, true);
}


static void set_ptr(ce_cdb_obj_o0 *_writer,
                    uint64_t property,
                    const void *value) {
    _set_ptr(_writer, property, value, true);
}

static void set_ref(ce_cdb_obj_o0 *_writer,
                    uint64_t property,
                    uint64_t ref) {
    _set_ref(_writer, property, ref, true);
}

uint64_t parent(ce_cdb_t0 _db,
                uint64_t object) {
    db_t *db = _get_db(_db);

    object_t *obj = _get_object_from_uid(db, object);
    if (!obj) {
        return 0;
    }
    return obj->parent;
}

void set_subobject(ce_cdb_obj_o0 *_writer,
                   uint64_t property,
                   uint64_t subobject) {
    _set_subobject(_writer, property, subobject, true);
}

void _set_blob(ce_cdb_obj_o0 *_writer,
               uint64_t property,
               const void *blob_data,
               uint64_t blob_size,
               bool log_event) {
    ce_alloc_t0 *a = _G.allocator;

    object_t *writer = _get_object_from_o(_writer);

    if (!writer) {
        return;
    }

    db_t *db = _get_db(writer->db);

    ce_cdb_value_u0 *value_ptr = _get_value_ptr(writer, property,
                                                CE_CDB_TYPE_BLOB);

    if (!value_ptr) {
        return;
    }

    if (value_ptr->blob) {
        struct ce_cdb_blob_t0 *blob = _get_blob(db, value_ptr->blob);
        CE_FREE(a, blob->data);
    } else {
        uint32_t new_blob_idx = _new_blob(db);
        value_ptr->blob = new_blob_idx;
    }

    void *new_blob = CE_ALLOC(a, uint8_t, blob_size);
    memcpy(new_blob, blob_data, blob_size);

    uint32_t blob_idx = value_ptr->blob;
    ce_cdb_blob_t0 *blob = _get_blob(db, blob_idx);

    *blob = (ce_cdb_blob_t0) {
            .size = blob_size,
            .data = new_blob,
    };

    if (log_event) {
        _add_change(writer, (ce_cdb_prop_ev_t0) {
                .obj = writer->orig_obj,
                .prop=property,
                .ev_type =CE_CDB_PROP_CHANGE_EVENT,
                .prop_type = CE_CDB_TYPE_BLOB,
                .old_value = *value_ptr,
                .new_value.blob = blob_idx,
        });
    }
}

void set_blob(ce_cdb_obj_o0 *_writer,
              uint64_t property,
              void *blob_data,
              uint64_t blob_size) {
    _set_blob(_writer, property, blob_data, blob_size, true);
}

void _add_obj(ce_cdb_obj_o0 *_writer,
              uint64_t property,
              uint64_t obj,
              bool log_event) {
    object_t *writer = _get_object_from_o(_writer);

    if (!writer) {
        return;
    }

    db_t *db = _get_db(writer->db);

    ce_cdb_value_u0 *value_ptr = _get_value_ptr(writer, property,
                                                CE_CDB_TYPE_SET_SUBOBJECT);

    if (!value_ptr) {
        return;
    }

    if (!value_ptr->set) {
        value_ptr->set = _new_set(db);
    }


    if (obj) {
        struct object_t *subobj = _get_object_from_uid(db, obj);

        if (subobj) {
            subobj->parent = writer->orig_obj;
            subobj->key = property;

            _add_to_set(db, value_ptr->set, obj);

            if (log_event) {
                _add_change(writer, (ce_cdb_prop_ev_t0) {
                        .obj = writer->orig_obj,
                        .prop=property,
                        .ev_type =CE_CDB_OBJSET_ADD_EVENT,
                        .prop_type = CE_CDB_TYPE_SET_SUBOBJECT,
                        .new_value.subobj = obj,
                });
            }
        }
    }
}


void add_obj(ce_cdb_obj_o0 *_writer,
             uint64_t property,
             uint64_t obj) {
    _add_obj(_writer, property, obj, true);
}

void _remove_obj(ce_cdb_obj_o0 *_writer,
                 uint64_t property,
                 uint64_t obj) {
    object_t *writer = _get_object_from_o(_writer);

    if (!writer) {
        return;
    }

    db_t *db = _get_db(writer->db);

    ce_cdb_value_u0 *value_ptr = _get_value_ptr_generic(writer, property);

    if (!value_ptr) {
        return;
    }

    uint32_t set_idx = value_ptr->set;

    _remove_from_set(db, set_idx, obj);

    _add_change(writer, (ce_cdb_prop_ev_t0) {
            .obj = writer->orig_obj,
            .prop=property,
            .ev_type =CE_CDB_OBJSET_REMOVE_EVENT,
            .prop_type = CE_CDB_TYPE_SET_SUBOBJECT,
            .old_value.subobj= obj,
    });
}

void remove_obj(ce_cdb_obj_o0 *_writer,
                uint64_t property,
                uint64_t obj) {
    _remove_obj(_writer, property, obj);

    object_t *writer = _get_object_from_o(_writer);

    if (!writer) {
        return;
    }

    destroy_object(writer->db, obj);
}

const ce_cdb_obj_o0 *read(ce_cdb_t0 _db,
                          uint64_t object) {
    if (!object) {
        return NULL;
    }

    db_t *db = _get_db(_db);
    object_t *obj = _get_object_from_uid(db, object);
    return (ce_cdb_obj_o0 *) obj;
}

static uint64_t type(ce_cdb_t0 _db,
                     uint64_t _obj) {
    if (!_obj) {
        return 0;
    }

    db_t *db = _get_db(_db);
    object_t *obj = _get_object_from_uid(db, _obj);

    if (!obj) {
        return 0;
    }

    return obj->type;
}


uint64_t read_to(ce_cdb_t0 db,
                 uint64_t object,
                 void *to,
                 size_t max_size);

uint64_t _read_to(ce_cdb_t0 db,
                  void *to,
                  uint64_t type,
                  ce_cdb_value_u0 *v,
                  ce_cdb_prop_def_t0 *def,
                  uint64_t cur_byte,
                  size_t max_size) {
    db_t *db_inst = _get_db(db);

    type_info_t ti = _TYPE_INFO[def->type];

    size_t padding = 0;//CE_ALIGN_PADDING(to + cur_byte, ti.align);

    if (def->flags & CE_CDB_PROP_FLAG_UNPACK) {
        return read_to(db, v->subobj, to + cur_byte, max_size);
    } else if (type == CE_CDB_TYPE_BLOB) {
        ce_cdb_blob_t0 *blob = _get_blob(db_inst, v->blob);
        memcpy(to + (cur_byte + padding), &blob->data, sizeof(void *));
        return ti.size + padding;
    } else {
        memcpy(to + (cur_byte + padding), v, ti.size);
        return ti.size + padding;
    }
}

uint64_t read_to(ce_cdb_t0 db,
                 uint64_t object,
                 void *to,
                 size_t max_size) {
    db_t *db_inst = _get_db(db);
    object_t *obj = _get_object_from_uid(db_inst, object);

    if (!obj) {
        return 0;
    }

    uint64_t cur_byte = 0;


    uint32_t n = obj->storage->prop_def->num;
    for (int i = 0; i < n; ++i) {
        ce_cdb_prop_def_t0 *def = &obj->storage->prop_def->defs[i];
        uint64_t name = obj->storage->prop_name[i];
        ce_cdb_type_e0 type = obj->storage->prop_type[i];

        ce_cdb_value_u0 *v = _get_prop_value_ptr(obj->storage, obj->typed_obj_idx, name);

        cur_byte += _read_to(db, to, type, v, def, cur_byte, max_size);
    }

    CE_ASSERT(LOG_WHERE, cur_byte <= max_size);
    return cur_byte;
}

static float read_float(const ce_cdb_obj_o0 *reader,
                        uint64_t property,
                        float defaultt) {
    if (!reader) {
        return defaultt;
    }

    object_t *obj = _get_object_from_o(reader);

    ce_cdb_value_u0 *v = _get_value_ptr_generic(obj, property);
    if (!v) {
        return defaultt;
    }

    return v->f;
}

static bool read_bool(const ce_cdb_obj_o0 *reader,
                      uint64_t property,
                      bool defaultt) {
    if (!reader) {
        return defaultt;
    }

    object_t *obj = _get_object_from_o(reader);
    ce_cdb_value_u0 *v = _get_value_ptr_generic(obj, property);
    if (!v) {
        return defaultt;
    }

    return v->b;
}

static const char *read_string(const ce_cdb_obj_o0 *reader,
                               uint64_t property,
                               const char *defaultt) {
    if (!reader) {
        return defaultt;
    }

    object_t *obj = _get_object_from_o(reader);
    ce_cdb_value_u0 *v = _get_value_ptr_generic(obj, property);
    if (!v) {
        return defaultt;
    }

    return v->str;
}


static uint64_t read_uint64(const ce_cdb_obj_o0 *reader,
                            uint64_t property,
                            uint64_t defaultt) {
    if (!reader) {
        return defaultt;
    }

    object_t *obj = _get_object_from_o(reader);

    ce_cdb_value_u0 *v = _get_value_ptr_generic(obj, property);
    if (!v) {
        return defaultt;
    }

    return v->uint64;
}

static void *read_ptr(const ce_cdb_obj_o0 *reader,
                      uint64_t property,
                      void *defaultt) {
    if (!reader) {
        return defaultt;
    }

    object_t *obj = _get_object_from_o(reader);

    ce_cdb_value_u0 *v = _get_value_ptr_generic(obj, property);
    if (!v) {
        return defaultt;
    }

    return v->ptr;
}

static uint64_t read_ref(const ce_cdb_obj_o0 *reader,
                         uint64_t property,
                         uint64_t defaultt) {
    if (!reader) {
        return defaultt;
    }

    object_t *obj = _get_object_from_o(reader);
    ce_cdb_value_u0 *v = _get_value_ptr_generic(obj, property);
    if (!v) {
        return defaultt;
    }

    return v->ref;
}

static uint64_t read_subobject(const ce_cdb_obj_o0 *reader,
                               uint64_t property,
                               uint64_t defaultt) {
    if (!reader) {
        return defaultt;
    }

    object_t *obj = _get_object_from_o(reader);
    ce_cdb_value_u0 *v = _get_value_ptr_generic(obj, property);
    if (!v) {
        return defaultt;
    }

    return v->subobj;
}

void read_objset(const ce_cdb_obj_o0 *reader,
                 uint64_t property,
                 uint64_t *objs) {
    object_t *obj = _get_object_from_o(reader);

    if (!obj) {
        return;
    }

    db_t *db_inst = _get_db(obj->db);


    ce_cdb_value_u0 *v = _get_value_ptr_generic(obj, property);
    if (!v) {
        return;
    }

    uint32_t setidx = v->set;
    set_t *set = _get_set(db_inst, setidx);

    if (!set) {
        return;
    }

    uint32_t size = ce_array_size(set->objs);

    memcpy(objs, set->objs, size * sizeof(uint64_t));
}

void *read_blob(const ce_cdb_obj_o0 *reader,
                uint64_t property,
                uint64_t *size,
                void *defaultt) {
    if (!reader) {
        return defaultt;
    }

    object_t *obj = _get_object_from_o(reader);

    ce_cdb_value_u0 *v = _get_value_ptr_generic(obj, property);
    if (!v) {
        return defaultt;
    }

    db_t *db = _get_db(obj->db);
    ce_cdb_blob_t0 *blob = _get_blob(db, v->blob);

    if (size) {
        *size = blob->size;
    }

    return blob->data;
}

static const uint64_t *prop_keys(const ce_cdb_obj_o0 *reader) {
    object_t *obj = _get_object_from_o(reader);

    if (!obj) {
        return 0;
    }


    return obj->storage->prop_name;

}

static uint64_t prop_count(const ce_cdb_obj_o0 *reader) {
    object_t *obj = _get_object_from_o(reader);

    if (!obj) {
        return 0;
    }


    return ce_array_size(obj->storage->prop_name);

}


bool prop_equal(const ce_cdb_obj_o0 *r1,
                const ce_cdb_obj_o0 *r2,
                uint64_t prorp) {
    ce_cdb_type_e0 t = prop_type(r1, prorp);

    if (!prop_exist(r1, prorp) || !prop_exist(r2, prorp)) {
        return false;
    }

    switch (t) {
        case CE_CDB_TYPE_NONE:
            break;
        case CE_CDB_TYPE_UINT64:
            return read_uint64(r1, prorp, 0) == read_uint64(r2, prorp, 0);
            break;
        case CE_CDB_TYPE_PTR:
            return read_ptr(r1, prorp, 0) == read_ptr(r2, prorp, 0);
            break;
        case CE_CDB_TYPE_REF:
            return read_ref(r1, prorp, 0) == read_ref(r2, prorp, 0);
            break;
        case CE_CDB_TYPE_FLOAT:
            return read_float(r1, prorp, 0) == read_float(r2, prorp, 0);
            break;
        case CE_CDB_TYPE_BOOL:
            return read_bool(r1, prorp, 0) == read_bool(r2, prorp, 0);
            break;

        case CE_CDB_TYPE_STR: {
            const char *s1 = read_string(r1, prorp, 0);
            const char *s2 = read_string(r2, prorp, 0);

            if (s1 == s2) {
                return true;
            }

            if (s1 == NULL || s2 == NULL) {
                return false;
            }

            return !strcmp(s1, s2);
        }
            break;
        case CE_CDB_TYPE_SUBOBJECT:
            return read_subobject(r1, prorp, 0) == read_subobject(r2, prorp, 0);
            break;
        case CE_CDB_TYPE_BLOB:
            break;

        default:
            break;
    }

    return false;
}

void prop_copy(const ce_cdb_obj_o0 *from,
               ce_cdb_obj_o0 *to,
               uint64_t prop) {
    ce_cdb_type_e0 t = prop_type(from, prop);
    union ce_cdb_value_u0 v;
    switch (t) {
        case CE_CDB_TYPE_NONE:
            break;

        case CE_CDB_TYPE_UINT64:
            v.uint64 = read_uint64(from, prop, 0);
            set_uint64(to, prop, v.uint64);
            break;
        case CE_CDB_TYPE_PTR:
            v.ptr = read_ptr(from, prop, 0);
            set_ptr(to, prop, v.ptr);
            break;

        case CE_CDB_TYPE_REF:
            v.ref = read_ref(from, prop, 0);
            set_ref(to, prop, v.ref);
            break;

        case CE_CDB_TYPE_FLOAT:
            v.f = read_float(from, prop, 0);
            set_float(to, prop, v.f);
            break;

        case CE_CDB_TYPE_BOOL:
            v.b = read_bool(from, prop, 0);
            set_bool(to, prop, v.b);
            break;

        case CE_CDB_TYPE_STR:
            v.str = (char *) read_string(from, prop, 0);
            set_string(to, prop, v.str);
            break;

        case CE_CDB_TYPE_SUBOBJECT:
            v.subobj = read_subobject(from, prop, 0);
            set_subobject(to, prop, v.subobj);
            break;

        case CE_CDB_TYPE_SET_SUBOBJECT: {
            uint64_t n = read_objset_count(from, prop);
            uint64_t k[n];
            read_objset(from, prop, k);
            for (int j = 0; j < n; ++j) {
                object_t *obj = _get_object_from_o(from);
                uint64_t new_subobj = create_from(obj->db, k[j]);
                if (new_subobj) {
                    add_obj(to, prop, new_subobj);
                }
            }
        }
            break;

        default:
            break;
    }
}


static struct ce_cdb_t0 global_db() {
    return _G.global_db;
}

void set_type(ce_cdb_t0 _db,
              uint64_t _obj,
              uint64_t type) {
    if (!_obj) {
        return;
    }

    db_t *db = _get_db(_db);

    object_t *obj = _get_object_from_uid(db, _obj);
    obj->type = type;
}


uint64_t key(ce_cdb_t0 _db,
             uint64_t _obj) {
    db_t *db = _get_db(_db);

    object_t *obj = _get_object_from_uid(db, _obj);
    return obj->key;
}

void move(ce_cdb_t0 _db,
          uint64_t _from_obj,
          uint64_t _to) {
    db_t *db = _get_db(_db);

    object_t *from = _get_object_from_uid(db, _from_obj);

    object_t **obj_addr = (object_t **) _to;

    *obj_addr = from;
}

void move_objset_obj(ce_cdb_obj_o0 *from_w,
                     ce_cdb_obj_o0 *to_w,
                     uint64_t prop,
                     uint64_t obj) {
    object_t *writer = _get_object_from_o(from_w);
    object_t *to = _get_object_from_o(to_w);

    ce_cdb_value_u0 *from_v = _get_value_ptr_generic(writer, prop);
    ce_cdb_value_u0 *to_v = _get_value_ptr(to, prop, CE_CDB_TYPE_SET_SUBOBJECT);

    if (!to_v) {
        return;
    }

    db_t *db = _get_db(writer->db);

    if (!to_v->set) {
        to_v->set = _new_set(db);
    }

    _remove_from_set(db, from_v->set, obj);
    _add_to_set(db, to_v->set, obj);

    if (obj) {
        struct object_t *subobj = _get_object_from_uid(db, obj);
        subobj->parent = to->orig_obj;
        subobj->key = prop;
    }

    _add_change(writer, (ce_cdb_prop_ev_t0) {
            .obj = writer->orig_obj,
            .prop=prop,
            .ev_type = CE_CDB_PROP_MOVE_EVENT,
            .prop_type =  CE_CDB_TYPE_SET_SUBOBJECT,
            .to = to->orig_obj,
            .value.subobj = obj,
    });
}

static void _push_space(char **buffer,
                        uint32_t level) {
    for (int j = 0; j < level; ++j) {
        ce_buffer_printf(buffer, _G.allocator, "  ");
    }
}

static void dump_str(ce_cdb_t0 _db,
                     char **buffer,
                     uint64_t from,
                     uint32_t level) {
    const ce_cdb_obj_o0 *reader = read(_db, from);

    const uint32_t prop_count = ce_cdb_a0->prop_count(reader);
    const uint64_t *keys = prop_keys(reader);

    _push_space(buffer, level);
    ce_buffer_printf(buffer, _G.allocator, "cdb_uid: 0x%llx\n", from);

    uint64_t type = ce_cdb_a0->obj_type(_db, from);
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
            const ce_cdb_obj_o0 *ir = ce_cdb_a0->read(_db, instance_of);
            if (prop_equal(ir, reader, key)) {
                continue;
            }
        }

        const char *k = ce_id_a0->str_from_id64(key);

        ce_cdb_type_e0 type = ce_cdb_a0->prop_type(reader, key);


        if ((type == CE_CDB_TYPE_BLOB) || (type == CE_CDB_TYPE_PTR)) {
//        if (type == CE_CDB_TYPE_PTR) {
            continue;
        }


        _push_space(buffer, level);

        const char *sufix = "";

        if (k) {
            ce_buffer_printf(buffer, _G.allocator, "%s%s:", k, sufix);
        } else {
            ce_buffer_printf(buffer, _G.allocator, "0x%llx%s:", key, sufix);
        }

        switch (type) {
            case CE_CDB_TYPE_SUBOBJECT: {
                uint64_t s = ce_cdb_a0->read_subobject(reader, key, 0);
                ce_buffer_printf(buffer, _G.allocator, "\n");
                dump_str(_db, buffer, s, level + 1);
            }
                break;

            case CE_CDB_TYPE_SET_SUBOBJECT: {
                ce_buffer_printf(buffer, _G.allocator, "\n");

                uint64_t num = read_objset_count(reader, key);
                uint64_t objs[num];
                read_objset(reader, key, objs);

                _push_space(buffer, level + 1);
                ce_buffer_printf(buffer, _G.allocator, "cdb_type: cdb_objset\n");

                for (int j = 0; j < num; ++j) {
                    uint64_t obj = objs[j];

                    _push_space(buffer, level + 1);
                    ce_buffer_printf(buffer, _G.allocator, "0x%llx:\n", obj);

                    dump_str(_db, buffer, obj, level + 2);
                }
            }
                break;


            case CE_CDB_TYPE_FLOAT: {
                float f = ce_cdb_a0->read_float(reader, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " %f\n", f);
            }
                break;

            case CE_CDB_TYPE_STR: {
                const char *s = ce_cdb_a0->read_str(reader, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " %s\n", s);
            }
                break;

            case CE_CDB_TYPE_BOOL: {
                bool b = ce_cdb_a0->read_bool(reader, key, 0);
                if (b) {
                    ce_buffer_printf(buffer, _G.allocator, " true\n");
                } else {
                    ce_buffer_printf(buffer, _G.allocator, " false\n");
                }
            }
                break;

            case CE_CDB_TYPE_NONE: {
                ce_buffer_printf(buffer, _G.allocator, " none\n");
            }
                break;

            case CE_CDB_TYPE_UINT64: {
                uint64_t i = ce_cdb_a0->read_uint64(reader, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " %llu\n", i);
            }
                break;

            case CE_CDB_TYPE_REF: {
                uint64_t ref = ce_cdb_a0->read_ref(reader, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " 0x%llx\n", ref);
            }
                break;
//            case CE_CDB_TYPE_BLOB:{
//                uint64_t size = 0;
//                ce_cdb_a0->read_blob(reader, key, &size, NULL);
//                ce_buffer_printf(buffer, _G.allocator, " BLOB %llu\n", size);
//            }
                break;
            default:
                break;
        }
    }

}

void log_obj(const char *where,
             ce_cdb_t0 db,
             uint64_t obj) {
    char *buffer = NULL;
    dump_str(db, &buffer, obj, 0);
    ce_log_a0->debug(where, "%s", buffer);
    ce_buffer_free(buffer, ce_memory_a0->system);

}

static uint64_t find_root(ce_cdb_t0 _db,
                          uint64_t _obj) {
    db_t *db = _get_db(_db);

    object_t *obj = _get_object_from_uid(db, _obj);

    if (!obj) {
        return 0;
    }

    if (!obj->parent) {
        return _obj;
    }

    return find_root(_db, obj->parent);
}

static void _dispatch_instances(ce_cdb_t0 db,
                                struct object_t *orig_obj) {
    const int changed_prop_n = ce_array_size(orig_obj->changed);

    if (!changed_prop_n) {
        return;
    }

    const int instances_n = ce_array_size(orig_obj->instances);
    for (int i = 0; i < instances_n; ++i) {
        uint64_t inst_obj = orig_obj->instances[i];

        ce_cdb_obj_o0 *w = write_begin(db, inst_obj);
        const uint32_t chn = ce_array_size(orig_obj->changed);
        for (int j = 0; j < chn; ++j) {
            ce_cdb_prop_ev_t0 *ev = &orig_obj->changed[j];
            ce_cdb_type_e0 t = prop_type((ce_cdb_obj_o0 *) orig_obj, ev->prop);

            if (ev->ev_type == CE_CDB_PROP_CHANGE_EVENT) {
                switch (t) {
                    case CE_CDB_TYPE_NONE:
                        break;

                    case CE_CDB_TYPE_UINT64: {
                        uint64_t u = read_uint64(w, ev->prop, 0);
                        if (u == ev->old_value.uint64) {
                            set_uint64(w, ev->prop, ev->new_value.uint64);
                        }
                    }
                        break;

                    case CE_CDB_TYPE_FLOAT: {
                        float f = read_float(w, ev->prop, 0);
                        if (f == ev->old_value.f) {
                            set_float(w, ev->prop, ev->new_value.f);
                        }
                    }
                        break;

                    case CE_CDB_TYPE_BOOL: {
                        bool b = read_bool(w, ev->prop, 0);
                        if (b == ev->old_value.b) {
                            set_bool(w, ev->prop, ev->new_value.b);
                        }
                    }
                        break;

                    case CE_CDB_TYPE_STR: {
                        const char *str = read_string(w, ev->prop, 0);
                        if (!str || !ev->old_value.str || !strcmp(str, ev->old_value.str)) {
                            set_string(w, ev->prop, ev->new_value.str);
                        }
                    }
                        break;

                    case CE_CDB_TYPE_PTR: {
                        void *ptr = read_ptr(w, ev->prop, 0);
                        if (ptr == ev->old_value.ptr) {
                            set_ptr(w, ev->prop, ev->new_value.ptr);
                        }
                    }
                        break;

                    case CE_CDB_TYPE_SUBOBJECT: {
                        uint64_t new_so = create_from(db, ev->new_value.subobj);
                        set_subobject(w, ev->prop, new_so);
                    }

                        break;

                    case CE_CDB_TYPE_REF: {
                        uint64_t ref = read_ref(w, ev->prop, 0);
                        if (ref == ev->old_value.ref) {
                            set_ref(w, ev->prop, ev->new_value.ref);
                        }
                    }
                        break;

                    default:
                        break;
                }
            } else if (ev->ev_type == CE_CDB_OBJSET_ADD_EVENT) {
                uint64_t obj = ev->new_value.subobj;
                uint64_t new_obj = create_from(db, obj);
                ce_cdb_a0->objset_add_obj(w, ev->prop, new_obj);
            } else if (ev->ev_type == CE_CDB_OBJSET_REMOVE_EVENT) {
                ce_cdb_a0->objset_remove_obj(w, ev->prop, ev->old_value.subobj);
            }
        }
        write_commit(w);
    }
}

const struct ce_cdb_prop_ev_t0 *changed(const ce_cdb_obj_o0 *reader,
                                        uint32_t *n) {
    object_t *obj = (object_t *) reader;
    *n = ce_array_size(obj->changed);
    if (*n) {
        return obj->changed;
    }
    return NULL;
}

ct_cdb_ev_queue_o0 *add_changed_obj_listener(ce_cdb_t0 _db) {
    db_t *db = _get_db(_db);
    return (ct_cdb_ev_queue_o0 *) _new_changed_obj_events_listener(db);
}

bool pop_changed_obj(ct_cdb_ev_queue_o0 *q,
                     ce_cdb_ev_t0 *ev) {
    ce_mpmc_queue_t0 *qq = (ce_mpmc_queue_t0 *) q;
    return ce_mpmc_dequeue(qq, ev);
}

ct_cdb_ev_queue_o0 *add_obj_listener(ce_cdb_t0 _db) {
    db_t *db = _get_db(_db);
    return (ct_cdb_ev_queue_o0 *) _new_obj_events_listener(db);
}

ct_cdb_ev_queue_o0 *add_obj_listener2(ce_cdb_t0 _db,
                                      uint64_t _obj) {
    db_t *db = _get_db(_db);
    return (ct_cdb_ev_queue_o0 *) _new_obj_events_listener2(db, _obj);
}

bool pop_obj_events(ct_cdb_ev_queue_o0 *q,
                    ce_cdb_prop_ev_t0 *ev) {
    ce_mpmc_queue_t0 *qq = (ce_mpmc_queue_t0 *) q;
    return ce_mpmc_dequeue(qq, ev);
}


bool pop_obj_events2(ct_cdb_ev_queue_o0 *q,
                     ce_cdb_prop_ev_t0 *ev) {
    ce_mpmc_queue_t0 *qq = (ce_mpmc_queue_t0 *) q;
    return ce_mpmc_dequeue(qq, ev);
}

const uint64_t *destroyed(ce_cdb_t0 _db,
                          uint32_t *n) {
    db_t *db = _get_db(_db);

    *n = db->to_free_objects_uid_n;

    return db->to_free_objects_uid;
}

const uint64_t *changed_objects(ce_cdb_t0 _db,
                                uint32_t *n) {
    db_t *db = _get_db(_db);


    uint32_t ch_n = ce_array_size(db->changed_obj);
    if (!ch_n) {
        *n = 0;
        return NULL;
    }

    *n = ch_n;
    return db->changed_obj;
}

void set_parent(ce_cdb_t0 _db,
                uint64_t object,
                uint64_t parent) {
    db_t *db = _get_db(_db);

    object_t *from = _get_object_from_uid(db, object);
    from->parent = parent;
}

uint64_t read_instance_of(const ce_cdb_obj_o0 *reader) {
    object_t *obj = _get_object_from_o(reader);
    if (!obj) {
        return 0;
    }

    return obj->instance_of;
}

static void load(ce_cdb_t0 db,
                 const char *input,
                 uint64_t _obj,
                 struct ce_alloc_t0 *allocator) {
    db_t *db_inst = _get_db(db);

    const cdb_binobj_header *header = (const cdb_binobj_header *) input;

    cnode_t *cnodes = (cnode_t *) (header + 1);
    const char *strbuffer = (char *) (cnodes + header->node_count);
    const char *blob_buffer = (char *) (strbuffer + header->string_buffer_size);

    struct state_t {
        ce_cdb_obj_o0 *writer;
        bool is_set;
        uint64_t key;
    } *states = NULL;

    uint64_t n = header->node_count;
    for (int i = 0; i < n; ++i) {
        cnode_t node = cnodes[i];

        uint32_t state_n = ce_array_size(states);
        uint32_t state_top = state_n - 1;

        switch (node.type) {
            case CNODE_INVALID: {
            }
                break;
            case CNODE_FLOAT: {
                struct state_t *state = &states[state_top];
                _set_float(state->writer, node.key, node.value.f, false);
            }
                break;
            case CNODE_UINT: {
                struct state_t *state = &states[state_top];
                _set_uint64(state->writer, node.key, node.value.uint64, false);
            }
                break;
            case CNODE_REF: {
                struct state_t *state = &states[state_top];
                _set_ref(state->writer, node.key, node.value.ref, false);
            }
                break;

            case CNODE_BOOL: {
                struct state_t *state = &states[state_top];
                _set_bool(state->writer, node.key, node.value.b, false);
            }
                break;

            case CNODE_STRING: {
                struct state_t *state = &states[state_top];
                const char *str = &strbuffer[node.value.uint64];
                _set_string(state->writer, node.key, str, false);
            }
                break;

            case CNODE_BLOB: {
                struct state_t *state = &states[state_top];
                const void *blob = &blob_buffer[node.value.uint64];
                _set_blob(state->writer, node.key, blob, node.blob.size, false);
            }
                break;

            case CNODE_OBJ_BEGIN: {
                uint64_t uid = node.obj.uid;
                if (node.obj.instance_of) {
                    _create_from_uid(db, node.obj.instance_of, uid, true);
                } else {
                    create_object_uid(db, uid, node.obj.type, true);
                }

                object_t *obj = _get_object_from_uid(db_inst, uid);
                obj->key = node.key;

                if (state_n) {
                    struct state_t *state = &states[state_top];
                    if (state->is_set) {
                        struct state_t *parent_obj = &states[state_top - 1];
                        _add_obj(parent_obj->writer, state->key, uid, false);
                    } else {
                        _set_subobject(state->writer, node.key, uid, false);
                    }
                }

                struct state_t new_state = {
                        .writer = (ce_cdb_obj_o0 *) obj,
                };

                ce_array_push(states, new_state, _G.allocator);
            }
                break;

            case CNODE_OBJ_END: {
//                struct state_t *state = &states[state_top];
//                write_commit(state->writer);
                ce_array_pop_back(states);
            }
                break;

            case CNODE_OBJSET: {
                struct state_t new_state = {
                        .is_set = true,
                        .key = node.key,
                };
                ce_array_push(states, new_state, _G.allocator);
            }
                break;

            case CNODE_OBJSET_END: {
                ce_array_pop_back(states);
            }
                break;
        }
    }
}

void _init_from_defs(ce_cdb_t0 db,
                     object_t *obj,
                     const ce_cdb_type_def_t0 *def) {

    ce_cdb_obj_o0 *w = (ce_cdb_obj_o0 *) obj;

    for (uint32_t i = 0; i < def->num; ++i) {
        ce_cdb_prop_def_t0 *prop_def = &def->defs[i];
        uint64_t prop_name = ce_id_a0->id64(prop_def->name);
        ce_cdb_type_e0 t = prop_def->type;

        switch (t) {
            case CE_CDB_TYPE_NONE:
                break;
            case CE_CDB_TYPE_UINT64:
                set_uint64(w, prop_name, prop_def->value.uint64);
                break;
            case CE_CDB_TYPE_PTR:
                set_ptr(w, prop_name, prop_def->value.ptr);
                break;
            case CE_CDB_TYPE_REF:
                set_ref(w, prop_name, prop_def->value.ref);
                break;
            case CE_CDB_TYPE_FLOAT:
                set_float(w, prop_name, prop_def->value.f);
                break;
            case CE_CDB_TYPE_BOOL:
                set_bool(w, prop_name, prop_def->value.b);
                break;
            case CE_CDB_TYPE_STR:
                set_string(w, prop_name, prop_def->value.str);
                break;
            case CE_CDB_TYPE_SUBOBJECT: {
                uint64_t sub_obj = create_object(db, prop_def->obj_type);
                set_subobject(w, prop_name, sub_obj);
            }
                break;
            case CE_CDB_TYPE_BLOB: {
                set_blob(w, prop_name, NULL, 0);
                break;
            }

            case CE_CDB_TYPE_SET_SUBOBJECT: {
                break;
            }
        }
    }
}

static struct ce_cdb_a0 cdb_api = {
        .create_db = create_db,
        .destroy_db = destroy_db,

        .reg_obj_type = reg_obj_type,
        .obj_type_def = _get_prop_def,
        .gen_uid = gen_uid,
        .set_loader = set_loader,
        .db  = global_db,
        .obj_type = type,
        .set_type = set_type,
        .move_obj = move,
        .move_objset_obj = move_objset_obj,

        .create_object = create_object,
        .create_object_uid = create_object_uid,
        .create_from = create_from,
        .create_from_uid = create_from_uid,
        .set_instance_of = set_instance_of,
        .destroy_object = destroy_object,

        .gc = gc,

        .dump_str = dump_str,
        .log_obj = log_obj,
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

        .read = read,
        .read_to = read_to,
        .read_instance_of = read_instance_of,
        .new_changed_obj_listener = add_changed_obj_listener,
        .pop_changed_obj = pop_changed_obj,
        .new_objs_listener = add_obj_listener,
        .pop_objs_events = pop_obj_events,
        .new_obj_listener = add_obj_listener2,
        .pop_obj_events = pop_obj_events2,
        .read_float = read_float,
        .read_bool = read_bool,
        .read_str = read_string,
        .read_uint64 = read_uint64,
        .read_ptr = read_ptr,
        .read_ref = read_ref,
        .read_blob = read_blob,
        .read_subobject = read_subobject,
        .read_objset = read_objset,
        .read_objset_num = read_objset_count,

        .write_begin = write_begin,
        .write_commit = write_commit,
        .write_try_commit = write_try_commit,

        .set_float = set_float,
        .set_bool = set_bool,
        .set_str = set_string,
        .set_uint64 = set_uint64,
        .set_ptr = set_ptr,
        .set_ref = set_ref,
        .set_subobject = set_subobject,
        .set_blob = set_blob,

        .objset_add_obj = add_obj,
        .objset_remove_obj = remove_obj,
};

struct ce_cdb_a0 *ce_cdb_a0 = &cdb_api;

void CE_MODULE_LOAD(cdb)(struct ce_api_a0 *api,
                         int reload) {
    CE_UNUSED(reload);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    _G.global_db = create_db(MAX_OBJECTS);

    api->register_api(CE_CDB_API, &cdb_api, sizeof(cdb_api));
}

void CE_MODULE_UNLOAD(cdb)(struct ce_api_a0 *api,
                           int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _G = (struct _G) {};
}
