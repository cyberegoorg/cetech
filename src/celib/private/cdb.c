#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

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


#define _G coredb_global
#define LOG_WHERE "coredb"

#define MAX_OBJECTS 1000000000ULL
#define MAX_FREE_OBJECE_POOL 10000
#define MAX_FREE_OBJECE_ID_POOL 10000

// TODO: non optimal braindump code
// TODO: remove null element

struct notify_pair {
    ce_cdb_notify notify;
    void *data;
};

// TODO: X(
struct object_t {
    struct notify_pair *notify;
    struct notify_pair *remove_notify;

    // prefab
    uint64_t prefab;
    uint64_t *instances;

    // hiearchy
    uint64_t key;
    uint64_t parent;
//    uint64_t *children;

    // writer
    uint64_t orig_data_idx;
    uint64_t orig_obj;
    uint64_t *changed_prop;
    uint64_t *removed_prop;

    // object
    uint64_t type;
    uint64_t idx;
    struct ce_cdb_t db;
    struct ce_hash_t prop_map;
    struct ce_hash_t removed_prop_map;

    uint64_t properties_count;

    uint64_t *keys;
    uint8_t *property_type;
    uint64_t *offset;
    uint8_t *values;
};

struct db_t {
    uint32_t idx;

    // id pool
    uint64_t *objects_mem;
    atomic_ullong object_used;

    uint32_t *free_objects_id;
    uint32_t *to_free_objects_id;
    atomic_ullong to_free_objects_id_n;
    atomic_ullong free_objects_id_n;

//    // Hiearchy
//    uint32_t *first_child;
//    uint32_t *next_sibling;
//    uint32_t *parent;

    // objects
    struct object_t *object_pool;
    uint32_t *free_objects;
    uint32_t *to_free_objects;

    atomic_uint_least64_t object_pool_n;
    atomic_uint_least64_t free_objects_n;
    atomic_uint_least64_t to_free_objects_n;
};

static struct _G {
    struct db_t *dbs;
    uint32_t *free_db;
    uint32_t *to_free_db;

    struct ce_alloc *allocator;
    struct ce_cdb_t global_db;
} _G;

struct blob_t {
    void *data;
    uint64_t size;
};

union type_u {
    uint64_t uint64;
    void *ptr;
    uint64_t ref;
    uint64_t subobj;

    float f;
    char *str;
    bool b;

    float vec3[3];
    float vec4[4];
    float mat4[16];
    struct blob_t blob;
};

static struct object_t *_get_object_from_objid(uint64_t objid) {
    if (!objid) {
        return NULL;
    }

    uint64_t idx = *(uint64_t *) objid;

    return &_G.dbs[0].object_pool[idx];
}

static struct object_t *_get_object_from_obj_o(ce_cdb_obj_o *obj_o) {
    uint64_t idx = (uint64_t) obj_o;

    return &_G.dbs[0].object_pool[idx];
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

    ce_hash_remove(&obj->removed_prop_map, key);

    if (size) {
        ce_array_resize(obj->values, values_size + size, alloc);
    }

    ce_hash_add(&obj->prop_map, key, prop_count, alloc);

    obj->properties_count = obj->properties_count + 1;
    return prop_count;
}

struct object_t *_new_object(struct db_t *db,
                             const struct ce_alloc *a) {

    uint32_t idx;
    if (db->free_objects_n) {
        atomic_fetch_sub(&db->free_objects_n, 1);
        idx = db->free_objects[db->free_objects_n];
    } else {
        idx = atomic_fetch_add(&db->object_pool_n, 1);

        db->object_pool[idx] = (struct object_t) {};
    }


    struct object_t *obj = &db->object_pool[idx];
    obj->idx = idx;
    obj->db.idx = db->idx;
    obj->key = 0;

    _object_new_property(obj, 0, CDB_TYPE_NONE, NULL, 0, a);
    return obj;
}

static uint64_t _new_object_id(struct db_t *db_inst) {
    uint64_t idx = 0;
    if (db_inst->free_objects_id_n) {
        atomic_fetch_sub(&db_inst->free_objects_id_n, 1);
        idx = db_inst->free_objects_id[db_inst->free_objects_id_n];
    } else {
        idx = atomic_fetch_add(&db_inst->object_used, 1);
    }

    return idx;
}

static struct object_t *_object_clone(struct db_t *db,
                                      struct object_t *obj,
                                      const struct ce_alloc *alloc) {
    const uint64_t properties_count = obj->properties_count;
    const uint64_t values_size = ce_array_size(obj->values);

    struct object_t *new_obj = _new_object(db, alloc);

    new_obj->prefab = obj->prefab;
    new_obj->parent = obj->parent;
    new_obj->key = obj->key;

    new_obj->type = obj->type;
    new_obj->properties_count = properties_count;

    ce_array_push_n(new_obj->keys, obj->keys + 1,
                    ce_array_size(obj->keys) - 1, alloc);

    ce_array_push_n(new_obj->property_type, obj->property_type + 1,
                    ce_array_size(obj->property_type) - 1, alloc);

    ce_array_push_n(new_obj->offset, obj->offset + 1,
                    ce_array_size(obj->offset) - 1, alloc);

    if (values_size)
        ce_array_push_n(new_obj->values, obj->values, values_size, alloc);


    ce_hash_clone(&obj->prop_map, &new_obj->prop_map, alloc);
    ce_hash_clone(&obj->removed_prop_map, &new_obj->removed_prop_map, alloc);

    uint32_t n = ce_array_size(obj->instances);
    if (n) {
        ce_array_push_n(new_obj->instances, obj->instances, n, alloc);
    }

//    n = ce_array_size(obj->children);
//    if (n) {
//        ce_array_push_n(new_obj->children, obj->children, n, alloc);
//    }

    n = ce_array_size(obj->notify);
    if (n) {
        ce_array_push_n(new_obj->notify, obj->notify, n, alloc);
    }

    n = ce_array_size(obj->remove_notify);
    if (n) {
        ce_array_push_n(new_obj->remove_notify, obj->remove_notify, n, alloc);
    }

    return new_obj;
}

static uint64_t _find_prop_index(const struct object_t *obj,
                                 uint64_t key) {
    if (ce_hash_contain(&obj->removed_prop_map, key)) {
        return 0;
    }

    return ce_hash_lookup(&obj->prop_map, key, 0);
}

static void _destroy_object(struct object_t *obj) {
    struct db_t *db_inst = &_G.dbs[obj->db.idx];

    uint64_t idx = atomic_fetch_add(&db_inst->to_free_objects_n, 1);
    db_inst->to_free_objects[idx] = obj->idx;
}

static struct ce_cdb_t create_db() {
    uint64_t idx = ce_array_size(_G.dbs);

    struct db_t db = (struct db_t) {
            .idx = idx,

            .objects_mem = (uint64_t *) mmap(NULL,
                                             MAX_OBJECTS * sizeof(uint64_t),
                                             PROT_READ | PROT_WRITE,
                                             MAP_PRIVATE | MAP_ANONYMOUS |
                                             MAP_NORESERVE,
                                             -1, 0),


            .free_objects_id = (uint32_t *) mmap(NULL,
                                                 MAX_OBJECTS * sizeof(uint32_t),
                                                 PROT_READ | PROT_WRITE,
                                                 MAP_PRIVATE | MAP_ANONYMOUS |
                                                 MAP_NORESERVE,
                                                 -1, 0),

            .to_free_objects_id = (uint32_t *) mmap(NULL,
                                                    MAX_OBJECTS *
                                                    sizeof(uint32_t),
                                                    PROT_READ | PROT_WRITE,
                                                    MAP_PRIVATE |
                                                    MAP_ANONYMOUS |
                                                    MAP_NORESERVE,
                                                    -1, 0),
            .object_pool = (struct object_t *) mmap(NULL,
                                                    MAX_OBJECTS *
                                                    sizeof(struct object_t),
                                                    PROT_READ | PROT_WRITE,
                                                    MAP_PRIVATE |
                                                    MAP_ANONYMOUS |
                                                    MAP_NORESERVE,
                                                    -1, 0),
            .free_objects = (uint32_t *) mmap(NULL,
                                              MAX_OBJECTS * sizeof(uint32_t),
                                              PROT_READ | PROT_WRITE,
                                              MAP_PRIVATE | MAP_ANONYMOUS |
                                              MAP_NORESERVE,
                                              -1, 0),

            .to_free_objects = (uint32_t *) mmap(NULL,
                                                 MAX_OBJECTS * sizeof(uint32_t),
                                                 PROT_READ | PROT_WRITE,
                                                 MAP_PRIVATE | MAP_ANONYMOUS |
                                                 MAP_NORESERVE,
                                                 -1, 0)
    };

    ce_array_push(_G.dbs, db, _G.allocator);
    return (struct ce_cdb_t) {.idx = idx};
};

static uint64_t create_object(struct ce_cdb_t db,
                              uint64_t type) {
    struct db_t *db_inst = &_G.dbs[db.idx];
    struct object_t *obj = _new_object(db_inst, _G.allocator);

    uint64_t idx = _new_object_id(db_inst);

    uint64_t *obj_addr = db_inst->objects_mem + idx;

    *obj_addr = obj->idx;
    obj->db = db;
    obj->type = type;

    return (uint64_t) obj_addr;
}

static ce_cdb_obj_o *write_begin(uint64_t _obj);

static void write_commit(ce_cdb_obj_o *_writer);

void set_subobject(ce_cdb_obj_o *_writer,
                   uint64_t property,
                   uint64_t subobject);

static uint64_t create_from(struct ce_cdb_t db,
                            uint64_t _obj) {
    struct db_t *db_inst = &_G.dbs[db.idx];

    struct object_t *obj = _get_object_from_objid(_obj);

    struct object_t *inst = _new_object(db_inst, _G.allocator);
    inst->db = db;

    uint64_t idx = _new_object_id(db_inst);

    uint64_t *obj_addr = db_inst->objects_mem + idx;

    *obj_addr = inst->idx;

    inst->prefab = _obj;
    inst->type = obj->type;

    ce_array_push(obj->instances, (uint64_t) obj_addr, _G.allocator);

//    uint32_t n = ce_array_size(obj->notify);
//    if (n) {
//        ce_array_push_n(inst->notify, obj->notify, n, _G.allocator);
//    }
//
//    n = ce_array_size(obj->remove_notify);
//    if (n) {
//        ce_array_push_n(inst->remove_notify, obj->remove_notify, n, _G.allocator);
//    }

    struct object_t *wr = write_begin((uint64_t) obj_addr);

    for (int i = 1; i < obj->properties_count; ++i) {
        switch (obj->property_type[i]) {
            case CDB_TYPE_SUBOBJECT: {
                union type_u *value_ptr = (union type_u *) (obj->values +
                                                            obj->offset[i]);

                uint64_t old_subobj = value_ptr->subobj;

                uint64_t new_subobj = create_from(db, old_subobj);

                set_subobject(wr, obj->keys[i], new_subobj);
            }

                break;

            default:
                break;
        }
    }

    write_commit(wr);

    return (uint64_t) obj_addr;
}

static void destroy_db(struct ce_cdb_t db) {
    ce_array_push(_G.to_free_db, db.idx, _G.allocator);
}

static void destroy_object(uint64_t _obj) {

    struct object_t *obj = _get_object_from_objid(_obj);
    struct db_t *db_inst = &_G.dbs[obj->db.idx];

    uint64_t idx = atomic_fetch_add(&db_inst->to_free_objects_id_n, 1);

    db_inst->to_free_objects_id[idx] = \
            ((uint64_t *) _obj) - db_inst->objects_mem;

    for (int i = 1; i < obj->properties_count; ++i) {
        switch (obj->property_type[i]) {
            case CDB_TYPE_SUBOBJECT: {
                union type_u *value_ptr = (union type_u *) (obj->values +
                                                            obj->offset[i]);

                uint64_t old_subobj = value_ptr->subobj;
                destroy_object(old_subobj);
            }

                break;

            default:
                break;
        }
    }

}


static void gc() {
    const uint32_t db_n = ce_array_size(_G.dbs);
    for (int i = 0; i < db_n; ++i) {
        struct db_t *db_inst = &_G.dbs[i];

        const uint32_t to_free_objects_id_n = db_inst->to_free_objects_id_n;

        for (int j = 0; j < to_free_objects_id_n; ++j) {
            const uint32_t idx = db_inst->to_free_objects_id[j];

            uint64_t *obj_addr = db_inst->objects_mem + idx;
            struct object_t *obj = _get_object_from_objid((uint64_t) obj_addr);

            if (obj->prefab) {
                struct object_t *prefab_obj = \
                    _get_object_from_objid(obj->prefab);

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

            _destroy_object(obj);

            db_inst->free_objects_id[atomic_fetch_add(
                    &db_inst->free_objects_id_n, 1)] = idx;
        }

        db_inst->to_free_objects_id_n = 0;

        const uint32_t to_free_objects_n = db_inst->to_free_objects_n;
        for (int j = 0; j < to_free_objects_n; ++j) {
            const uint32_t idx = db_inst->to_free_objects[j];

            struct object_t *obj = &db_inst->object_pool[idx];

//            ce_array_clean(obj->children);
            ce_array_clean(obj->instances);

            ce_array_clean(obj->property_type);
            ce_array_clean(obj->keys);
            ce_array_clean(obj->values);
            ce_array_clean(obj->offset);

            ce_hash_clean(&obj->prop_map);
            ce_hash_clean(&obj->removed_prop_map);

            ce_array_clean(obj->changed_prop);
            ce_array_clean(obj->removed_prop);
            ce_array_clean(obj->notify);
            ce_array_clean(obj->remove_notify);

            *obj = (struct object_t) {
//                    .children = obj->children,
                    .instances = obj->instances,
                    .property_type = obj->property_type,
                    .keys = obj->keys,
                    .values = obj->values,
                    .offset = obj->offset,
                    .prop_map = obj->prop_map,
                    .removed_prop_map = obj->removed_prop_map,
                    .changed_prop = obj->changed_prop,
                    .removed_prop = obj->removed_prop,
                    .notify = obj->notify,
                    .remove_notify = obj->remove_notify,
            };


            db_inst->free_objects[atomic_fetch_add(&db_inst->free_objects_n,
                                                   1)] = idx;
        }

        db_inst->to_free_objects_n = 0;
    }
}

struct cdb_binobj_header {
    uint64_t version;
    uint64_t type;
    uint64_t properties_count;
    uint64_t values_size;
    uint64_t string_buffer_size;
    uint64_t subobject_buffer_size;
    uint64_t blob_buffer_size;
};

static void dump(uint64_t _obj,
                 char **output,
                 struct ce_alloc *allocator) {
    struct object_t *obj = _get_object_from_objid(_obj);

    uint64_t *keys = obj->keys;
    uint8_t *type = obj->property_type;
    uint64_t *offset = obj->offset;
    uint8_t *values = obj->values;

    uint8_t *values_copy = CE_ALLOC(allocator, uint8_t, ce_array_size(values));
    memcpy(values_copy, values, ce_array_size(values));

    char *str_buffer = NULL;
    char *subobject_buffer = NULL;
    char *blob_buffer = NULL;
    for (int i = 1; i < obj->properties_count; ++i) {
        switch (obj->property_type[i]) {
            case CDB_TYPE_SUBOBJECT: {
                uint64_t subobject_offset = ce_array_size(subobject_buffer);
                uint64_t subobject_ptr;

                union type_u *value_ptr = (union type_u *) (values_copy +
                                                            offset[i]);
                subobject_ptr = value_ptr->subobj;

                dump(subobject_ptr, &subobject_buffer, allocator);

                uint64_t *ptr = (uint64_t *) (values_copy + offset[i]);
                *ptr = subobject_offset;
            }

                break;

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

                struct blob_t *blob;
                blob = (struct blob_t *) (values_copy + offset[i]);

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
            .properties_count = obj->properties_count - 1,
            .values_size = ce_array_size(values),
            .string_buffer_size = ce_array_size(str_buffer),
            .subobject_buffer_size = ce_array_size(subobject_buffer),
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

    ce_array_push_n(*output, (char *) subobject_buffer,
                    header.subobject_buffer_size,
                    allocator);

    ce_array_push_n(*output, (char *) blob_buffer,
                    header.blob_buffer_size,
                    allocator);

    CE_FREE(allocator, values_copy);
}

static void load(struct ce_cdb_t db,
                 const char *input,
                 uint64_t _obj,
                 struct ce_alloc *allocator) {

    const struct cdb_binobj_header *header;
    header = (const struct cdb_binobj_header *) input;

    struct object_t *obj = _get_object_from_objid(_obj);
    if (!obj->type) {
        obj->type = header->type;
    }

    uint64_t *keys = (uint64_t *) (header + 1);
    uint8_t *ptype = (uint8_t *) (keys + header->properties_count);
    uint64_t *offset = (uint64_t *) (ptype + header->properties_count);
    uint8_t *values = (uint8_t *) (offset + header->properties_count);
    const char *strbuffer = (char *) (values + header->values_size);
    const char *subobject_buffer = (char *) (strbuffer +
                                             header->string_buffer_size);
    const char *blob_buffer = (char *) (subobject_buffer +
                                        header->subobject_buffer_size);
    if (!header->properties_count) {
        return;
    }

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
            case CDB_TYPE_SUBOBJECT: {
                uint64_t suboffset = *(uint64_t *) (obj->values +
                                                    obj->offset[i]);

                const char *subobj_data;
                subobj_data = subobject_buffer + suboffset;

                uint64_t subobj = create_object(db, 0);
                load(obj->db, subobj_data, subobj, allocator);

                struct object_t *sub_obj = _get_object_from_objid(subobj);
                sub_obj->parent = _obj;

                union type_u *value_ptr = (union type_u *) (obj->values +
                                                            obj->offset[i]);
                value_ptr->subobj = subobj;
            }

                break;

            case CDB_TYPE_STR: {
                uint64_t str_offset = *(uint64_t *) (obj->values +
                                                     obj->offset[i]);

                char *dup_str = ce_memory_a0->str_dup(strbuffer + str_offset,
                                                      allocator);

                union type_u *value_ptr = (union type_u *) (obj->values +
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

                union type_u *value_ptr = (union type_u *) (obj->values +
                                                            obj->offset[i]);
                value_ptr->blob.size = size;
                value_ptr->blob.data = copy_blob_data;
            }
                break;
            default:
                break;
        }

    }
}

static ce_cdb_obj_o *write_begin(uint64_t _obj) {
    struct object_t *obj = _get_object_from_objid(_obj);
    struct db_t *db_inst = &_G.dbs[obj->db.idx];

    struct object_t *new_obj = _object_clone(db_inst,
                                             obj,
                                             _G.allocator);
    new_obj->orig_data_idx = obj->idx;

    new_obj->orig_obj = _obj;

    return (void *) new_obj->idx;
}

static void _notify(struct object_t *obj,
                    uint64_t notify_obj,
                    struct notify_pair *notify,
                    uint64_t *changed_prop) {

    const int notify_n = ce_array_size(notify);
    const int changed_prop_n = ce_array_size(changed_prop);

    if (!changed_prop_n) {
        return;
    }

    for (int i = 0; i < notify_n; ++i) {
        struct notify_pair *pair = &notify[i];
        pair->notify(notify_obj, changed_prop, changed_prop_n, pair->data);
    }

    const int instances_n = ce_array_size(obj->instances);
    for (int i = 0; i < instances_n; ++i) {
        struct object_t *instance_obj = _get_object_from_objid(
                obj->instances[i]);
        _notify(instance_obj, obj->instances[i], notify, changed_prop);
    }
}

static void write_commit(ce_cdb_obj_o *_writer) {
    struct object_t *writer = _get_object_from_obj_o(_writer);
    struct object_t *orig_obj = _get_object_from_objid(writer->orig_obj);

    _notify(orig_obj, writer->orig_obj, writer->remove_notify,
            writer->removed_prop);

    uint64_t *obj_addr = (uint64_t *) writer->orig_obj;

    *obj_addr = writer->idx;

//    (*(struct object_t **) writer->obj) = writer;

    _notify(orig_obj, writer->orig_obj, writer->notify, writer->changed_prop);

    ce_array_clean(writer->removed_prop);
    ce_array_clean(writer->changed_prop);

    _destroy_object(orig_obj);
}

static bool write_try_commit(ce_cdb_obj_o *_writer) {
    struct object_t *writer = _get_object_from_obj_o(_writer);
    struct object_t *orig_obj = _get_object_from_objid(writer->orig_obj);

    _notify(orig_obj, writer->orig_obj, writer->remove_notify,
            writer->removed_prop);

    uint64_t *obj_addr = (uint64_t *) writer->orig_obj;

    bool ok = atomic_compare_exchange_weak((atomic_ullong *) obj_addr,
                                           &writer->orig_data_idx, writer->idx);

    if (!ok) {
        goto end;
    }


    _notify(writer, writer->orig_obj, writer->notify, writer->changed_prop);

    end:
    _destroy_object(orig_obj);
    return ok;
}


static void set_float(ce_cdb_obj_o *_writer,
                      uint64_t property,
                      float value) {
    struct object_t *writer = _get_object_from_obj_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_FLOAT, &value,
                                   sizeof(float),
                                   _G.allocator);
    }

    ce_array_push(writer->changed_prop, property, _G.allocator);

    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);
    value_ptr->f = value;
}

static void set_bool(ce_cdb_obj_o *_writer,
                     uint64_t property,
                     bool value) {
    struct object_t *writer = _get_object_from_obj_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_BOOL, &value,
                                   sizeof(bool),
                                   _G.allocator);
    }

    ce_array_push(writer->changed_prop, property, _G.allocator);

    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);
    value_ptr->b = value;
}

static void set_vec3(ce_cdb_obj_o *_writer,
                     uint64_t property,
                     const float *value) {
    const size_t size = sizeof(float) * 3;

    struct object_t *writer = _get_object_from_obj_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_VEC3, value,
                                   size, _G.allocator);
    }

    ce_array_push(writer->changed_prop, property, _G.allocator);

    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);
    memcpy(value_ptr->vec3, value, size);
}

static void set_vec4(ce_cdb_obj_o *_writer,
                     uint64_t property,
                     const float *value) {
    const size_t size = sizeof(float) * 4;

    struct object_t *writer = _get_object_from_obj_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_VEC4, value,
                                   size, _G.allocator);
    }

    ce_array_push(writer->changed_prop, property, _G.allocator);

    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);
    memcpy(value_ptr->vec4, value, size);
}

static void set_mat4(ce_cdb_obj_o *_writer,
                     uint64_t property,
                     const float *value) {
    const size_t size = sizeof(float) * 16;
    struct object_t *writer = _get_object_from_obj_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_MAT4, value,
                                   size, _G.allocator);
    }

    ce_array_push(writer->changed_prop, property, _G.allocator);

    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);
    memcpy(value_ptr->mat4, value, size);
}

static void set_string(ce_cdb_obj_o *_writer,
                       uint64_t property,
                       const char *value) {
    struct ce_alloc *a = _G.allocator;

    struct object_t *writer = _get_object_from_obj_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);

    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_STR,
                                   &value, sizeof(char *),
                                   _G.allocator);
    } else {
        union type_u *value_ptr = (union type_u *) (writer->values +
                                                    writer->offset[idx]);

        CE_FREE(a, value_ptr->str);
    }

    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);

    char *value_clone = ce_memory_a0->str_dup(value, a);

    ce_array_push(writer->changed_prop, property, _G.allocator);

    value_ptr->str = value_clone;
}

static void set_uint64(ce_cdb_obj_o *_writer,
                       uint64_t property,
                       uint64_t value) {
    struct object_t *writer = _get_object_from_obj_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property, CDB_TYPE_UINT64,
                                   &value, sizeof(uint64_t), _G.allocator);
    }
    ce_array_push(writer->changed_prop, property, _G.allocator);


    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);
    value_ptr->uint64 = value;
}

static void set_ptr(ce_cdb_obj_o *_writer,
                    uint64_t property,
                    const void *value) {
    struct object_t *writer = _get_object_from_obj_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property, CDB_TYPE_PTR, &value,
                                   sizeof(void *),
                                   _G.allocator);
    }
    ce_array_push(writer->changed_prop, property, _G.allocator);

    memcpy((writer->values + writer->offset[idx]), &value, sizeof(void *));
}

static void set_ref(ce_cdb_obj_o *_writer,
                    uint64_t property,
                    uint64_t ref) {

    struct object_t *writer = _get_object_from_obj_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_REF, &ref,
                                   sizeof(uint64_t),
                                   _G.allocator);
    }

    writer->key = property;

    ce_array_push(writer->changed_prop, property, _G.allocator);

    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);
    value_ptr->ref = ref;
}

uint64_t parent(uint64_t object) {
    struct object_t *obj = _get_object_from_objid(object);
    return obj->parent;
}

void set_subobjectw(ce_cdb_obj_o *_writer,
                    uint64_t property,
                    ce_cdb_obj_o *_subwriter) {
    struct object_t *writer = _get_object_from_obj_o(_writer);
    struct object_t *subwriter = _get_object_from_obj_o(_subwriter);

    subwriter->key = property;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_SUBOBJECT, &subwriter->orig_obj,
                                   sizeof(uint64_t),
                                   _G.allocator);
    }

    ce_array_push(writer->changed_prop, property, _G.allocator);

    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);
    value_ptr->subobj = subwriter->orig_obj;
    subwriter->parent = writer->orig_obj;
}

void set_subobject(ce_cdb_obj_o *_writer,
                   uint64_t property,
                   uint64_t subobject) {
    struct object_t *writer = _get_object_from_obj_o(_writer);

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_SUBOBJECT, &subobject,
                                   sizeof(uint64_t),
                                   _G.allocator);
    }

    ce_array_push(writer->changed_prop, property, _G.allocator);

    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);
    value_ptr->subobj = subobject;

    if (subobject) {
        struct object_t *subobj = _get_object_from_objid(subobject);
        subobj->parent = writer->orig_obj;
        subobj->key = property;
    }
}

void set_blob(ce_cdb_obj_o *_writer,
              uint64_t property,
              void *blob_data,
              uint64_t blob_size) {
    struct ce_alloc *a = _G.allocator;

    struct object_t *writer = _get_object_from_obj_o(_writer);

    void *new_blob = CE_ALLOC(a, char, blob_size);

    memcpy(new_blob, blob_data, blob_size);

    struct blob_t blob = {
            .size = blob_size,
            .data = new_blob,
    };


    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_BLOB,
                                   &blob, sizeof(struct blob_t),
                                   _G.allocator);
    } else {
        union type_u *value_ptr = (union type_u *) (writer->values +
                                                    writer->offset[idx]);
        CE_FREE(a, value_ptr->blob.data);
    }

    ce_array_push(writer->changed_prop, property, _G.allocator);


    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);

    value_ptr->blob = blob;
}

void set_prefab(uint64_t _obj,
                uint64_t _prefab) {
    struct object_t *obj = _get_object_from_objid(_obj);
    struct object_t *prefab = _get_object_from_objid(_prefab);

    obj->prefab = _prefab;
    ce_array_push(prefab->instances,
                  _obj,
                  _G.allocator);
}

static bool prop_exist(uint64_t _object,
                       uint64_t key);

void remove_property(ce_cdb_obj_o *_writer,
                     uint64_t property) {
    struct object_t *writer = _get_object_from_obj_o(_writer);

    if (!prop_exist(writer->orig_obj, property)) {
        return;
    }

    uint64_t idx = _find_prop_index(writer, property);

    if (idx) {
        uint64_t last_idx = --writer->properties_count;
        ce_hash_add(&writer->prop_map, writer->keys[last_idx], idx,
                    _G.allocator);

        writer->keys[idx] = writer->keys[last_idx];
        writer->property_type[idx] = writer->property_type[last_idx];
        writer->offset[idx] = writer->offset[last_idx];

        ce_array_pop_back(writer->keys);
        ce_array_pop_back(writer->property_type);
        ce_array_pop_back(writer->offset);
    }

    ce_hash_add(&writer->removed_prop_map, property, 1, _G.allocator);
    ce_hash_remove(&writer->prop_map, property);

    ce_array_push(writer->removed_prop, property, _G.allocator);
}

static bool prop_exist(uint64_t _object,
                       uint64_t key) {
    struct object_t *obj = _get_object_from_objid(_object);

    if (ce_hash_contain(&obj->removed_prop_map, key)) {
        return false;
    }

    uint32_t idx = _find_prop_index(obj, key);

    if (idx) {
        return true;
    }

    if (obj->prefab) {
        return prop_exist(obj->prefab, key);
    }

    return false;
}

static bool prop_exist2(uint64_t _object,
                        uint64_t key) {
    struct object_t *obj = _get_object_from_objid(_object);

    if (ce_hash_contain(&obj->removed_prop_map, key)) {
        return false;
    }

    uint32_t idx = _find_prop_index(obj, key);

    if (idx) {
        return true;
    }

    return false;
}

static enum ce_cdb_type prop_type(uint64_t _object,
                                  uint64_t key) {
    struct object_t *obj = _get_object_from_objid(_object);
    uint64_t idx = _find_prop_index(obj, key);

    if (idx) {
        return (enum ce_cdb_type) obj->property_type[idx];
    }

    if (obj->prefab) {
        return prop_type(obj->prefab, key);
    }

    return CDB_TYPE_NONE;
}

static float read_float(uint64_t _obj,
                        uint64_t property,
                        float defaultt) {
    struct object_t *obj = _get_object_from_objid(_obj);

    if (ce_hash_contain(&obj->removed_prop_map, property)) {
        return defaultt;
    }

    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(float *) (obj->values + obj->offset[idx]);
    }

    if (obj->prefab) {
        return read_float(obj->prefab, property, defaultt);
    }

    return defaultt;
}

static bool read_bool(uint64_t _obj,
                      uint64_t property,
                      bool defaultt) {
    struct object_t *obj = _get_object_from_objid(_obj);

    if (ce_hash_contain(&obj->removed_prop_map, property)) {
        return defaultt;
    }

    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(bool *) (obj->values + obj->offset[idx]);
    }

    if (obj->prefab) {
        return read_bool(obj->prefab, property, defaultt);
    }

    return defaultt;
}

static void read_vec3(uint64_t _obj,
                      uint64_t property,
                      float *value) {
    struct object_t *obj = _get_object_from_objid(_obj);

    if (ce_hash_contain(&obj->removed_prop_map, property)) {
        return;
    }

    uint64_t idx = _find_prop_index(obj, property);


    if (idx) {
        const float *f = (const float *) (obj->values + obj->offset[idx]);
        memcpy(value, f, sizeof(float) * 3);

        return;
    }

    if (obj->prefab) {
        read_vec3(obj->prefab, property, value);
    }
}

static void read_vec4(uint64_t _obj,
                      uint64_t property,
                      float *value) {
    struct object_t *obj = _get_object_from_objid(_obj);

    if (ce_hash_contain(&obj->removed_prop_map, property)) {
        return;
    }

    uint64_t idx = _find_prop_index(obj, property);


    if (idx) {
        const float *f = (const float *) (obj->values + obj->offset[idx]);
        memcpy(value, f, sizeof(float) * 4);

        return;
    }

    if (obj->prefab) {
        read_vec4(obj->prefab, property, value);
    }
}

static void read_mat4(uint64_t _obj,
                      uint64_t property,
                      float *value) {
    struct object_t *obj = _get_object_from_objid(_obj);

    if (ce_hash_contain(&obj->removed_prop_map, property)) {
        return;
    }

    uint64_t idx = _find_prop_index(obj, property);


    if (idx) {
        const float *f = (const float *) (obj->values + obj->offset[idx]);
        memcpy(value, f, sizeof(float) * 16);

        return;
    }

    if (obj->prefab) {
        read_mat4(obj->prefab, property, value);
    }

}

static const char *read_string(uint64_t _obj,
                               uint64_t property,
                               const char *defaultt) {
    struct object_t *obj = _get_object_from_objid(_obj);

    if (ce_hash_contain(&obj->removed_prop_map, property)) {
        return defaultt;
    }

    uint64_t idx = _find_prop_index(obj, property);
    if (idx) {
        return *(const char **) (obj->values + obj->offset[idx]);
    }

    if (obj->prefab) {
        return read_string(obj->prefab, property, defaultt);
    }

    return defaultt;
}


static uint64_t read_uint64(uint64_t _obj,
                            uint64_t property,
                            uint64_t defaultt) {
    struct object_t *obj = _get_object_from_objid(_obj);

    if (ce_hash_contain(&obj->removed_prop_map, property)) {
        return defaultt;
    }

    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(uint64_t *) (obj->values + obj->offset[idx]);
    }

    if (obj->prefab) {
        return read_uint64(obj->prefab, property, defaultt);
    }

    return defaultt;
}

static void *read_ptr(uint64_t _obj,
                      uint64_t property,
                      void *defaultt) {
    struct object_t *obj = _get_object_from_objid(_obj);

    if (ce_hash_contain(&obj->removed_prop_map, property)) {
        return defaultt;
    }

    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(void **) (obj->values + obj->offset[idx]);
    }

    if (obj->prefab) {
        return read_ptr(obj->prefab, property, defaultt);
    }

    return defaultt;
}

static uint64_t read_ref(uint64_t _obj,
                         uint64_t property,
                         uint64_t defaultt) {
    struct object_t *obj = _get_object_from_objid(_obj);

    if (ce_hash_contain(&obj->removed_prop_map, property)) {
        return defaultt;
    }

    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(uint64_t *) (obj->values + obj->offset[idx]);
    }

    if (obj->prefab) {
        return read_ref(obj->prefab, property, defaultt);
    }

    return defaultt;
}

static uint64_t read_subobject(uint64_t _obj,
                               uint64_t property,
                               uint64_t defaultt) {
    struct object_t *obj = _get_object_from_objid(_obj);

    if (ce_hash_contain(&obj->removed_prop_map, property)) {
        return defaultt;
    }

    uint64_t idx = _find_prop_index(obj, property);

    uint64_t subobj = 0;

    if (idx) {
        subobj = *(uint64_t *) (obj->values + obj->offset[idx]);
    } else if (obj->prefab) {
        subobj = read_subobject(obj->prefab, property, defaultt);
    }

    if (subobj) {
//        CE_ASSERT(LOG_WHERE, parent(subobj) != 0);
        return subobj;
    }

    return defaultt;
}

void *read_blob(uint64_t _obj,
                uint64_t property,
                uint64_t *size,
                void *defaultt) {

    struct object_t *obj = _get_object_from_objid(_obj);

    if (ce_hash_contain(&obj->removed_prop_map, property)) {
        return defaultt;
    }

    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        void *data = (obj->values + obj->offset[idx]);

        struct blob_t *blob = data;

        if (size) {
            *size = blob->size;
        }

        return blob->data;
    }

    if (obj->prefab) {
        return read_blob(obj->prefab, property, size, defaultt);
    }

    return defaultt;
}

static uint64_t prop_count(uint64_t _obj);

static void prop_keys(uint64_t _obj,
                      uint64_t *keys) {
    struct object_t *obj = _get_object_from_objid(_obj);

    if (!obj) {
        return;
    }

    uint32_t prop_n = (obj->properties_count - 1);
    memcpy(keys, obj->keys + 1,
           sizeof(uint64_t) * prop_n);

    if (obj->prefab) {
        uint32_t prefab_prop_count = prop_count(obj->prefab);

        uint64_t prefab_keys[prefab_prop_count];
        prop_keys(obj->prefab, prefab_keys);

        for (int i = 0; i < prefab_prop_count; ++i) {
            if (ce_hash_contain(&obj->prop_map, prefab_keys[i])) {
                continue;
            }

            if (ce_hash_contain(&obj->removed_prop_map, prefab_keys[i])) {
                continue;
            }

            keys[prop_n++] = prefab_keys[i];
        }
    }
}

static uint64_t prop_count(uint64_t _obj) {
    struct object_t *obj = _get_object_from_objid(_obj);

    if (!obj) {
        return 0;
    }

    uint64_t count = obj->properties_count - 1;

    if (obj->prefab) {
        uint32_t prefab_prop_count = prop_count(obj->prefab);

        uint64_t keys[prefab_prop_count];
        prop_keys(obj->prefab, keys);

        for (int i = 0; i < prefab_prop_count; ++i) {
            if (ce_hash_contain(&obj->prop_map, keys[i])) {
                continue;
            }

            if (ce_hash_contain(&obj->removed_prop_map, keys[i])) {
                continue;
            }

            count += 1;
        }

    }

    return count;
}

void register_notify(uint64_t _obj,
                     ce_cdb_notify notify,
                     void *data) {
    struct object_t *obj = _get_object_from_objid(_obj);

    struct notify_pair pair = {
            .notify = notify,
            .data = data
    };

    ce_array_push(obj->notify, pair, _G.allocator);
}


static void register_remove_notify(uint64_t _obj,
                                   ce_cdb_notify notify,
                                   void *data) {
    struct object_t *obj = _get_object_from_objid(_obj);

    struct notify_pair pair = {
            .notify = notify,
            .data = data
    };

    ce_array_push(obj->remove_notify, pair, _G.allocator);
}

static struct ce_cdb_t global_db() {
    return _G.global_db;
}

static uint64_t type(uint64_t _obj) {
    struct object_t *obj = _get_object_from_objid(_obj);

    return obj->type;
}

void set_type(uint64_t _obj,
              uint64_t type) {
    struct object_t *obj = _get_object_from_objid(_obj);
    obj->type = type;
}


uint64_t key(uint64_t _obj) {
    struct object_t *obj = _get_object_from_objid(_obj);
    return obj->key;
}

void move(uint64_t _from_obj,
          uint64_t _to) {
    struct object_t *from = _get_object_from_objid(_from_obj);
    uint64_t *obj_addr = (uint64_t *) _to;
    *obj_addr = from->idx;

//    ce_cdb_a0->destroy_object(_from_obj);
}


static void dump_str(char **buffer,
                     uint64_t from,
                     uint32_t level) {
//    const char *prefab = ce_cdb_a0->read_str(from, PREFAB_NAME_PROP, NULL);

    const uint32_t prop_count = ce_cdb_a0->prop_count(from);
    uint64_t keys[prop_count];
    ce_cdb_a0->prop_keys(from, keys);

    for (int i = 0; i < prop_count; ++i) {
        uint64_t key = keys[i];

        const char *k = ce_id_a0->str_from_id64(key);

        enum ce_cdb_type type = ce_cdb_a0->prop_type(from, key);

        for (int j = 0; j < level; ++j) {
            ce_buffer_printf(buffer, _G.allocator, "  ");
        }

        bool from_parent = !ce_cdb_a0->prop_exist_norecursive(from, key);

        ce_buffer_printf(buffer, _G.allocator, "%s:", k);

        switch (type) {
            case CDB_TYPE_SUBOBJECT: {
                uint64_t s = ce_cdb_a0->read_subobject(from, key, 0);
                ce_buffer_printf(buffer, _G.allocator, "\n");
                dump_str(buffer, s, level + 1);
            }
                break;

            case CDB_TYPE_FLOAT: {
                float f = ce_cdb_a0->read_float(from, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " %f", f);
            }
                break;

            case CDB_TYPE_STR: {
                const char *s = ce_cdb_a0->read_str(from, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " %s", s);
            }
                break;
            case CDB_TYPE_VEC3: {
                float v[3] = {};
                ce_cdb_a0->read_vec3(from, key, v);
                ce_buffer_printf(buffer, _G.allocator, " [%f, %f, %f]", v[0],
                                 v[1], v[2]);
            }
                break;
            case CDB_TYPE_VEC4: {
                float v[4] = {};
                ce_cdb_a0->read_vec3(from, key, v);
                ce_buffer_printf(buffer, _G.allocator, " [%f, %f, %f, %f]",
                                 v[0], v[1], v[2], v[3]);
            }
                break;

            case CDB_TYPE_BOOL: {
                bool b = ce_cdb_a0->read_bool(from, key, 0);
                if (b) {
                    ce_buffer_printf(buffer, _G.allocator, "true");
                } else {
                    ce_buffer_printf(buffer, _G.allocator, "false");
                }
            }
                break;

            case CDB_TYPE_NONE: {
                ce_buffer_printf(buffer, _G.allocator, "none");
            }
                break;

            case CDB_TYPE_UINT64: {
                uint64_t i = ce_cdb_a0->read_uint64(from, key, 0);
                ce_buffer_printf(buffer, _G.allocator, "%llu", i);
            }
                break;

            case CDB_TYPE_PTR:
            case CDB_TYPE_REF:
            case CDB_TYPE_MAT4:
            case CDB_TYPE_BLOB:
            default:
                break;
        }

        ce_buffer_printf(buffer, _G.allocator, " %s\n",
                         from_parent ? " # from_parents" : "");
    }
}


static struct ce_cdb_a0 cdb_api = {
        .register_notify = register_notify,
        .register_remove_notify = register_remove_notify,
//        .create_db = create_db,

        . db  = global_db,

        .type = type,
        .set_type = set_type,
        .key= key,
        .move = move,
        .create_object = create_object,
        .create_from = create_from,
        .destroy_object = destroy_object,
        .destroy_db = destroy_db,

        .gc = gc,

        .dump_str = dump_str,
        .dump = dump,
        .load = load,

        .prop_exist = prop_exist,
        .prop_exist_norecursive = prop_exist2,
        .prop_type = prop_type,
        .prop_keys = prop_keys,
        .prop_count = prop_count,
        .parent = parent,

        .read_float = read_float,
        .read_bool = read_bool,
        .read_vec3 = read_vec3,
        .read_vec4 = read_vec4,
        .read_mat4 = read_mat4,
        .read_str = read_string,
        .read_uint64 = read_uint64,
        .read_ptr = read_ptr,
        .read_ref = read_ref,
        .read_subobject = read_subobject,
        .read_blob = read_blob,

        .write_begin = write_begin,
        .write_commit = write_commit,
        .write_try_commit = write_try_commit,

        .set_float = set_float,
        .set_bool = set_bool,
        .set_vec3 = set_vec3,
        .set_vec4 = set_vec4,
        .set_mat4 = set_mat4,
        .set_str = set_string,
        .set_uint64 = set_uint64,
        .set_ptr = set_ptr,
        .set_ref = set_ref,
        .set_subobject = set_subobject,
        .set_subobjectw = set_subobjectw,
        .set_prefab = set_prefab,
        .set_blob = set_blob,
        .remove_property = remove_property,
};

struct ce_cdb_a0 *ce_cdb_a0 = &cdb_api;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    _G.global_db = create_db();

    api->register_api("ce_cdb_a0", &cdb_api);
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