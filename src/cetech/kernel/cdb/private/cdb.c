#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <cetech/kernel/macros.h>
#include <cetech/kernel/api/api_system.h>
#include <cetech/kernel/memory/memory.h>
#include <cetech/kernel/log/log.h>
#include <cetech/kernel/cdb/cdb.h>
#include <cetech/kernel/module/module.h>
#include <cetech/kernel/memory/allocator.h>
#include <cetech/kernel/containers/hash.h>
#include <cetech/kernel/hashlib/hashlib.h>


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_log_a0);

#define _G coredb_global
#define LOG_WHERE "coredb"

#define MAX_OBJECTS 1000000000ULL

// TODO: non optimal braindump code

struct object_t {
    ct_cdb_notify *notify;
    // prefab
    struct ct_cdb_obj_t *owner;
    struct ct_cdb_obj_t **instances;

    // hiearchy
    struct ct_cdb_obj_t *parent;
    struct ct_cdb_obj_t **children;

    // writer
    struct ct_cdb_obj_t *obj;
    uint64_t *changed_prop;

    // object
    uint64_t type;
    uint32_t idx;
    struct ct_cdb_t db;
    struct ct_hash_t prop_map;

    uint64_t properties_count;

    uint64_t *keys;
    uint8_t *ptype;
    uint64_t *offset;
    uint8_t *values;
};

struct db_t {
    uint32_t idx;

    // id pool
    struct object_t **objects_mem;
    uint64_t object_used;
    uint32_t *free_objects_id;
    uint32_t *to_free_objects_id;

    // Hiearchy
    uint32_t* first_child;
    uint32_t* next_sibling;
    uint32_t* parent;

    // objects
    struct object_t *object_pool;
    uint32_t *free_objects;
    uint32_t *to_free_objects;
};

static struct _G {
    struct db_t *dbs;
    uint32_t *free_db;
    uint32_t *to_free_db;

    struct ct_alloc *allocator;
    struct ct_cdb_t global_db;
} _G;


static uint64_t _object_new_property(struct object_t *obj,
                                     uint64_t key,
                                     enum ct_cdb_type type,
                                     const void *value,
                                     size_t size,
                                     const struct ct_alloc *alloc) {

    const uint64_t prop_count = obj->properties_count;
    const uint64_t values_size = ct_array_size(obj->values);

    ct_array_push(obj->keys, key, alloc);
    ct_array_push(obj->ptype, type, alloc);
    ct_array_push(obj->offset, values_size, alloc);

    if (size)
        ct_array_resize(obj->values, values_size + size, alloc);

    ct_hash_add(&obj->prop_map, key, prop_count, alloc);

    obj->properties_count = obj->properties_count + 1;
    return prop_count;
}

struct object_t *_new_object(struct db_t *db,
                             const struct ct_alloc *a) {
    uint32_t idx;

    if (ct_array_size(db->free_objects)) {
        idx = ct_array_back(db->free_objects);
        ct_array_pop_back(db->free_objects);
    } else {
        idx = ct_array_size(db->object_pool);
        ct_array_push(db->object_pool, (struct object_t) {0}, a);
    }

    struct object_t *obj = &db->object_pool[idx];
    obj->idx = idx;
    obj->db.idx = db->idx;

    _object_new_property(obj, 0, CDB_TYPE_NONE, NULL, 0, a);
    return obj;
}

static struct object_t *_object_clone(struct db_t *db,
                                      struct object_t *obj,
                                      const struct ct_alloc *alloc) {
    const uint64_t properties_count = obj->properties_count;
    const uint64_t values_size = ct_array_size(obj->values);

    struct object_t *new_obj = _new_object(db, alloc);

    new_obj->owner = obj->owner;
    new_obj->parent = obj->parent;

    new_obj->type = obj->type;
    new_obj->properties_count = properties_count;

    ct_array_push_n(new_obj->keys, obj->keys + 1,
                    ct_array_size(obj->keys) - 1, alloc);

    ct_array_push_n(new_obj->ptype, obj->ptype + 1,
                    ct_array_size(obj->ptype) - 1, alloc);

    ct_array_push_n(new_obj->offset, obj->offset + 1,
                    ct_array_size(obj->offset) - 1, alloc);

    if (values_size)
        ct_array_push_n(new_obj->values, obj->values, values_size, alloc);


    ct_hash_clone(&obj->prop_map, &new_obj->prop_map, alloc);

    uint32_t n = ct_array_size(obj->instances);
    if (n) {
        ct_array_push_n(new_obj->instances, obj->instances, n, alloc);
    }

    n = ct_array_size(obj->children);
    if (n) {
        ct_array_push_n(new_obj->children, obj->children, n, alloc);
    }

    n = ct_array_size(obj->notify);
    if (n) {
        ct_array_push_n(new_obj->notify, obj->notify, n, alloc);
    }


    return new_obj;
}

static uint64_t _find_prop_index(const struct object_t *obj,
                                 uint64_t key) {
    return ct_hash_lookup(&obj->prop_map, key, 0);
}

static void _destroy_object(struct object_t *obj) {
    struct db_t *db_inst = &_G.dbs[obj->db.idx];
    ct_array_push(db_inst->to_free_objects, obj->idx, _G.allocator);
}

static struct ct_cdb_t create_db() {
    uint64_t idx = ct_array_size(_G.dbs);

    struct db_t db = (struct db_t) {
            .objects_mem = (struct object_t **) mmap(
                    NULL,
                    MAX_OBJECTS * sizeof(struct object_t *),
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1, 0),
            .idx = idx,
    };

    ct_array_push(_G.dbs, db, _G.allocator);

    return (struct ct_cdb_t) {.idx = idx};
};

static struct ct_cdb_obj_t *create_object(struct ct_cdb_t db,
                                          uint64_t type) {
    struct db_t *db_inst = &_G.dbs[db.idx];
    struct object_t *obj = _new_object(db_inst, _G.allocator);

    uint64_t idx = 0;
    if (ct_array_size(db_inst->free_objects_id)) {
        idx = ct_array_back(db_inst->free_objects_id);
        ct_array_pop_back(db_inst->free_objects_id);
    } else {
        idx = db_inst->object_used++;
    }

    struct object_t **obj_addr = db_inst->objects_mem + idx;

    *obj_addr = obj;
    obj->db = db;
    obj->type = type;

    return (struct ct_cdb_obj_t *) obj_addr;
}

static struct ct_cdb_obj_t *create_from(struct ct_cdb_t db,
                                        struct ct_cdb_obj_t *_obj) {
    struct db_t *db_inst = &_G.dbs[db.idx];

    struct object_t *obj = *(struct object_t **) _obj;

    struct object_t *inst = _new_object(db_inst, _G.allocator);
    inst->db = db;

    struct object_t **obj_addr = db_inst->objects_mem + db_inst->object_used++;

    *obj_addr = inst;

    inst->owner = _obj;
    ct_array_push(obj->instances,
                  (struct ct_cdb_obj_t *) obj_addr,
                  _G.allocator);

    return (struct ct_cdb_obj_t *) obj_addr;
}

static void destroy_db(struct ct_cdb_t db) {
    ct_array_push(_G.to_free_db, db.idx, _G.allocator);
}

static void destroy_object(struct ct_cdb_obj_t *_obj) {
    struct object_t *obj = *(struct object_t **) _obj;
    struct db_t *db_inst = &_G.dbs[obj->db.idx];

    ct_array_push(db_inst->to_free_objects_id, (struct object_t **)_obj - db_inst->objects_mem, _G.allocator);
}


static void gc() {
    const uint32_t db_n = ct_array_size(_G.dbs);
    for (int i = 0; i < db_n; ++i) {
        struct db_t *db_inst = &_G.dbs[i];

        const uint32_t to_free_objects_id_n = ct_array_size(db_inst->to_free_objects_id);
        for (int j = 0; j < to_free_objects_id_n; ++j) {
            const uint32_t idx = db_inst->to_free_objects_id[j];


            struct object_t *obj = * (db_inst->objects_mem+idx);
            _destroy_object(obj);

            ct_array_push(db_inst->free_objects_id, idx, _G.allocator);
        }

        if (to_free_objects_id_n) {
            ct_array_resize(db_inst->to_free_objects_id, 0, _G.allocator);
        }

        const uint32_t to_free_objects_n = ct_array_size(db_inst->to_free_objects);
        for (int j = 0; j < to_free_objects_n; ++j) {
            const uint32_t idx = db_inst->to_free_objects[j];

            struct object_t *obj = &db_inst->object_pool[idx];

            ct_array_clean(obj->children);
            ct_array_clean(obj->instances);

            ct_array_clean(obj->ptype);
            ct_array_clean(obj->keys);
            ct_array_clean(obj->values);
            ct_array_clean(obj->offset);

            ct_hash_clean(&obj->prop_map);

            ct_array_clean(obj->changed_prop);
            ct_array_clean(obj->notify);

            *obj = (struct object_t) {
                    .children = obj->children,
                    .instances = obj->instances,
                    .ptype = obj->ptype,
                    .keys = obj->keys,
                    .values = obj->values,
                    .offset = obj->offset,
                    .prop_map = obj->prop_map,
            };

            ct_array_push(db_inst->free_objects, idx, _G.allocator);
        }

        if (to_free_objects_n) {
            ct_array_resize(db_inst->to_free_objects, 0, _G.allocator);
        }
    }
}

static void add_child(struct ct_cdb_obj_t *_parent,
                      struct ct_cdb_obj_t *_child) {
    struct object_t *parent = *(struct object_t **) _parent;
    struct object_t *child = *(struct object_t **) _child;

    child->parent = _parent;
    ct_array_push(parent->children, _child, _G.allocator);
}

struct ct_cdb_obj_t **children(struct ct_cdb_obj_t *obj) {
    struct object_t *_obj = *(struct object_t **) obj;
    return _obj->children;
}

struct cdb_binobj_header {
    uint64_t version;
    uint64_t properties_count;
    uint64_t values_size;
    uint64_t string_buffer_size;
};

static void dump(struct ct_cdb_obj_t *_obj,
                 char **output,
                 struct ct_alloc *allocator) {
    struct object_t *obj = *(struct object_t **) _obj;

    uint64_t *keys = obj->keys;
    uint8_t *type = obj->ptype;
    uint64_t *offset = obj->offset;
    uint8_t *values = obj->values;

    char *str_buffer = NULL;
    for (int i = 1; i < obj->properties_count; ++i) {
        if (obj->ptype[i] != CDB_TYPE_STR) {
            continue;
        }

        uint64_t stroffset = ct_array_size(str_buffer);
        char *str = *(char **) (values + offset[i]);
        ct_array_push_n(str_buffer, str, strlen(str) + 1, allocator);

        uint64_t *strptr = (uint64_t *) (values + offset[i]);
        *strptr = stroffset;
    }

    struct cdb_binobj_header header = {
            .properties_count = obj->properties_count - 1,
            .values_size = ct_array_size(values),
            .string_buffer_size = ct_array_size(str_buffer)
    };

    ct_array_push_n(*output, (char *) &header,
                    sizeof(struct cdb_binobj_header),
                    allocator);

    ct_array_push_n(*output, (char *) (keys + 1),
                    sizeof(uint64_t) * (ct_array_size(keys) - 1),
                    allocator);

    ct_array_push_n(*output, (char *) (type + 1),
                    sizeof(uint8_t) * (ct_array_size(type) - 1),
                    allocator);

    ct_array_push_n(*output, (char *) (offset + 1),
                    sizeof(uint64_t) * (ct_array_size(offset) - 1),
                    allocator);

    ct_array_push_n(*output, (char *) values,
                    ct_array_size(values),
                    allocator);

    ct_array_push_n(*output, (char *) str_buffer,
                    header.string_buffer_size,
                    allocator);


}

static void load(struct ct_cdb_obj_t *_obj,
                 const char *input,
                 struct ct_alloc *allocator) {
    struct object_t *obj = *(struct object_t **) _obj;

    const struct cdb_binobj_header *header;
    header = (const struct cdb_binobj_header *) input;

    uint64_t *keys = (uint64_t *) (header + 1);
    uint8_t *ptype = (uint8_t *) (keys + header->properties_count);
    uint64_t *offset = (uint64_t *) (ptype + header->properties_count);
    uint8_t *values = (uint8_t *) (offset + header->properties_count);
    const char *strbuffer = (char *) (values + header->values_size);

    ct_array_push_n(obj->keys, keys,
                    header->properties_count,
                    allocator);

    ct_array_push_n(obj->ptype, ptype,
                    header->properties_count,
                    allocator);

    ct_array_push_n(obj->offset, offset,
                    header->properties_count,
                    allocator);

    ct_array_push_n(obj->values, values,
                    header->values_size,
                    allocator);

    obj->properties_count += header->properties_count;

    for (int i = 1; i < obj->properties_count; ++i) {
        ct_hash_add(&obj->prop_map, obj->keys[i], i, allocator);
    }


    for (int i = 1; i < obj->properties_count; ++i) {
        if (obj->ptype[i] != CDB_TYPE_STR) {
            continue;
        }

        uint64_t str_offset = *(uint64_t *) (obj->values + obj->offset[i]);

        char *dup_str = ct_memory_a0.str_dup(strbuffer + str_offset, allocator);

        *(char **) (obj->values + obj->offset[i]) = dup_str;
    }
}

static ct_cdb_obj_o *write_begin(struct ct_cdb_obj_t *_obj) {
    struct object_t *obj = *(struct object_t **) _obj;
    struct db_t *db_inst = &_G.dbs[obj->db.idx];

    struct object_t *new_obj = _object_clone(db_inst,
                                             *(struct object_t **) (_obj),
                                             _G.allocator);
    new_obj->obj = _obj;

    return (struct ct_cdb_obj_o *) new_obj;
}

static void _notify(struct ct_cdb_obj_t *_obj,
                    uint64_t *changed_prop) {
    struct object_t *obj = *(struct object_t **) _obj;

    const int notify_n = ct_array_size(obj->notify);
    const int changed_prop_n = ct_array_size(changed_prop);

    for (int i = 0; i < notify_n; ++i) {
        obj->notify[i](_obj, changed_prop, changed_prop_n);
    }

    for (int i = 0; i < ct_array_size(obj->instances); ++i) {
        _notify(obj->instances[i], changed_prop);
    }
}

static void write_commit(ct_cdb_obj_o *_writer) {
    struct object_t *writer = _writer;
    struct object_t *orig_obj = *(struct object_t **) writer->obj;

    (*(struct object_t **) writer->obj) = writer;

    _notify(writer->obj, writer->changed_prop);

    _destroy_object(orig_obj);
}

static void set_float(ct_cdb_obj_o *_writer,
                      uint64_t property,
                      float value) {
    struct object_t *writer = _writer;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_FLOAT, &value,
                                   sizeof(float),
                                   _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);

    memcpy((writer->values + writer->offset[idx]), &value, sizeof(float));
}

static void set_vec3(ct_cdb_obj_o *_writer,
                     uint64_t property,
                     const float *value) {
    const size_t size = sizeof(float) * 3;

    struct object_t *writer = _writer;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_VEC3, value,
                                   size, _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy((writer->values + writer->offset[idx]), value, size);
}

static void set_vec4(ct_cdb_obj_o *_writer,
                     uint64_t property,
                     const float *value) {
    const size_t size = sizeof(float) * 4;

    struct object_t *writer = _writer;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_VEC4, value,
                                   size, _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy((writer->values + writer->offset[idx]), value, size);
}

static void set_mat4(ct_cdb_obj_o *_writer,
                     uint64_t property,
                     const float *value) {
    const size_t size = sizeof(float) * 4;
    struct object_t *writer = _writer;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_MAT4, value,
                                   size, _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy((writer->values + writer->offset[idx]), value, size);
}

static void set_string(ct_cdb_obj_o *_writer,
                       uint64_t property,
                       const char *value) {
    struct ct_alloc *a = _G.allocator;

    struct object_t *writer = _writer;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_STR,
                                   &value, sizeof(char *),
                                   _G.allocator);
    } else {
        CT_FREE(a, *((char **) (writer->values + writer->offset[idx])));
    }

    char *value_clone = ct_memory_a0.str_dup(value, a);

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy(writer->values + writer->offset[idx], &value_clone,
           sizeof(value_clone));
}

static void set_uint64(ct_cdb_obj_o *_writer,
                       uint64_t property,
                       uint64_t value) {
    struct object_t *writer = _writer;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property, CDB_TYPE_UINT64,
                                   &value, sizeof(uint64_t), _G.allocator);
    }
    ct_array_push(writer->changed_prop, property, _G.allocator);

    memcpy((writer->values + writer->offset[idx]), &value, sizeof(uint64_t));
}

static void set_ptr(ct_cdb_obj_o *_writer,
                    uint64_t property,
                    const void *value) {
    struct object_t *writer = _writer;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property, CDB_TYPE_PTR, &value,
                                   sizeof(void *),
                                   _G.allocator);
    }
    ct_array_push(writer->changed_prop, property, _G.allocator);

    memcpy((writer->values + writer->offset[idx]), &value, sizeof(void *));
}

static void set_ref(ct_cdb_obj_o *_writer,
                    uint64_t property,
                    struct ct_cdb_obj_t *ref) {

    struct object_t *writer = _writer;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_REF, ref,
                                   sizeof(struct ct_cdb_obj_t *),
                                   _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy((writer->values + writer->offset[idx]), &ref,
           sizeof(struct ct_cdb_obj_t *));
}

static bool prop_exist(struct ct_cdb_obj_t *_object,
                       uint64_t key) {
    struct object_t *obj = *(struct object_t **) _object;
    return _find_prop_index(obj, key) > 0;
}

static enum ct_cdb_type prop_type(struct ct_cdb_obj_t *_object,
                                  uint64_t key) {
    struct object_t *obj = *(struct object_t **) _object;
    uint64_t idx = _find_prop_index(obj, key);

    if (idx) {
        return (enum ct_cdb_type) obj->ptype[idx];
    }

    if (obj->owner) {
        return prop_type(obj->owner, key);
    }

    return CDB_TYPE_NONE;
}

static float read_float(struct ct_cdb_obj_t *_obj,
                        uint64_t property,
                        float defaultt) {
    struct object_t *obj = *(struct object_t **) _obj;
    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(float *) (obj->values + obj->offset[idx]);
    }

    if (obj->owner) {
        return read_float(obj->owner, property, defaultt);
    }

    return defaultt;
}

static void read_vec3(struct ct_cdb_obj_t *_obj,
                      uint64_t property,
                      float *value) {
    struct object_t *obj = *(struct object_t **) _obj;
    uint64_t idx = _find_prop_index(obj, property);


    if (idx) {
        const float *f = (const float *) (obj->values + obj->offset[idx]);
        memcpy(value, f, sizeof(float) * 3);

        return;
    }

    if (obj->owner) {
        read_vec3(obj->owner, property, value);
    }
}

static void read_vec4(struct ct_cdb_obj_t *_obj,
                      uint64_t property,
                      float *value) {
    struct object_t *obj = *(struct object_t **) _obj;
    uint64_t idx = _find_prop_index(obj, property);


    if (idx) {
        const float *f = (const float *) (obj->values + obj->offset[idx]);
        memcpy(value, f, sizeof(float) * 4);

        return;
    }

    if (obj->owner) {
        read_vec4(obj->owner, property, value);
    }
}

static void read_mat4(struct ct_cdb_obj_t *_obj,
                      uint64_t property,
                      float *value) {
    struct object_t *obj = *(struct object_t **) _obj;
    uint64_t idx = _find_prop_index(obj, property);


    if (idx) {
        const float *f = (const float *) (obj->values + obj->offset[idx]);
        memcpy(value, f, sizeof(float) * 16);

        return;
    }

    if (obj->owner) {
        read_mat4(obj->owner, property, value);
    }

}

static const char *read_string(struct ct_cdb_obj_t *_obj,
                               uint64_t property,
                               const char *defaultt) {
    struct object_t *obj = *(struct object_t **) _obj;
    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(const char **) (obj->values + obj->offset[idx]);
    }

    if (obj->owner) {
        return read_string(obj->owner, property, defaultt);
    }

    return defaultt;
}


static uint64_t read_uint64(struct ct_cdb_obj_t *_obj,
                            uint64_t property,
                            uint64_t defaultt) {
    struct object_t *obj = *(struct object_t **) _obj;
    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(uint64_t *) (obj->values + obj->offset[idx]);
    }

    if (obj->owner) {
        return read_uint64(obj->owner, property, defaultt);
    }

    return defaultt;
}

static void *read_ptr(struct ct_cdb_obj_t *_obj,
                      uint64_t property,
                      void *defaultt) {
    struct object_t *obj = *(struct object_t **) _obj;
    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(void **) (obj->values + obj->offset[idx]);
    }

    if (obj->owner) {
        return read_ptr(obj->owner, property, defaultt);
    }

    return defaultt;
}

static struct ct_cdb_obj_t *read_ref(struct ct_cdb_obj_t *_obj,
                                     uint64_t property,
                                     struct ct_cdb_obj_t *defaultt) {
    struct object_t *obj = *(struct object_t **) _obj;
    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(struct ct_cdb_obj_t **) (obj->values + obj->offset[idx]);
    }

    if (obj->owner) {
        return read_ptr(obj->owner, property, defaultt);
    }

    return defaultt;
}

static uint64_t *prop_keys(struct ct_cdb_obj_t *_obj) {
    struct object_t *obj = *(struct object_t **) _obj;
    return obj->keys + 1;
}

static uint64_t prop_count(struct ct_cdb_obj_t *_obj) {
    struct object_t *obj = *(struct object_t **) _obj;
    return obj->properties_count - 1;
}

void register_notify(struct ct_cdb_obj_t *_obj,
                     ct_cdb_notify notify) {
    struct object_t *obj = *(struct object_t **) _obj;

    ct_array_push(obj->notify, notify, _G.allocator);
}


static struct ct_cdb_t global_db() {
    return _G.global_db;
}


static uint64_t type(struct ct_cdb_obj_t *_obj) {
    struct object_t *obj = *(struct object_t **) _obj;

    return obj->type;
}

static struct ct_cdb_a0 cdb_api = {
        .register_notify = register_notify,
//        .create_db = create_db,

        . global_db  = global_db,

        .type = type,
        .create_object = create_object,
        .create_from = create_from,
        .destroy_object = destroy_object,
        .destroy_db = destroy_db,

        .gc = gc,

        .add_child = add_child,
        .children = children,

        .dump = dump,
        .load = load,

        .prop_exist = prop_exist,
        .prop_type = prop_type,
        .prop_keys = prop_keys,
        .prop_count = prop_count,

        .read_float = read_float,
        .read_vec3 = read_vec3,
        .read_vec4 = read_vec4,
        .read_mat4 = read_mat4,
        .read_str = read_string,
        .read_uint64 = read_uint64,
        .read_ptr = read_ptr,
        .read_ref = read_ref,

        .write_begin = write_begin,
        .write_commit = write_commit,

        .set_float = set_float,
        .set_vec3 = set_vec3,
        .set_vec4 = set_vec4,
        .set_mat4 = set_mat4,
        .set_string = set_string,
        .set_uint64 = set_uint64,
        .set_ptr = set_ptr,
        .set_ref = set_ref,
};


static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator(),
    };

    _G.global_db = create_db(),
            api->register_api("ct_cdb_a0", &cdb_api);
}

static void _shutdown() {
    _G = (struct _G) {0};
}

CETECH_MODULE_DEF(
        cdb,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_log_a0);
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