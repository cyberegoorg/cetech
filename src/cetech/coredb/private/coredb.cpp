#include <stdlib.h>
#include <sys/mman.h>

#include <cetech/api/api_system.h>
#include <cetech/os/memory.h>
#include <cetech/coredb/coredb.h>
#include <cetech/module/module.h>
#include <celib/macros.h>
#include <celib/allocator.h>
#include <cstring>

CETECH_DECL_API(ct_memory_a0);

#define _G coredb_global
#define LOG_WHERE "coredb"

#define MAX_OBJECTS 1000000000ULL



struct object {
    uint8_t *buffer;

    uint64_t *keys;
    ct_coredb_prop_type *type;
    uint64_t *offset;
    uint8_t *values;

    uint64_t buffer_size;
    uint64_t properties_count;
    uint64_t values_size;
};


static struct _G {
    object **objects;
    uint64_t object_used;
} _G;


static uint64_t _object_new_property(object *obj,
                                     uint64_t key,
                                     enum ct_coredb_prop_type type,
                                     const void *value,
                                     size_t size,
                                     const cel_alloc *alloc) {

    const uint64_t prop_count = obj->properties_count;
    const uint64_t values_size = obj->values_size;

    const uint64_t new_prop_count = prop_count + 1;
    const uint64_t new_value_size = values_size + size;

    const size_t new_buffer_size = ((sizeof(uint64_t) +
                                     sizeof(ct_coredb_prop_type) +
                                     sizeof(uint64_t)) * new_prop_count) +
                                   (sizeof(uint8_t) * new_value_size);

    struct object new_obj = {0};

    new_obj.buffer = CEL_ALLOCATE(alloc, uint8_t, new_buffer_size);
    new_obj.buffer_size = new_buffer_size;
    new_obj.properties_count = new_prop_count;
    new_obj.values_size = new_value_size;

    new_obj.keys = (uint64_t *) new_obj.buffer;
    new_obj.type = (ct_coredb_prop_type *) (new_obj.keys + new_prop_count);
    new_obj.offset = (uint64_t *) (new_obj.type + new_prop_count);
    new_obj.values = (uint8_t *) (new_obj.offset + new_prop_count);

    memcpy(new_obj.keys, obj->keys, sizeof(uint64_t) * prop_count);
    memcpy(new_obj.type, obj->type, sizeof(ct_coredb_prop_type) * prop_count);
    memcpy(new_obj.offset, obj->offset, sizeof(uint64_t) * prop_count);
    memcpy(new_obj.values, obj->values, sizeof(uint8_t) * values_size);

    new_obj.keys[prop_count] = key;
    new_obj.offset[prop_count] = values_size;
    new_obj.type[prop_count] = type;

    memcpy(new_obj.values + values_size, value, size);

    CEL_FREE(alloc, obj->buffer);

    *obj = new_obj;

    return prop_count;
}

struct object *_new_object(const struct cel_alloc *a) {
    struct object *obj = CEL_ALLOCATE(a, struct object, sizeof(struct object));
    *obj = {};

    _object_new_property(obj, 0, COREDB_TYPE_NONE, NULL, 0, a);

    return obj;
}

static object *_object_clone(object *obj, const cel_alloc *alloc) {
    const uint64_t properties_count = obj->properties_count;
    const uint64_t values_size = obj->values_size;
    const uint64_t buffer_size = obj->buffer_size;

    object *new_obj = _new_object(alloc);

    new_obj->buffer = CEL_ALLOCATE(alloc, uint8_t, buffer_size);
    new_obj->buffer_size = buffer_size;
    new_obj->properties_count = properties_count;
    new_obj->values_size = values_size;

    new_obj->keys = (uint64_t *) new_obj->buffer;
    new_obj->type = (ct_coredb_prop_type *) (new_obj->keys + properties_count);
    new_obj->offset = (uint64_t *) (new_obj->type + properties_count);
    new_obj->values = (uint8_t *) (new_obj->offset + properties_count);

    memcpy(new_obj->buffer, obj->buffer, sizeof(uint8_t) * buffer_size);

    return new_obj;
}

uint64_t _find_prop_index(uint64_t key, const uint64_t *keys, size_t size) {
    for (uint64_t i = 1; i < size; ++i) {
        if (keys[i] != key) {
            continue;
        }

        return i;
    }

    return 0;
}

static ct_coredb_object_t *create_object() {
    struct cel_alloc *a = ct_memory_a0.main_allocator();

    struct object *obj = _new_object(a);

    object **obj_addr = _G.objects + _G.object_used++;
    *obj_addr = obj;

    return (ct_coredb_object_t *) obj_addr;
}


struct writer_t {
    object **obj;
    object *clone_obj;
};

ct_coredb_writer_t *write_begin(ct_coredb_object_t *obj) {
    writer_t *writer = CEL_ALLOCATE(ct_memory_a0.main_allocator(),
                                    writer_t,
                                    sizeof(writer_t));

    writer->clone_obj = _object_clone(*(object **) (obj),
                                      ct_memory_a0.main_allocator());
    writer->obj = (object **) obj;

    return (ct_coredb_writer_t *) writer;
}

void write_commit(ct_coredb_writer_t *_writer) {
    writer_t *writer = (writer_t *) _writer;

    *writer->obj = writer->clone_obj;
}

void set_float(ct_coredb_writer_t *_writer,
               uint64_t property,
               float value) {
    writer_t *writer = (writer_t *) _writer;

    object *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(property, obj->keys, obj->properties_count);
    if (!idx) {
        idx = _object_new_property(obj, property, COREDB_TYPE_FLOAT, &value,
                                   sizeof(float),
                                   ct_memory_a0.main_allocator());
    }

    *(float *) (obj->values + obj->offset[idx]) = value;
}

void set_string(ct_coredb_writer_t *_writer,
                uint64_t property,
                const char *value) {
    cel_alloc *a = ct_memory_a0.main_allocator();

    writer_t *writer = (writer_t *) _writer;

    object *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(property, obj->keys, obj->properties_count);
    if (!idx) {
        idx = _object_new_property(obj, property,
                                   COREDB_TYPE_STRPTR,
                                   &value, sizeof(char*),
                                   ct_memory_a0.main_allocator());
    } else {
        CEL_FREE(a, ((char *) (obj->values + obj->offset[idx])));
    }

    char *value_clone = ct_memory_a0.str_dup(value, a);

    memcpy(obj->values + obj->offset[idx], &value_clone, sizeof(value_clone));
}

void set_uint32(ct_coredb_writer_t *_writer,
                uint64_t property,
                uint32_t value) {
    writer_t *writer = (writer_t *) _writer;

    object *obj = writer->clone_obj;

    uint64_t idx = _find_prop_index(property, obj->keys, obj->properties_count);
    if (!idx) {
        idx = _object_new_property(obj, property, COREDB_TYPE_UINT32, &value,
                                   sizeof(uint32_t),
                                   ct_memory_a0.main_allocator());
    }

    *(uint32_t *) (obj->values + obj->offset[idx]) = value;
}

bool prop_exist(ct_coredb_object_t *_object, uint64_t key) {
    object *obj = *(object **) _object;

    return _find_prop_index(key, obj->keys, obj->properties_count) > 0;
}

ct_coredb_prop_type prop_type(ct_coredb_object_t *_object, uint64_t key) {
    object *obj = *(object **) _object;

    uint64_t idx = _find_prop_index(key, obj->keys, obj->properties_count);

    return idx ? obj->type[idx] : COREDB_TYPE_NONE;
}

float read_float(ct_coredb_object_t *_obj,
                 uint64_t property,
                 float defaultt) {
    object *obj = *(object **) _obj;

    uint64_t idx = _find_prop_index(property, obj->keys, obj->properties_count);
    return idx ? *(float *) (obj->values + obj->offset[idx]) : defaultt;
}

const char *read_string(ct_coredb_object_t *_obj,
                        uint64_t property,
                        const char *defaultt) {
    object *obj = *(object **) _obj;

    uint64_t idx = _find_prop_index(property, obj->keys, obj->properties_count);
    return idx ? *(const char**) (obj->values + obj->offset[idx]) : defaultt;
}

uint32_t read_uint32(ct_coredb_object_t *_obj,
                        uint64_t property,
                        uint32_t defaultt) {
    object *obj = *(object **) _obj;

    uint64_t idx = _find_prop_index(property, obj->keys, obj->properties_count);
    return idx ? *(uint32_t *) (obj->values + obj->offset[idx]) : defaultt;
}

static struct ct_coredb_a0 coredb_api = {
        .create_object = create_object,

        .prop_exist = prop_exist,
        .prop_type = prop_type,

        .read_float = read_float,
        .read_string = read_string,
        .read_uint32 = read_uint32,

        .write_begin = write_begin,
        .write_commit = write_commit,

        .set_float = set_float,
        .set_string = set_string,
        .set_uint32 = set_uint32,
};


static void _init(ct_api_a0 *api) {
    _G = {};

    _G.objects = (object **) mmap(NULL,
                                  MAX_OBJECTS * sizeof(object *),
                                  PROT_READ | PROT_WRITE,
                                  MAP_PRIVATE | MAP_ANONYMOUS,
                                  -1, 0);

    api->register_api("ct_coredb_a0", &coredb_api);
}

static void _shutdown() {
    _G = {};
}

CETECH_MODULE_DEF(
        coredb,
        {
            CETECH_GET_API(api, ct_memory_a0);
        },
        {
            CEL_UNUSED(reload);
            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
            _shutdown();
        }
)