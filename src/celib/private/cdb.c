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
#include <celib/ebus.h>

#define _G coredb_global
#define LOG_WHERE "coredb"

#define MAX_OBJECTS 1000000000ULL
#define MAX_FREE_OBJECE_POOL 10000
#define MAX_FREE_OBJECE_ID_POOL 10000

// TODO: non optimal braindump code
// TODO: remove null element


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
    struct blob_t blob;
};

// TODO: X(
struct object_t {
    // prefab
    uint64_t instance_of;
    uint64_t *instances;

    // hiearchy
    uint64_t key;
    uint64_t parent;

    // writer
    uint64_t orig_obj;
    uint64_t *changed_prop;
    uint64_t *removed_prop;

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
    uint32_t idx;

    // id pool
    struct object_t **objects_mem;
    struct object_t ***free_objects_id;
    struct object_t ***to_free_objects_id;

    atomic_ullong object_used;
    atomic_ullong to_free_objects_id_n;
    atomic_ullong free_objects_id_n;

    // objects
    struct object_t *object_pool;
    struct object_t **free_objects;
    struct object_t **to_free_objects;

    atomic_ullong object_pool_n;
    atomic_ullong free_objects_n;
    atomic_ullong to_free_objects_n;
};

static struct _G {
    struct db_t *dbs;
    uint32_t *free_db;
    uint32_t *to_free_db;

    struct ce_alloc *allocator;
    struct ce_cdb_t global_db;
} _G;


static struct object_t *_get_object_from_id(uint64_t objid) {
    if (!objid) {
        return NULL;
    }

    return *((struct object_t **) objid);
}

static struct object_t *_get_object_from_o(const ce_cdb_obj_o *obj_o) {
    return ((struct object_t*) obj_o);
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

static struct object_t **_new_object_id(struct db_t *db_inst) {
    struct object_t **obj;
    if (db_inst->free_objects_id_n) {
        atomic_fetch_sub(&db_inst->free_objects_id_n, 1);
        obj = db_inst->free_objects_id[db_inst->free_objects_id_n];
    } else {
        uint64_t idx = atomic_fetch_add(&db_inst->object_used, 1);
        obj = db_inst->objects_mem + idx;
    }

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

static void _destroy_object(struct object_t *obj) {
    struct db_t *db_inst = &_G.dbs[obj->db.idx];

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

static struct ce_cdb_t create_db() {
    uint64_t idx = ce_array_size(_G.dbs);

    struct db_t db = (struct db_t) {
            .idx = idx,

            .objects_mem = (struct object_t **) virt_alloc(MAX_OBJECTS *
                                                           sizeof(struct object_t *)),


            .free_objects_id = (struct object_t ***) virt_alloc(MAX_OBJECTS *
                                                                sizeof(struct object_t **)),

            .to_free_objects_id = (struct object_t ***) virt_alloc(MAX_OBJECTS *
                                                                   sizeof(struct object_t **)),

            .object_pool = (struct object_t *) virt_alloc(MAX_OBJECTS *
                                                          sizeof(struct object_t)),

            .free_objects = (struct object_t **) virt_alloc(MAX_OBJECTS *
                                                            sizeof(struct object_t *)),

            .to_free_objects = (struct object_t **) virt_alloc(MAX_OBJECTS *
                                                               sizeof(struct object_t *))
    };

    ce_array_push(_G.dbs, db, _G.allocator);
    return (struct ce_cdb_t) {.idx = idx};
};

static uint64_t create_object(struct ce_cdb_t db,
                              uint64_t type) {
    struct db_t *db_inst = &_G.dbs[db.idx];
    struct object_t *obj = _new_object(db_inst, _G.allocator);

    struct object_t **obj_addr = _new_object_id(db_inst);

    obj->db = db;
    obj->type = type;

    *obj_addr = obj;

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

    struct object_t *obj = _get_object_from_id(_obj);

    struct object_t *inst = _new_object(db_inst, _G.allocator);
    inst->db = db;

    struct object_t **obj_addr = _new_object_id(db_inst);

    *obj_addr = inst;

    inst->instance_of = _obj;
    inst->type = obj->type;

    ce_array_push(obj->instances, (uint64_t) obj_addr, _G.allocator);


    struct object_t *wr = write_begin((uint64_t) obj_addr);


    const ce_cdb_obj_o *reader = ce_cdb_a0->read(_obj);

    for (int i = 1; i < obj->properties_count; ++i) {
        uint64_t key = obj->keys[i];
        switch (obj->property_type[i]) {
            case CDB_TYPE_SUBOBJECT: {
                union type_u *value_ptr = (union type_u *) (obj->values +
                                                            obj->offset[i]);

                uint64_t old_subobj = value_ptr->subobj;
                uint64_t new_subobj = create_from(db, old_subobj);
                set_subobject(wr, obj->keys[i], new_subobj);
            }
                break;

            case CDB_TYPE_UINT64: {
                uint64_t v = ce_cdb_a0->read_uint64(reader, key, 0);
                ce_cdb_a0->set_uint64(wr, key, v);
            }
                break;
            case CDB_TYPE_PTR: {
                void *v = ce_cdb_a0->read_ptr(reader, key, 0);
                ce_cdb_a0->set_ptr(wr, key, v);
            }
                break;
            case CDB_TYPE_REF: {
                uint64_t v = ce_cdb_a0->read_uint64(reader, key, 0);
                ce_cdb_a0->set_ref(wr, key, v);
            }
                break;
            case CDB_TYPE_FLOAT: {
                float v = ce_cdb_a0->read_float(reader, key, 0);
                ce_cdb_a0->set_float(wr, key, v);
            }
                break;
            case CDB_TYPE_BOOL: {
                bool v = ce_cdb_a0->read_bool(reader, key, 0);
                ce_cdb_a0->set_bool(wr, key, v);
            }
                break;
            case CDB_TYPE_STR: {
                const char *v = ce_cdb_a0->read_str(reader, key, NULL);
                ce_cdb_a0->set_str(wr, key, v);
            }
                break;
            case CDB_TYPE_BLOB: {
                uint64_t size = 0;
                void *v = ce_cdb_a0->read_blob(reader, key, &size,
                                               NULL);
                ce_cdb_a0->set_blob(wr, key, v, size);
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

    struct object_t *obj = _get_object_from_id(_obj);
    struct db_t *db_inst = &_G.dbs[obj->db.idx];

    uint64_t idx = atomic_fetch_add(&db_inst->to_free_objects_id_n, 1);

    db_inst->to_free_objects_id[idx] = (struct object_t **) _obj;

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
            struct object_t **objid = db_inst->to_free_objects_id[j];
            struct object_t *obj = _get_object_from_id((uint64_t) objid);

            // remove from instance from parent instance
            if (obj->instance_of) {
                struct object_t *prefab_obj = \
                    _get_object_from_id(obj->instance_of);

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

            uint64_t fidx = atomic_fetch_add(&db_inst->free_objects_id_n, 1);
            db_inst->free_objects_id[fidx] = objid;
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

            ce_array_clean(obj->changed_prop);
            ce_array_clean(obj->removed_prop);

            *obj = (struct object_t) {
                    .instances = obj->instances,
                    .property_type = obj->property_type,
                    .keys = obj->keys,
                    .values = obj->values,
                    .offset = obj->offset,
                    .prop_map = obj->prop_map,
                    .changed_prop = obj->changed_prop,
                    .removed_prop = obj->removed_prop,
            };


            uint64_t fidx = atomic_fetch_add(&db_inst->free_objects_n, 1);
            db_inst->free_objects[fidx] = obj;
        }

        db_inst->to_free_objects_n = 0;
    }

    struct ebus_event_header *it = ce_ebus_a0->events(CDB_EBUS);
    struct ebus_event_header *end_it = ce_ebus_a0->events_end(CDB_EBUS);

    while (it != end_it) {
        struct ce_cdb_prop_ev0 *ev = CE_EBUS_BODY(it);
        ce_array_free(ev->prop, _G.allocator);
//
//        switch(it->type) {
//            case CDB_PROP_REMOVED_EVENT:
//            case CDB_PROP_CHANGED_EVENT:
//                break;
//
//            default:
//                break;
//        }

        it = CE_EBUS_NEXT(it);
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
    struct object_t *obj = _get_object_from_id(_obj);

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

    struct object_t *obj = _get_object_from_id(_obj);
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

                struct object_t *sub_obj = _get_object_from_id(subobj);
                sub_obj->parent = _obj;
                sub_obj->key = obj->keys[i];

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
    struct object_t *obj = _get_object_from_id(_obj);
    struct db_t *db_inst = &_G.dbs[obj->db.idx];

    struct object_t *new_obj = _object_clone(db_inst, obj, _G.allocator);

    new_obj->orig_obj = _obj;

    return new_obj;
}

static enum ce_cdb_type prop_type(uint64_t _object,
                                  uint64_t key) {
    struct object_t *obj = _get_object_from_id(_object);
    uint64_t idx = _find_prop_index(obj, key);

    if (idx) {
        return (enum ce_cdb_type) obj->property_type[idx];
    }

    return CDB_TYPE_NONE;
}

static void _notify(struct object_t *obj,
                    uint64_t notify_obj,
                    uint64_t *changed_prop,
                    uint64_t event);

//static void _dispatch_instances(struct object_t *obj,
//                                struct object_t *writer,
//                                uint64_t *changed_prop,
//                                uint64_t event);

static void write_commit(ce_cdb_obj_o *_writer) {
    struct object_t *writer = _get_object_from_o(_writer);
    struct object_t *orig_obj = _get_object_from_id(writer->orig_obj);

    struct object_t **obj_addr = (struct object_t **) writer->orig_obj;

    _notify(orig_obj, writer->orig_obj,
            writer->removed_prop, CDB_PROP_REMOVED_EVENT);

    _notify(orig_obj, writer->orig_obj,
            writer->changed_prop, CDB_PROP_CHANGED_EVENT);


//    _dispatch_instances(orig_obj, writer,
//                        writer->changed_prop, CDB_PROP_CHANGED_EVENT);

    *obj_addr = writer;

    ce_array_clean(writer->removed_prop);
    ce_array_clean(writer->changed_prop);

    _destroy_object(orig_obj);
}

static bool write_try_commit(ce_cdb_obj_o *_writer) {
    struct object_t *writer = _get_object_from_o(_writer);
    struct object_t *orig_obj = _get_object_from_id(writer->orig_obj);


    struct object_t **obj_addr = (struct object_t **) writer->orig_obj;

    bool ok = atomic_compare_exchange_weak((atomic_ullong *) obj_addr,
                                           ((uint64_t *) &orig_obj),
                                           ((uint64_t) writer));

    if (!ok) {
        goto end;
    }

    _notify(orig_obj, writer->orig_obj,
            writer->removed_prop, CDB_PROP_REMOVED_EVENT);

    _notify(writer, writer->orig_obj,
            writer->changed_prop, CDB_PROP_CHANGED_EVENT);

    end:
    _destroy_object(orig_obj);
    return ok;
}


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

    ce_array_push(writer->changed_prop, property, _G.allocator);

    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);
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

    ce_array_push(writer->changed_prop, property, _G.allocator);

    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);
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
    struct object_t *writer = _get_object_from_o(_writer);

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
    struct object_t *writer = _get_object_from_o(_writer);

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

    struct object_t *writer = _get_object_from_o(_writer);

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
    struct object_t *obj = _get_object_from_id(object);
    return obj->parent;
}

uint64_t prefab(uint64_t object) {
    if (!object) {
        return 0;
    }

    struct object_t *obj = _get_object_from_id(object);
    return obj->instance_of;
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

    ce_array_push(writer->changed_prop, property, _G.allocator);

    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);
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

    ce_array_push(writer->changed_prop, property, _G.allocator);

    union type_u *value_ptr = (union type_u *) (writer->values +
                                                writer->offset[idx]);
    value_ptr->subobj = subobject;

    if (subobject) {
        struct object_t *subobj = _get_object_from_id(subobject);
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
    struct object_t *obj = _get_object_from_id(_obj);
    struct object_t *prefab = _get_object_from_id(_prefab);

    obj->instance_of = _prefab;
    ce_array_push(prefab->instances,
                  _obj,
                  _G.allocator);
}

static bool prop_exist(uint64_t _object,
                       uint64_t key);

void remove_property(ce_cdb_obj_o *_writer,
                     uint64_t property) {
    struct object_t *writer = _get_object_from_o(_writer);

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

    ce_hash_remove(&writer->prop_map, property);

    ce_array_push(writer->removed_prop, property, _G.allocator);

//    struct object_t *writer = _get_object_from_obj_o(_writer);
//
//    if (!prop_exist(writer->orig_obj, property)) {
//        return;
//    }
//
//    uint64_t idx = _find_prop_index(writer, property);
//
//    if (idx) {
//        uint64_t last_idx = --writer->properties_count;
//        ce_hash_add(&writer->prop_map, writer->keys[last_idx], idx,
//                    _G.allocator);
//
//        writer->keys[idx] = writer->keys[last_idx];
//        writer->property_type[idx] = writer->property_type[last_idx];
//        writer->offset[idx] = writer->offset[last_idx];
//
//        ce_array_pop_back(writer->keys);
//        ce_array_pop_back(writer->property_type);
//        ce_array_pop_back(writer->offset);
//    }
//
//    ce_hash_add(&writer->removed_prop_map, property, 1, _G.allocator);
//    ce_hash_remove(&writer->prop_map, property);
//
//    ce_array_push(writer->removed_prop, property, _G.allocator);
}

void delete_property(ce_cdb_obj_o *_writer,
                     uint64_t property) {
    struct object_t *writer = _get_object_from_o(_writer);

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

    ce_hash_remove(&writer->prop_map, property);

    ce_array_push(writer->changed_prop, property, _G.allocator);
}

static bool prop_exist(uint64_t _object,
                       uint64_t key) {
    struct object_t *obj = _get_object_from_id(_object);

    uint32_t idx = _find_prop_index(obj, key);

    if (idx) {
        return true;
    }

    return false;
}

const ce_cdb_obj_o *read(uint64_t object) {
    if (!object) {
        return NULL;
    }

    struct object_t *obj = _get_object_from_id(object);
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

        struct blob_t *blob = data;

        if (size) {
            *size = blob->size;
        }

        return blob->data;
    }

    return defaultt;
}

static uint64_t prop_count(uint64_t _obj);

static const uint64_t *prop_keys(uint64_t _obj) {
    struct object_t *obj = _get_object_from_id(_obj);

    if (!obj) {
        return NULL;
    }

    return obj->keys + 1;
}

static uint64_t prop_count(uint64_t _obj) {
    struct object_t *obj = _get_object_from_id(_obj);

    if (!obj) {
        return 0;
    }

    uint64_t count = obj->properties_count - 1;

    return count;
}

static struct ce_cdb_t global_db() {
    return _G.global_db;
}

static uint64_t type(uint64_t _obj) {
    struct object_t *obj = _get_object_from_id(_obj);

    return obj->type;
}

void set_type(uint64_t _obj,
              uint64_t type) {
    if (!_obj) {
        return;
    }

    struct object_t *obj = _get_object_from_id(_obj);
    obj->type = type;
}


uint64_t key(uint64_t _obj) {
    struct object_t *obj = _get_object_from_id(_obj);
    return obj->key;
}

void move(uint64_t _from_obj,
          uint64_t _to) {
    struct object_t *from = _get_object_from_id(_from_obj);

    struct object_t** obj_addr = (struct object_t**)_to;

    *obj_addr = from;

    ce_cdb_a0->destroy_object(_from_obj);
}


static void dump_str(char **buffer,
                     uint64_t from,
                     uint32_t level) {
    const uint32_t prop_count = ce_cdb_a0->prop_count(from);
    const uint64_t *keys = ce_cdb_a0->prop_keys(from);

    for (int i = 0; i < prop_count; ++i) {
        uint64_t key = keys[i];

        const char *k = ce_id_a0->str_from_id64(key);

        enum ce_cdb_type type = ce_cdb_a0->prop_type(from, key);

        for (int j = 0; j < level; ++j) {
            ce_buffer_printf(buffer, _G.allocator, "  ");
        }

        ce_buffer_printf(buffer, _G.allocator, "%s:", k);

        const ce_cdb_obj_o *reader = ce_cdb_a0->read(from);
        switch (type) {
            case CDB_TYPE_SUBOBJECT: {
                uint64_t s = ce_cdb_a0->read_subobject(reader, key, 0);
                ce_buffer_printf(buffer, _G.allocator, "\n");
                dump_str(buffer, s, level + 1);
            }
                break;

            case CDB_TYPE_FLOAT: {
                float f = ce_cdb_a0->read_float(reader, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " %f", f);
            }
                break;

            case CDB_TYPE_STR: {
                const char *s = ce_cdb_a0->read_str(reader, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " %s", s);
            }
                break;

            case CDB_TYPE_BOOL: {
                bool b = ce_cdb_a0->read_bool(reader, key, 0);
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
                uint64_t i = ce_cdb_a0->read_uint64(reader, key, 0);
                ce_buffer_printf(buffer, _G.allocator, "%llu", i);
            }
                break;

            case CDB_TYPE_PTR:
            case CDB_TYPE_REF:
            case CDB_TYPE_BLOB:
            default:
                break;
        }

        ce_buffer_printf(buffer, _G.allocator, " %s\n", "");
    }

}

static uint64_t find_root(uint64_t obj) {
    while (parent(obj)) {
        obj = parent(obj);
    }

    return obj;
}

//static void _dispatch_instances(struct object_t *orig_obj,
//                                struct object_t *writer,
//                                uint64_t *changed_prop,
//                                uint64_t event) {
//    const int changed_prop_n = ce_array_size(changed_prop);
//
//    if (!changed_prop_n) {
//        return;
//    }
//
//
//    const int instances_n = ce_array_size(orig_obj->instances);
//    for (int i = 0; i < instances_n; ++i) {
//        uint64_t inst_obj = orig_obj->instances[i];
//
//        if (event == CDB_PROP_CHANGED_EVENT) {
//            ce_cdb_obj_o *w = write_begin(inst_obj);
//            for (int j = 0; j < changed_prop_n; ++j) {
//                uint64_t prop = changed_prop[j];
//                enum ce_cdb_type t = prop_type(notify_obj, prop);
//                switch (t) {
//                    case CDB_TYPE_NONE:
//                        break;
//                    case CDB_TYPE_UINT64:
//                        break;
//                    case CDB_TYPE_PTR:
//                        break;
//                    case CDB_TYPE_REF:
//                        break;
//                    case CDB_TYPE_FLOAT: {
//                        float v = read_float(notify_obj, prop, 0.0f);
//                        float inst_v = read_float(inst_obj, prop, 0.0f);
//
//                        ce_cdb_a0->set_float(w, prop, v);
//                    }
//                        break;
//                    case CDB_TYPE_BOOL:
//                        break;
//                    case CDB_TYPE_STR:
//                        break;
//                    case CDB_TYPE_SUBOBJECT:
//                        break;
//                    case CDB_TYPE_BLOB:
//                        break;
//                    default:
//                        break;
//                }
//            }
//            write_commit(w);
//        }
//    }
//}

static void _notify(struct object_t *obj,
                    uint64_t notify_obj,
                    uint64_t *changed_prop,
                    uint64_t event) {
    const int changed_prop_n = ce_array_size(changed_prop);

    if (!changed_prop_n) {
        return;
    }


    struct ce_cdb_prop_ev0 ev = {
            .obj = notify_obj,
            .prop=  ce_array_clone(changed_prop, sizeof(uint64_t),
                                   _G.allocator),
            .prop_count = changed_prop_n,
    };

    ce_ebus_a0->broadcast(CDB_EBUS, event,
                          &ev, sizeof(struct ce_cdb_prop_ev0));


}

static struct ce_cdb_a0 cdb_api = {
//        .create_db = create_db,
        .db  = global_db,
        .obj_type = type,
        .set_type = set_type,
        .obj_key= key,
        .move = move,
        .create_object = create_object,
        .create_from = create_from,
        .destroy_object = destroy_object,
        .destroy_db = destroy_db,

        .gc = gc,

        .dump_str = dump_str,
        .dump = dump,
        .load = load,

        .find_root = find_root,
        .prop_exist = prop_exist,
        .prop_type = prop_type,
        .prop_keys = prop_keys,
        .prop_count = prop_count,

        .parent = parent,

        .read = read,
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
        .write_try_commit = write_try_commit,

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
        .delete_property = delete_property,
};

struct ce_cdb_a0 *ce_cdb_a0 = &cdb_api;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    _G.global_db = create_db();

    api->register_api("ce_cdb_a0", &cdb_api);

    ce_ebus_a0->create_ebus(CDB_EBUS);
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