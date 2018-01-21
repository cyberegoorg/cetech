#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <cetech/core/api/api_system.h>
#include <cetech/core/memory/memory.h>
#include <cetech/core/coredb/coredb.h>
#include <cetech/core/module/module.h>
#include <cetech/core/macros.h>
#include <cetech/core/memory/allocator.h>
#include <cetech/core/containers/hash.h>

CETECH_DECL_API(ct_memory_a0);

#define _G coredb_global
#define LOG_WHERE "coredb"

#define MAX_OBJECTS 1000000000ULL

struct object {
    struct ct_hash_t prop_map;
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
    struct object **objects;
    uint64_t object_used;
} _G;


static uint64_t _object_new_property(struct object *obj,
                                     uint64_t key,
                                     enum ct_coredb_prop_type type,
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

    struct object new_obj = {
            .buffer = CT_ALLOC(alloc, uint8_t, new_buffer_size),
            .buffer_size = new_buffer_size,
            .properties_count = new_prop_count,
            .values_size = new_value_size
    };

    new_obj.keys = (uint64_t *) new_obj.buffer,
    new_obj.type = (uint8_t *) (new_obj.keys + new_prop_count),
    new_obj.offset = (uint64_t *) (new_obj.type + new_prop_count),
    new_obj.values = (uint8_t *) (new_obj.offset + new_prop_count),

            ct_hash_clone(&obj->prop_map, &new_obj.prop_map, alloc);

    memcpy(new_obj.keys, obj->keys, sizeof(uint64_t) * prop_count);
    memcpy(new_obj.offset, obj->offset, sizeof(uint64_t) * prop_count);
    memcpy(new_obj.type, obj->type, sizeof(uint8_t) * prop_count);
    memcpy(new_obj.values, obj->values, sizeof(uint8_t) * values_size);

    new_obj.keys[prop_count] = key;
    new_obj.offset[prop_count] = values_size;
    new_obj.type[prop_count] = type;

    memcpy(new_obj.values + values_size, value, size);

    CT_FREE(alloc, obj->buffer);

    ct_hash_add(&new_obj.prop_map, key, prop_count, alloc);

    *obj = new_obj;

    return prop_count;
}

struct object *_new_object(const struct ct_alloc *a) {
    struct object *obj = CT_ALLOC(a, struct object, sizeof(struct object));
    *obj = (struct object) {{0}};

    _object_new_property(obj, 0, COREDB_TYPE_NONE, NULL, 0, a);

    return obj;
}

static struct object *_object_clone(struct object *obj,
                                    const struct ct_alloc *alloc) {
    const uint64_t properties_count = obj->properties_count;
    const uint64_t values_size = obj->values_size;
    const uint64_t buffer_size = obj->buffer_size;

    struct object *new_obj = _new_object(alloc);

    new_obj->buffer = CT_ALLOC(alloc, uint8_t, buffer_size);
    new_obj->buffer_size = buffer_size;
    new_obj->properties_count = properties_count;
    new_obj->values_size = values_size;

    new_obj->keys = (uint64_t *) new_obj->buffer;
    new_obj->type = (uint8_t *) (new_obj->keys + properties_count);
    new_obj->offset = (uint64_t *) (new_obj->type + properties_count);
    new_obj->values = (uint8_t *) (new_obj->offset + properties_count);

    memcpy(new_obj->buffer, obj->buffer, sizeof(uint8_t) * buffer_size);
    ct_hash_clone(&obj->prop_map, &new_obj->prop_map, alloc);

    return new_obj;
}

static uint64_t _find_prop_index(const struct object *obj,
                                 uint64_t key) {
    return ct_hash_lookup(&obj->prop_map, key, 0);
}

static struct ct_cdb_object_t *create_object() {
    struct ct_alloc *a = ct_memory_a0.main_allocator();

    struct object *obj = _new_object(a);

    struct object **obj_addr = _G.objects + _G.object_used++;
    *obj_addr = obj;

    return (struct ct_cdb_object_t *) obj_addr;
}


struct writer_t {
    struct object **obj;
    struct object *clone_obj;
};

static struct ct_cdb_writer_t *write_begin(struct ct_cdb_object_t *obj) {
    struct writer_t *writer = CT_ALLOC(ct_memory_a0.main_allocator(),
                                       struct writer_t,
                                       sizeof(struct writer_t));

    writer->clone_obj = _object_clone(*(struct object **) (obj),
                                      ct_memory_a0.main_allocator());
    writer->obj = (struct object **) obj;

    return (struct ct_cdb_writer_t *) writer;
}

static void write_commit(struct ct_cdb_writer_t *_writer) {
    struct writer_t *writer = (struct writer_t *) _writer;

    *writer->obj = writer->clone_obj;
}

static void set_float(struct ct_cdb_writer_t *_writer,
                      uint64_t property,
                      float value) {
    struct writer_t *writer = (struct writer_t *) _writer;

    struct object *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property, COREDB_TYPE_FLOAT, &value,
                                   sizeof(float),
                                   ct_memory_a0.main_allocator());
    }

    memcpy((obj->values + obj->offset[idx]), &value, sizeof(float));
}

static void set_string(struct ct_cdb_writer_t *_writer,
                       uint64_t property,
                       const char *value) {
    struct ct_alloc *a = ct_memory_a0.main_allocator();

    struct writer_t *writer = (struct writer_t *) _writer;

    struct object *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property,
                                   COREDB_TYPE_STRPTR,
                                   &value, sizeof(char *),
                                   ct_memory_a0.main_allocator());
    } else {
        CT_FREE(a, *((char **) (obj->values + obj->offset[idx])));
    }

    char *value_clone = ct_memory_a0.str_dup(value, a);

    memcpy(obj->values + obj->offset[idx], &value_clone, sizeof(value_clone));
}

static void set_uint32(struct ct_cdb_writer_t *_writer,
                       uint64_t property,
                       uint32_t value) {
    struct writer_t *writer = (struct writer_t *) _writer;

    struct object *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property, COREDB_TYPE_UINT32, &value,
                                   sizeof(uint64_t),
                                   ct_memory_a0.main_allocator());
    }

    memcpy((obj->values + obj->offset[idx]), &value, sizeof(uint32_t));
}

static void set_uint64(struct ct_cdb_writer_t *_writer,
                       uint64_t property,
                       uint64_t value) {
    struct writer_t *writer = (struct writer_t *) _writer;

    struct object *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property, COREDB_TYPE_UINT32, &value,
                                   sizeof(uint64_t),
                                   ct_memory_a0.main_allocator());
    }

    memcpy((obj->values + obj->offset[idx]), &value, sizeof(uint64_t));
}

static void set_ptr(struct ct_cdb_writer_t *_writer,
                    uint64_t property,
                    void *value) {
    struct writer_t *writer = (struct writer_t *) _writer;

    struct object *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property, COREDB_TYPE_PTR, &value,
                                   sizeof(void*),
                                   ct_memory_a0.main_allocator());
    }

    memcpy((obj->values + obj->offset[idx]), &value, sizeof(void *));
}

static void set_ref(struct ct_cdb_writer_t *_writer,
                    uint64_t property,
                    struct ct_cdb_object_t *ref) {

    struct writer_t *writer = (struct writer_t *) _writer;

    struct object *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(obj, property);
    if (!idx) {
        idx = _object_new_property(obj, property, COREDB_TYPE_REF, ref,
                                   sizeof(struct ct_cdb_object_t *),
                                   ct_memory_a0.main_allocator());
    }
    memcpy((obj->values + obj->offset[idx]), &ref, sizeof(struct ct_cdb_object_t*));
}

static bool prop_exist(struct ct_cdb_object_t *_object,
                       uint64_t key) {
    struct object *obj = *(struct object **) _object;
    return _find_prop_index(obj, key) > 0;
}

static enum ct_coredb_prop_type prop_type(struct ct_cdb_object_t *_object,
                                          uint64_t key) {
    struct object *obj = *(struct object **) _object;
    uint64_t idx = _find_prop_index(obj, key);
    return idx ? obj->type[idx] : COREDB_TYPE_NONE;
}

static float read_float(struct ct_cdb_object_t *_obj,
                        uint64_t property,
                        float defaultt) {
    struct object *obj = *(struct object **) _obj;
    uint64_t idx = _find_prop_index(obj, property);
    return idx ? *(float *) (obj->values + obj->offset[idx]) : defaultt;
}

static const char *read_string(struct ct_cdb_object_t *_obj,
                               uint64_t property,
                               const char *defaultt) {
    struct object *obj = *(struct object **) _obj;
    uint64_t idx = _find_prop_index(obj, property);
    return idx ? *(const char **) (obj->values + obj->offset[idx]) : defaultt;
}

static uint32_t read_uint32(struct ct_cdb_object_t *_obj,
                            uint64_t property,
                            uint32_t defaultt) {
    struct object *obj = *(struct object **) _obj;
    uint64_t idx = _find_prop_index(obj, property);
    return idx ? *(uint32_t *) (obj->values + obj->offset[idx]) : defaultt;
}

static uint64_t read_uint64(struct ct_cdb_object_t *_obj,
                            uint64_t property,
                            uint64_t defaultt) {
    struct object *obj = *(struct object **) _obj;
    uint64_t idx = _find_prop_index(obj, property);
    return idx ? *(uint64_t *) (obj->values + obj->offset[idx]) : defaultt;
}

static void *read_ptr(struct ct_cdb_object_t *_obj,
                      uint64_t property,
                      void *defaultt) {
    struct object *obj = *(struct object **) _obj;
    uint64_t idx = _find_prop_index(obj, property);
    return idx ? *(void **) (obj->values + obj->offset[idx]) : defaultt;
}

static struct ct_cdb_object_t *read_ref(struct ct_cdb_object_t *_obj,
                                           uint64_t property,
                                           struct ct_cdb_object_t *defaultt) {
    struct object *obj = *(struct object **) _obj;
    uint64_t idx = _find_prop_index(obj, property);
    return idx ? *(struct ct_cdb_object_t **) (obj->values + obj->offset[idx]) : defaultt;
}

static uint64_t * prop_keys(struct ct_cdb_object_t *_obj){
    struct object *obj = *(struct object **) _obj;
    return obj->keys + 1;
}

static uint64_t prop_count(struct ct_cdb_object_t *_obj) {
    struct object *obj = *(struct object **) _obj;
    return obj->properties_count - 1;
}

static struct ct_cdb_a0 coredb_api = {
        .create_object = create_object,

        .prop_exist = prop_exist,
        .prop_type = prop_type,
        .prop_keys = prop_keys,
        .prop_count = prop_count,

        .read_float = read_float,
        .read_string = read_string,
        .read_uint32 = read_uint32,
        .read_uint64 = read_uint64,
        .read_ptr = read_ptr,
        .read_ref = read_ref,

        .write_begin = write_begin,
        .write_commit = write_commit,

        .set_float = set_float,
        .set_string = set_string,
        .set_uint32 = set_uint32,
        .set_uint64 = set_uint64,
        .set_ptr = set_ptr,
        .set_ref = set_ref,
};


static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {0};

    _G.objects = (struct object **) mmap(NULL,
                                         MAX_OBJECTS * sizeof(struct object *),
                                         PROT_READ | PROT_WRITE,
                                         MAP_PRIVATE | MAP_ANONYMOUS,
                                         -1, 0);

    api->register_api("ct_cdb_a0", &coredb_api);
}

static void _shutdown() {
    _G = (struct _G) {0};
}

CETECH_MODULE_DEF(
        coredb,
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