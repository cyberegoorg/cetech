#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <cetech/core/api/api_system.h>
#include <cetech/core/memory/memory.h>
#include <cetech/core/cdb/cdb.h>
#include <cetech/core/module/module.h>
#include <cetech/core/macros.h>
#include <cetech/core/memory/allocator.h>
#include <cetech/core/containers/hash.h>

CETECH_DECL_API(ct_memory_a0);

#define _G coredb_global
#define LOG_WHERE "coredb"

#define MAX_OBJECTS 1000000000ULL

struct object_t {
    struct ct_cdb_obj_t *prefab;
    struct ct_cdb_obj_t **instances;
    ct_cdb_chg_notify *notify;

    struct ct_hash_t prop_map; // TODO: STATIC HASH 128?
    uint8_t *buffer;
    uint64_t *keys;
    uint8_t *type;
    uint64_t *offset;
    uint8_t *values;
    uint64_t buffer_size;
    uint64_t properties_count;
    uint64_t values_size;
};

static struct _G {
    struct object_t **objects;
    uint64_t object_used;
    struct ct_alloc *allocator;
} _G;


static uint64_t _object_new_property(struct object_t *obj,
                                     uint64_t key,
                                     enum ct_cdb_prop_type type,
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
            .prefab = obj->prefab,
            .notify = obj->notify,
            .buffer = CT_ALLOC(alloc, uint8_t, new_buffer_size),
            .buffer_size = new_buffer_size,
            .properties_count = new_prop_count,
            .values_size = new_value_size
    };

    new_obj.keys = (uint64_t *) new_obj.buffer;
    new_obj.type = (uint8_t *) (new_obj.keys + new_prop_count);
    new_obj.offset = (uint64_t *) (new_obj.type + new_prop_count);
    new_obj.values = (uint8_t *) (new_obj.offset + new_prop_count);

    ct_hash_clone(&obj->prop_map, &new_obj.prop_map, alloc);

    memcpy(new_obj.keys, obj->keys, sizeof(uint64_t) * prop_count);
    memcpy(new_obj.offset, obj->offset, sizeof(uint64_t) * prop_count);
    memcpy(new_obj.type, obj->type, sizeof(uint8_t) * prop_count);
    memcpy(new_obj.values, obj->values, sizeof(uint8_t) * values_size);

    new_obj.keys[prop_count] = key;
    new_obj.offset[prop_count] = values_size;
    new_obj.type[prop_count] = type;

    if (value) {
        memcpy(new_obj.values + values_size, value, size);
    }

    CT_FREE(alloc, obj->buffer);

    ct_hash_add(&new_obj.prop_map, key, prop_count, alloc);

    *obj = new_obj;

    return prop_count;
}

struct object_t *_new_object(const struct ct_alloc *a) {
    struct object_t *obj = CT_ALLOC(a, struct object_t,
                                    sizeof(struct object_t));
    *obj = (struct object_t) {0};
    _object_new_property(obj, 0, CDB_TYPE_NONE, NULL, 0, a);
    return obj;
}

static struct object_t *_object_clone(struct object_t *obj,
                                      const struct ct_alloc *alloc) {
    const uint64_t properties_count = obj->properties_count;
    const uint64_t values_size = obj->values_size;
    const uint64_t buffer_size = obj->buffer_size;

    struct object_t *new_obj = _new_object(alloc);

    new_obj->buffer = CT_ALLOC(alloc, uint8_t, buffer_size);
    new_obj->buffer_size = buffer_size;
    new_obj->properties_count = properties_count;
    new_obj->values_size = values_size;
    new_obj->prefab = obj->prefab;

    new_obj->keys = (uint64_t *) new_obj->buffer;
    new_obj->type = (uint8_t *) (new_obj->keys + properties_count);
    new_obj->offset = (uint64_t *) (new_obj->type + properties_count);
    new_obj->values = (uint8_t *) (new_obj->offset + properties_count);

    memcpy(new_obj->buffer, obj->buffer, sizeof(uint8_t) * buffer_size);
    ct_hash_clone(&obj->prop_map, &new_obj->prop_map, alloc);

    uint32_t n = ct_array_size(obj->notify);
    if (n) {
        ct_array_push_n(new_obj->notify, obj->notify, n, alloc);
    }

    n = ct_array_size(obj->instances);
    if (n) {
        ct_array_push_n(new_obj->instances, obj->instances, n, alloc);
    }

    return new_obj;
}

static uint64_t _find_prop_index(const struct object_t *obj,
                                 uint64_t key) {
    return ct_hash_lookup(&obj->prop_map, key, 0);
}

static struct ct_cdb_obj_t *create_object() {
    struct ct_alloc *a = _G.allocator;

    struct object_t *obj = _new_object(a);

    struct object_t **obj_addr = _G.objects + _G.object_used++;
    *obj_addr = obj;

    return (struct ct_cdb_obj_t *) obj_addr;
}

static struct ct_cdb_obj_t *create_from(struct ct_cdb_obj_t *_obj) {
    struct object_t *obj = *(struct object_t **) _obj;

    struct object_t *inst = _new_object(_G.allocator);
    struct object_t **obj_addr = _G.objects + _G.object_used++;
    *obj_addr = inst;

    ct_array_push(obj->instances,
                  (struct ct_cdb_obj_t *) obj_addr,
                  _G.allocator);

    inst->prefab = _obj;

    return (struct ct_cdb_obj_t *) obj_addr;
}

struct obj_header {
    uint64_t buffer_size;
    uint64_t properties_count;
    uint64_t values_size;
};

static void dump(struct ct_cdb_obj_t *_obj,
                 char **output,
                 struct ct_alloc *allocator) {
    struct object_t *obj = *(struct object_t **) _obj;

    struct obj_header header = {
            .buffer_size = obj->buffer_size,
            .properties_count = obj->properties_count,
            .values_size = obj->values_size,
    };

    ct_array_push_n(*output, (char *) &header, sizeof(header), allocator);
    ct_array_push_n(*output, (char *) obj->buffer, obj->buffer_size, allocator);
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
    obj->type = (uint8_t *) (obj->keys + header->properties_count);
    obj->offset = (uint64_t *) (obj->type + header->properties_count);
    obj->values = (uint8_t *) (obj->offset + header->properties_count);

    for (int i = 0; i < header->properties_count; ++i) {
        ct_hash_add(&obj->prop_map, obj->keys[i], i, allocator);
    }
}

void register_notify(struct ct_cdb_obj_t *_obj,
                     ct_cdb_chg_notify clb) {
    struct object_t *obj = *(struct object_t **) _obj;
    ct_array_push(obj->notify, clb, _G.allocator);
}

struct writer_t {
    struct object_t **obj;
    struct object_t *clone_obj;
    uint64_t *changed_prop;
};

static struct ct_cdb_writer_t *write_begin(struct ct_cdb_obj_t *obj) {
    struct writer_t *writer = CT_ALLOC(_G.allocator,
                                       struct writer_t,
                                       sizeof(struct writer_t));
    *writer = (struct writer_t) {0};

    writer->clone_obj = _object_clone(*(struct object_t **) (obj),
                                      _G.allocator);
    writer->obj = (struct object_t **) obj;

    return (struct ct_cdb_writer_t *) writer;
}

static void _notify(struct ct_cdb_obj_t *_obj,
                    uint64_t *changed_prop) {
    struct object_t *obj = *(struct object_t **) _obj;

    for (int i = 0; i < ct_array_size(obj->notify); ++i) {
        obj->notify[i](_obj, changed_prop, ct_array_size(changed_prop));
    }

    obj = *(struct object_t **) _obj;
    for (int i = 0; i < ct_array_size(obj->instances); ++i) {
        _notify(obj->instances[i], changed_prop);
    }
}

static void write_commit(struct ct_cdb_writer_t *_writer) {
    struct writer_t *writer = (struct writer_t *) _writer;
    struct object_t *old_obj = *writer->obj;
    *writer->obj = writer->clone_obj;

    _notify((struct ct_cdb_obj_t *) writer->obj, writer->changed_prop);

    ct_hash_free(&old_obj->prop_map, _G.allocator);
    ct_array_free(writer->changed_prop, _G.allocator);
    CT_FREE(_G.allocator, old_obj->buffer);
    CT_FREE(_G.allocator, old_obj);
    CT_FREE(_G.allocator, writer);
}

static void set_float(struct ct_cdb_writer_t *_writer,
                      uint64_t property,
                      float value) {
    struct writer_t *writer = (struct writer_t *) _writer;

    struct object_t *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property,
                                   CDB_TYPE_FLOAT, &value,
                                   sizeof(float),
                                   _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);

    memcpy((obj->values + obj->offset[idx]), &value, sizeof(float));
}

static void set_vec3(struct ct_cdb_writer_t *_writer,
                     uint64_t property,
                     const float *value) {
    const size_t size = sizeof(float) * 3;
    struct writer_t *writer = (struct writer_t *) _writer;
    struct object_t *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property,
                                   CDB_TYPE_VEC3, value,
                                   size, _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy((obj->values + obj->offset[idx]), value, size);
}

static void set_vec4(struct ct_cdb_writer_t *_writer,
                     uint64_t property,
                     const float *value) {
    const size_t size = sizeof(float) * 4;
    struct writer_t *writer = (struct writer_t *) _writer;
    struct object_t *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property,
                                   CDB_TYPE_VEC4, value,
                                   size, _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy((obj->values + obj->offset[idx]), value, size);
}

static void set_mat4(struct ct_cdb_writer_t *_writer,
                     uint64_t property,
                     const float *value) {
    const size_t size = sizeof(float) * 4;
    struct writer_t *writer = (struct writer_t *) _writer;
    struct object_t *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property,
                                   CDB_TYPE_MAT4, value,
                                   size, _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy((obj->values + obj->offset[idx]), value, size);
}

static void set_string(struct ct_cdb_writer_t *_writer,
                       uint64_t property,
                       const char *value) {
    struct ct_alloc *a = _G.allocator;

    struct writer_t *writer = (struct writer_t *) _writer;

    struct object_t *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property,
                                   CDB_TYPE_STRPTR,
                                   &value, sizeof(char *),
                                   _G.allocator);
    } else {
        CT_FREE(a, *((char **) (obj->values + obj->offset[idx])));
    }

    char *value_clone = ct_memory_a0.str_dup(value, a);

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy(obj->values + obj->offset[idx], &value_clone, sizeof(value_clone));
}

static void set_uint32(struct ct_cdb_writer_t *_writer,
                       uint64_t property,
                       uint32_t value) {
    struct writer_t *writer = (struct writer_t *) _writer;

    struct object_t *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property, CDB_TYPE_UINT32, &value,
                                   sizeof(uint32_t),
                                   _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy((obj->values + obj->offset[idx]), &value, sizeof(uint32_t));
}

static void set_uint64(struct ct_cdb_writer_t *_writer,
                       uint64_t property,
                       uint64_t value) {
    struct writer_t *writer = (struct writer_t *) _writer;

    struct object_t *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property, CDB_TYPE_UINT64,
                                   &value, sizeof(uint64_t), _G.allocator);
    }
    ct_array_push(writer->changed_prop, property, _G.allocator);

    memcpy((obj->values + obj->offset[idx]), &value, sizeof(uint64_t));
}

static void set_ptr(struct ct_cdb_writer_t *_writer,
                    uint64_t property,
                    void *value) {
    struct writer_t *writer = (struct writer_t *) _writer;

    struct object_t *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property, CDB_TYPE_PTR, &value,
                                   sizeof(void *),
                                   _G.allocator);
    }
    ct_array_push(writer->changed_prop, property, _G.allocator);

    memcpy((obj->values + obj->offset[idx]), &value, sizeof(void *));
}

static void set_ref(struct ct_cdb_writer_t *_writer,
                    uint64_t property,
                    struct ct_cdb_obj_t *ref) {

    struct writer_t *writer = (struct writer_t *) _writer;

    struct object_t *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property,
                                   CDB_TYPE_REF, ref,
                                   sizeof(struct ct_cdb_obj_t *),
                                   _G.allocator);
    }

    ct_array_push(writer->changed_prop, property, _G.allocator);
    memcpy((obj->values + obj->offset[idx]), &ref,
           sizeof(struct ct_cdb_obj_t *));
}

static bool prop_exist(struct ct_cdb_obj_t *_object,
                       uint64_t key) {
    struct object_t *obj = *(struct object_t **) _object;
    return _find_prop_index(obj, key) > 0;
}

static enum ct_cdb_prop_type prop_type(struct ct_cdb_obj_t *_object,
                                       uint64_t key) {
    struct object_t *obj = *(struct object_t **) _object;
    uint64_t idx = _find_prop_index(obj, key);

    if (idx) {
        return (enum ct_cdb_prop_type) obj->type[idx];
    }

    if (obj->prefab) {
        return prop_type(obj->prefab, key);
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

    if (obj->prefab) {
        return read_float(obj->prefab, property, defaultt);
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

    if (obj->prefab) {
        read_vec3(obj->prefab, property, value);
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

    if (obj->prefab) {
        read_vec4(obj->prefab, property, value);
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

    if (obj->prefab) {
        read_mat4(obj->prefab, property, value);
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

    if (obj->prefab) {
        return read_string(obj->prefab, property, defaultt);
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

    if (obj->prefab) {
        return read_uint32(obj->prefab, property, defaultt);
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

    if (obj->prefab) {
        return read_uint64(obj->prefab, property, defaultt);
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

    if (obj->prefab) {
        return read_ptr(obj->prefab, property, defaultt);
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

    if (obj->prefab) {
        return read_ptr(obj->prefab, property, defaultt);
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
        .create_object = create_object,
        .create_from = create_from,

        .dump = dump,
        .load = load,

        .register_notify = register_notify,
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

    _G.objects = (struct object_t **) mmap(NULL,
                                           MAX_OBJECTS *
                                           sizeof(struct object_t *),
                                           PROT_READ | PROT_WRITE,
                                           MAP_PRIVATE | MAP_ANONYMOUS,
                                           -1, 0);

    api->register_api("ct_cdb_a0", &cdb_api);
}

static void _shutdown() {
    _G = (struct _G) {0};
}

CETECH_MODULE_DEF(
        cdb,
        {
            CETECH_GET_API(api, ct_memory_a0);
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