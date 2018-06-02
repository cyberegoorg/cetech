#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <cetech/kernel/api/api_system.h>
#include <cetech/kernel/memory/memory.h>
#include <cetech/kernel/cdb/cdb.h>
#include <cetech/kernel/module/module.h>
#include <cetech/kernel/macros.h>
#include <cetech/kernel/memory/allocator.h>
#include <cetech/kernel/containers/hash.h>
#include <cetech/kernel/ebus/ebus.h>
#include <cetech/kernel/hashlib/hashlib.h>


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_ebus_a0);
CETECH_DECL_API(ct_hashlib_a0);

#define _G coredb_global
#define LOG_WHERE "coredb"

#define MAX_OBJECTS 1000000000ULL

struct object_t {
    // prefab
    struct ct_cdb_obj_t *owner;
    struct ct_cdb_obj_t **instances;

    // hiearchy
    struct ct_cdb_obj_t *parent;
    struct ct_cdb_obj_t **children;

    // writer
    struct ct_cdb_obj_t *orig_obj;
    uint64_t *changed_prop;

    // object
    uint64_t type;
    uint32_t idx;
    struct ct_cdb_t db;
    struct ct_hash_t prop_map; // TODO: STATIC HASH 128?

    uint64_t buffer_size;
    uint64_t properties_count;
    uint64_t values_size;

    uint8_t *buffer;
    uint64_t *keys;
    uint8_t *ptype;
    uint64_t *offset;
    uint8_t *values;
};

//struct type_item_t {
//    struct ct_hash_t object_hash;
//    struct ct_cdb_obj_t **objects;
//};

struct db_t {
    uint32_t idx;
    struct ct_hash_t type_hash;
    struct type_item_t *types;

    struct object_t **objects;
    uint64_t object_used;

    struct object_t *object_pool;
    uint32_t *free_objects;
    uint32_t *to_free_objects;
};

static struct _G {
    struct db_t *dbs;
    uint32_t *free_db;
    uint32_t *to_free_db;

    struct ct_alloc *allocator;
} _G;


static uint64_t _object_new_property(struct object_t *obj,
                                     uint64_t key,
                                     enum ct_cdb_type type,
                                     const void *value,
                                     size_t size,
                                     const struct ct_alloc *alloc) {

    const uint64_t prop_count = obj->properties_count;
    const uint64_t values_size = obj->values_size;

    const uint64_t new_prop_count = prop_count + 1;
    const uint64_t new_value_size = values_size + size;

    const size_t new_buffer_size = ((sizeof(uint64_t) +
                                     sizeof(uint8_t) +
                                     sizeof(uint64_t)) * new_prop_count) +
                                   (sizeof(uint8_t) * new_value_size);

    struct object_t new_obj = {
            .db = obj->db,
            .type = obj->type,
            .idx = obj->idx,
            .owner = obj->owner,
            .parent = obj->parent,
            .orig_obj = obj->orig_obj,
            .buffer = CT_ALLOC(alloc, uint8_t, new_buffer_size),
            .buffer_size = new_buffer_size,
            .properties_count = new_prop_count,
            .values_size = new_value_size
    };

    new_obj.keys = (uint64_t *) new_obj.buffer;
    new_obj.ptype = (uint8_t *) (new_obj.keys + new_prop_count);
    new_obj.offset = (uint64_t *) (new_obj.ptype + new_prop_count);
    new_obj.values = (uint8_t *) (new_obj.offset + new_prop_count);

    ct_hash_clone(&obj->prop_map, &new_obj.prop_map, alloc);

    if (obj->instances)
        ct_array_push_n(new_obj.instances, obj->instances,
                        ct_array_size(obj->instances), _G.allocator);

    if (obj->children)
        ct_array_push_n(new_obj.children, obj->children,
                        ct_array_size(obj->children), _G.allocator);

//    if (obj->changed_prop)
//        ct_array_push_n(new_obj.changed_prop, obj->changed_prop,
//                        ct_array_size(obj->changed_prop), _G.allocator);

    memcpy(new_obj.keys, obj->keys, sizeof(uint64_t) * prop_count);
    memcpy(new_obj.offset, obj->offset, sizeof(uint64_t) * prop_count);
    memcpy(new_obj.ptype, obj->ptype, sizeof(uint8_t) * prop_count);
    memcpy(new_obj.values, obj->values, sizeof(uint8_t) * values_size);

    new_obj.keys[prop_count] = key;
    new_obj.offset[prop_count] = values_size;
    new_obj.ptype[prop_count] = type;

    memcpy(new_obj.values + values_size, value, size);

    CT_FREE(alloc, obj->buffer);

    ct_hash_add(&new_obj.prop_map, key, prop_count, alloc);

    *obj = new_obj;

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
    const uint64_t values_size = obj->values_size;
    const uint64_t buffer_size = obj->buffer_size;

    struct object_t *new_obj = _new_object(db, alloc);

    new_obj->owner = obj->owner;
    new_obj->parent = obj->parent;
    new_obj->type = obj->type;

    new_obj->buffer = CT_ALLOC(alloc, uint8_t, buffer_size);
    new_obj->buffer_size = buffer_size;
    new_obj->properties_count = properties_count;
    new_obj->values_size = values_size;

    new_obj->keys = (uint64_t *) new_obj->buffer;
    new_obj->ptype = (uint8_t *) (new_obj->keys + properties_count);
    new_obj->offset = (uint64_t *) (new_obj->ptype + properties_count);
    new_obj->values = (uint8_t *) (new_obj->offset + properties_count);


    memcpy(new_obj->buffer, obj->buffer, sizeof(uint8_t) * buffer_size);
    ct_hash_clone(&obj->prop_map, &new_obj->prop_map, alloc);

    uint32_t n = ct_array_size(obj->instances);
    if (n) {
        ct_array_push_n(new_obj->instances, obj->instances, n, alloc);
    }

    n = ct_array_size(obj->children);
    if (n) {
        ct_array_push_n(new_obj->children, obj->children, n, alloc);
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
            .objects = (struct object_t **) mmap(
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
    struct object_t **obj_addr = db_inst->objects + db_inst->object_used++;

    *obj_addr = obj;
    obj->db = db;

    return (struct ct_cdb_obj_t *) obj_addr;
}

static struct ct_cdb_obj_t *create_from(struct ct_cdb_t db,
                                        struct ct_cdb_obj_t *_obj) {
    struct db_t *db_inst = &_G.dbs[db.idx];

    struct object_t *obj = *(struct object_t **) _obj;

    struct object_t *inst = _new_object(db_inst, _G.allocator);
    inst->db = db;

    struct object_t **obj_addr = db_inst->objects + db_inst->object_used++;
    
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

//    _remove_from_type_slot(db_inst, _obj);

    ct_array_push(db_inst->to_free_objects, obj->idx, _G.allocator);
}


static void gc() {
    const uint32_t db_n = ct_array_size(_G.dbs);
    for (int i = 0; i < db_n; ++i) {
        struct db_t *db_inst = &_G.dbs[i];

        const uint32_t to_free_n = ct_array_size(db_inst->to_free_objects);
        for (int j = 0; j < to_free_n; ++j) {
            const uint32_t idx = db_inst->to_free_objects[j];

            struct object_t *obj = &db_inst->object_pool[idx];

            ct_array_clean(obj->children);
            ct_array_clean(obj->instances);
            ct_hash_clean(&obj->prop_map);

            CT_FREE(_G.allocator, obj->buffer);
            ct_array_free(obj->changed_prop, _G.allocator);

            *obj = (struct object_t) {
                    .children = obj->children,
                    .instances = obj->instances,
                    .prop_map = obj->prop_map
            };

            ct_array_push(db_inst->free_objects, idx, _G.allocator);
        }

        if (to_free_n) {
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

struct obj_header {
    uint64_t buffer_size;
    uint64_t properties_count;
    uint64_t values_size;
    uint64_t string_buffer_size;
};

static void dump(struct ct_cdb_obj_t *_obj,
                 char **output,
                 struct ct_alloc *allocator) {
    struct object_t *obj = *(struct object_t **) _obj;

    uint8_t *buffer = CT_ALLOC(allocator, uint8_t,
                               sizeof(uint8_t) * obj->buffer_size);
    memcpy(buffer, obj->buffer, sizeof(uint8_t) * obj->buffer_size);

    uint64_t *keys = (uint64_t *) buffer;
    uint8_t *type = (uint8_t *) (keys + obj->properties_count);
    uint64_t *offset = (uint64_t *) (type + obj->properties_count);
    uint8_t *values = (uint8_t *) (offset + obj->properties_count);

    char *str_buffer = NULL;
    for (int i = 0; i < obj->properties_count; ++i) {
        if (obj->ptype[i] != CDB_TYPE_STR) {
            continue;
        }

        uint64_t offset = ct_array_size(str_buffer);
        char *str = *(char **) (values + obj->offset[i]);
        ct_array_push_n(str_buffer, str, strlen(str) + 1, allocator);

        uint64_t *strptr = (uint64_t *) (values + obj->offset[i]);
        *strptr = offset;
    }

    struct obj_header header = {
            .buffer_size = obj->buffer_size,
            .properties_count = obj->properties_count,
            .values_size = obj->values_size,
            .string_buffer_size = ct_array_size(str_buffer)
    };

    ct_array_push_n(*output, (char *) &header, sizeof(struct obj_header),
                    allocator);
    ct_array_push_n(*output, (char *) buffer, obj->buffer_size, allocator);
    ct_array_push_n(*output, (char *) str_buffer, header.string_buffer_size,
                    allocator);

    CT_FREE(allocator, buffer);
}

static void load(struct ct_cdb_obj_t *_obj,
                 const char *input,
                 struct ct_alloc *allocator) {
    struct object_t *obj = *(struct object_t **) _obj;

    struct obj_header *header = (struct obj_header *) input;

    obj->buffer_size = header->buffer_size;
    obj->properties_count = header->properties_count;
    obj->values_size = header->values_size;

    obj->buffer = CT_ALLOC(allocator, uint8_t, header->buffer_size);
    memcpy(obj->buffer, (uint8_t *) (header + 1), header->buffer_size);

    obj->keys = (uint64_t *) obj->buffer;
    obj->ptype = (uint8_t *) (obj->keys + header->properties_count);
    obj->offset = (uint64_t *) (obj->ptype + header->properties_count);
    obj->values = (uint8_t *) (obj->offset + header->properties_count);

    for (int i = 0; i < header->properties_count; ++i) {
        ct_hash_add(&obj->prop_map, obj->keys[i], i, allocator);
    }

    char *strbuffer = (char *) ((uint8_t *) (header + 1) + obj->buffer_size);
    for (int i = 0; i < obj->properties_count; ++i) {
        if (obj->ptype[i] != CDB_TYPE_STR) {
            continue;
        }

        uint64_t offset = *(uint64_t *) (obj->values + obj->offset[i]);
        *(char **) (obj->values + obj->offset[i]) = ct_memory_a0.str_dup(
                strbuffer + offset, allocator);
    }
}

static struct ct_cdb_obj_t *write_begin(struct ct_cdb_obj_t *_obj) {
    struct object_t *obj = *(struct object_t **) _obj;
    struct db_t *db_inst = &_G.dbs[obj->db.idx];

    struct object_t *new_obj = _object_clone(db_inst,
                                             *(struct object_t **) (_obj),
                                             _G.allocator);
    new_obj->orig_obj = _obj;

    struct object_t **clone_obj_addr =
            db_inst->objects + db_inst->object_used++;
    *clone_obj_addr = new_obj;

    return (struct ct_cdb_obj_t *) clone_obj_addr;
}

static void _notify(struct ct_cdb_obj_t *_obj,
                    uint64_t *changed_prop) {
    struct object_t *obj = *(struct object_t **) _obj;

    struct ct_cdb_obj_change_ev ev = {
            .obj = _obj,
            .prop_count = ct_array_size(changed_prop),
            .prop = changed_prop,
    };

    ct_ebus_a0.send(CDB_EBUS, CDB_OBJ_CHANGE,
                    (uint64_t) _obj, &ev, sizeof(ev));

    for (int i = 0; i < ct_array_size(obj->instances); ++i) {
        _notify(obj->instances[i], changed_prop);
    }
}

static void write_commit(struct ct_cdb_obj_t *_writer) {
    struct object_t *writer = *(struct object_t **) _writer;
    struct object_t *orig_obj = *(struct object_t **) writer->orig_obj;

    (*(struct object_t **) writer->orig_obj) = writer;

    _notify(writer->orig_obj, writer->changed_prop);

    _destroy_object(orig_obj);
}

static void set_float(struct ct_cdb_obj_t *_writer,
                      uint64_t property,
                      float value) {
    struct object_t *writer = *(struct object_t **) _writer;

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

static void set_vec3(struct ct_cdb_obj_t *_writer,
                     uint64_t property,
                     const float *value) {
    const size_t size = sizeof(float) * 3;

    struct object_t *writer = *(struct object_t **) _writer;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_VEC3, value,
                                   size, _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy((writer->values + writer->offset[idx]), value, size);
}

static void set_vec4(struct ct_cdb_obj_t *_writer,
                     uint64_t property,
                     const float *value) {
    const size_t size = sizeof(float) * 4;

    struct object_t *writer = *(struct object_t **) _writer;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_VEC4, value,
                                   size, _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy((writer->values + writer->offset[idx]), value, size);
}

static void set_mat4(struct ct_cdb_obj_t *_writer,
                     uint64_t property,
                     const float *value) {
    const size_t size = sizeof(float) * 4;
    struct object_t *writer = *(struct object_t **) _writer;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property,
                                   CDB_TYPE_MAT4, value,
                                   size, _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy((writer->values + writer->offset[idx]), value, size);
}

static void set_string(struct ct_cdb_obj_t *_writer,
                       uint64_t property,
                       const char *value) {
    struct ct_alloc *a = _G.allocator;

    struct object_t *writer = *(struct object_t **) _writer;

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

static void set_uint32(struct ct_cdb_obj_t *_writer,
                       uint64_t property,
                       uint32_t value) {
    struct object_t *writer = *(struct object_t **) _writer;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property, CDB_TYPE_UINT32, &value,
                                   sizeof(uint32_t),
                                   _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy((writer->values + writer->offset[idx]), &value, sizeof(uint32_t));
}

static void set_uint64(struct ct_cdb_obj_t *_writer,
                       uint64_t property,
                       uint64_t value) {
    struct object_t *writer = *(struct object_t **) _writer;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property, CDB_TYPE_UINT64,
                                   &value, sizeof(uint64_t), _G.allocator);
    }
    ct_array_push(writer->changed_prop, property, _G.allocator);

    memcpy((writer->values + writer->offset[idx]), &value, sizeof(uint64_t));
}

static void set_ptr(struct ct_cdb_obj_t *_writer,
                    uint64_t property,
                    void *value) {
    struct object_t *writer = *(struct object_t **) _writer;

    uint64_t idx = _find_prop_index(writer, property);
    if (!idx) {
        idx = _object_new_property(writer, property, CDB_TYPE_PTR, &value,
                                   sizeof(void *),
                                   _G.allocator);
    }
    ct_array_push(writer->changed_prop, property, _G.allocator);

    memcpy((writer->values + writer->offset[idx]), &value, sizeof(void *));
}

static void set_ref(struct ct_cdb_obj_t *_writer,
                    uint64_t property,
                    struct ct_cdb_obj_t *ref) {

    struct object_t *writer = *(struct object_t **) _writer;

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

static uint32_t read_uint32(struct ct_cdb_obj_t *_obj,
                            uint64_t property,
                            uint32_t defaultt) {
    struct object_t *obj = *(struct object_t **) _obj;
    uint64_t idx = _find_prop_index(obj, property);

    if (idx) {
        return *(uint32_t *) (obj->values + obj->offset[idx]);
    }

    if (obj->owner) {
        return read_uint32(obj->owner, property, defaultt);
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

static struct ct_cdb_a0 cdb_api = {
        .create_db = create_db,
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
        .read_uint32 = read_uint32,
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
        .set_uint32 = set_uint32,
        .set_uint64 = set_uint64,
        .set_ptr = set_ptr,
        .set_ref = set_ref,
};


static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator()
    };

    ct_ebus_a0.create_ebus(CDB_EBUS_NAME, CDB_EBUS);

    api->register_api("ct_cdb_a0", &cdb_api);
}

static void _shutdown() {
    _G = (struct _G) {0};
}

CETECH_MODULE_DEF(
        cdb,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_ebus_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
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