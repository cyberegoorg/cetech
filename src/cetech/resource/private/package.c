//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <corelib/api_system.h>
#include <corelib/memory.h>
#include <cetech/resource/resource.h>
#include <corelib/task.h>
#include <corelib/os.h>
#include <corelib/hashlib.h>
#include <corelib/ydb.h>
#include <corelib/macros.h>
#include <corelib/array.inl>
#include <corelib/cdb.h>
#include <corelib/yng.h>
#include <cetech/resource/package.h>

//==============================================================================
// Public interface
//==============================================================================

struct package_task_data {
    uint64_t name;
};

#define _G PackageGlobals
struct _G {
    struct ct_alloc *allocator;
} _G = {};

void online(uint64_t name,
            struct ct_vio *input,
            uint64_t obj) {
    const uint64_t size = input->size(input);
    char *data = CT_ALLOC(_G.allocator, char, size);
    input->read(input, data, 1, size);

    ct_cdb_a0->load(ct_cdb_a0->db(), data, obj, _G.allocator);
}

void offline(uint64_t name,
             uint64_t obj) {
    CT_UNUSED(name, obj);
}

void *reloader(uint64_t name,
               void *old_data,
               void *new_data,
               struct ct_alloc *allocator) {
    CT_UNUSED(name);

    CT_FREE(allocator, old_data);
    return new_data;
}

static uint64_t cdb_type() {
    return PACKAGE_TYPE;
}


//==============================================================================
// Resource compiler
//==============================================================================

void _package_compiler(const char *filename,
                       char **output) {
    uint64_t tmp_keys = 0;

    uint64_t type_keys[32] = {};
    uint32_t type_keys_count = 0;
    ct_ydb_a0->get_map_keys(filename,
                            &tmp_keys, 1,
                            type_keys, CT_ARRAY_LEN(type_keys),
                            &type_keys_count);

    uint64_t obj = ct_cdb_a0->create_object(ct_cdb_a0->db(), PACKAGE_TYPE);
    uint64_t types_obj = ct_cdb_a0->create_object(ct_cdb_a0->db(), PACKAGE_TYPES_PROP);

    ct_cdb_obj_o *w_obj = ct_cdb_a0->write_begin(obj);
    ct_cdb_a0->set_subobject(w_obj, PACKAGE_TYPES_PROP, types_obj);
    ct_cdb_a0->write_commit(w_obj);

    ct_cdb_obj_o *w_types_obj = ct_cdb_a0->write_begin(types_obj);

    for (uint32_t i = 0; i < type_keys_count; ++i) {
        uint64_t type_id = type_keys[i];

        uint64_t type_obj = ct_cdb_a0->create_object(ct_cdb_a0->db(), type_id);

        ct_cdb_a0->set_subobject(w_types_obj, type_id, type_obj);

        ct_cdb_obj_o *w_type_obj = ct_cdb_a0->write_begin(type_obj);

        uint64_t name_keys[32] = {};
        uint32_t name_keys_count = 0;
        ct_ydb_a0->get_map_keys(filename,
                                &type_keys[i], 1,
                                name_keys, CT_ARRAY_LEN(name_keys),
                                &name_keys_count);

        for (uint32_t j = 0; j < name_keys_count; ++j) {
            ct_cdb_a0->set_str(w_type_obj, name_keys[j], "");
        }

        ct_cdb_a0->write_commit(w_type_obj);
    }

    ct_cdb_a0->write_commit(w_types_obj);

    ct_cdb_a0->dump(obj, output, _G.allocator);
    ct_cdb_a0->destroy_object(obj);
}

static struct ct_resource_i0 ct_resource_i0 = {
        .cdb_type = cdb_type,
        .online = online,
        .offline =offline,
        .compilator = _package_compiler,
};

int package_init(struct ct_api_a0 *api) {
    CT_INIT_API(api, ct_memory_a0);
    CT_INIT_API(api, ct_resource_a0);
    CT_INIT_API(api, ct_task_a0);
    CT_INIT_API(api, ct_os_a0);
    CT_INIT_API(api, ct_hashlib_a0);
    CT_INIT_API(api, ct_ydb_a0);
    CT_INIT_API(api, ct_yng_a0);
    CT_INIT_API(api, ct_cdb_a0);

    _G = (struct _G) {
            .allocator = ct_memory_a0->system,
    };

    ct_api_a0->register_api(RESOURCE_I_NAME, &ct_resource_i0);

    return 1;
}

void package_shutdown() {

}

void package_load_task(void *data) {
    uint64_t type_obj = (uint64_t) data;

    const uint64_t name_n = ct_cdb_a0->prop_count(type_obj);
    uint64_t names[name_n];
    ct_cdb_a0->prop_keys(type_obj, names);

    ct_resource_a0->load_now(ct_cdb_a0->type(type_obj), names, name_n);
}

void package_task(void *data) {
    struct package_task_data *task_data = (struct package_task_data *) data;

    struct ct_resource_id rid = (struct ct_resource_id) {
            .type = PACKAGE_TYPE,
            .name = task_data->name
    };

    uint64_t obj = ct_resource_a0->get(rid);
    uint64_t types_obj = ct_cdb_a0->read_subobject(obj, PACKAGE_TYPES_PROP, 0);

    struct ct_task_item *load_tasks = NULL;
    const uint64_t type_n = ct_cdb_a0->prop_count(types_obj);
    uint64_t types[type_n];
    ct_cdb_a0->prop_keys(types_obj, types);

    for (uint32_t j = 0; j < type_n; ++j) {
        uint64_t type_obj = ct_cdb_a0->read_subobject(types_obj, types[j], 0);

        struct ct_task_item item = {
                .name = "package_task",
                .work = package_load_task,
                .data = (void *) type_obj,
        };

        ct_array_push(load_tasks, item, _G.allocator);
    }

    struct ct_task_counter_t *counter = NULL;
    ct_task_a0->add(load_tasks, type_n, &counter);
    ct_task_a0->wait_for_counter(counter, 0);

    CT_FREE(_G.allocator, task_data);
}

struct ct_task_counter_t *package_load(uint64_t name) {
    struct package_task_data *task_data =
            CT_ALLOC(_G.allocator,
                     struct package_task_data,
                     sizeof(struct package_task_data));

    task_data->name = name;

    struct ct_task_item item = {
            .name = "package_task",
            .work = package_task,
            .data = task_data,
    };

    struct ct_task_counter_t *counter = NULL;
    ct_task_a0->add(&item, 1, &counter);

    return counter;
}

void package_unload(uint64_t name) {
    struct ct_resource_id rid = (struct ct_resource_id) {
            .type = PACKAGE_TYPE,
            .name = name
    };

    uint64_t obj = ct_resource_a0->get(rid);

    uint64_t types_obj = ct_cdb_a0->read_subobject(obj, PACKAGE_TYPES_PROP, 0);

    const uint64_t type_n = ct_cdb_a0->prop_count(types_obj);
    uint64_t types[type_n];
    ct_cdb_a0->prop_keys(types_obj, types);

    for (uint32_t j = 0; j < type_n; ++j) {
        uint64_t type_obj = ct_cdb_a0->read_subobject(types_obj, types[j], 0);

        const uint64_t name_n = ct_cdb_a0->prop_count(type_obj);
        uint64_t names[name_n];
        ct_cdb_a0->prop_keys(type_obj, names);

        ct_resource_a0->unload(types[j], names, name_n);
    }
}

int package_is_loaded(uint64_t name) {
    struct ct_resource_id rid = (struct ct_resource_id) {
            .type = PACKAGE_TYPE,
            .name = name
    };

    uint64_t obj = ct_resource_a0->get(rid);

    if (!obj) {
        return 0;
    }

    uint64_t types_obj = ct_cdb_a0->read_subobject(obj, PACKAGE_TYPES_PROP, 0);

    const uint64_t type_n = ct_cdb_a0->prop_count(types_obj);
    uint64_t types[type_n];
    ct_cdb_a0->prop_keys(types_obj, types);

    for (uint32_t j = 0; j < type_n; ++j) {
        uint64_t type_obj = ct_cdb_a0->read_subobject(types_obj, types[j], 0);

        const uint64_t name_n = ct_cdb_a0->prop_count(type_obj);
        uint64_t names[name_n];
        ct_cdb_a0->prop_keys(type_obj, names);

        if (!ct_resource_a0->can_get_all(types[j], names, name_n)) {
            return 0;
        }
    }

    return 1;
}

void package_flush(struct ct_task_counter_t *counter) {
    ct_task_a0->wait_for_counter(counter, 0);
}