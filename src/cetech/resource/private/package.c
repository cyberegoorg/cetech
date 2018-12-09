//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

#include <celib/api_system.h>
#include <celib/memory.h>
#include <celib/task.h>
#include <celib/os.h>
#include <celib/hashlib.h>
#include <celib/ydb.h>
#include <celib/macros.h>
#include <celib/array.inl>
#include <celib/cdb.h>

#include <cetech/resource/resource.h>
#include <cetech/resource/package.h>
#include <cetech/resource/builddb.h>
#include <celib/buffer.inl>

//==============================================================================
// Public interface
//==============================================================================

struct package_task_data {
    uint64_t name;
};

#define _G PackageGlobals
struct _G {
    struct ce_alloc *allocator;
} _G = {};


static uint64_t cdb_type() {
    return PACKAGE_TYPE;
}


//==============================================================================
// Resource compiler
//==============================================================================
static struct ct_resource_i0 ct_resource_i0 = {
        .cdb_type = cdb_type,
};

int package_init(struct ce_api_a0 *api) {
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_task_a0);
    CE_INIT_API(api, ce_os_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ce_cdb_a0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    ce_api_a0->register_api(RESOURCE_I_NAME, &ct_resource_i0);

    return 1;
}

void package_shutdown() {

}

void package_load_task(void *data) {
    uint64_t type_obj = (uint64_t) data;

    const uint64_t name_n = ce_cdb_a0->prop_count(type_obj);
    const uint64_t* names = ce_cdb_a0->prop_keys(type_obj);
    ct_resource_a0->load_now(ce_cdb_a0->obj_type(type_obj), names, name_n);
}

void package_task(void *data) {
    struct package_task_data *task_data = (struct package_task_data *) data;

    struct ct_resource_id rid = (struct ct_resource_id) {
            .type = PACKAGE_TYPE,
            .name = task_data->name
    };

    uint64_t obj = ct_resource_a0->get(rid);

    const ce_cdb_obj_o * reader = ce_cdb_a0->read(obj);
    uint64_t types_obj = ce_cdb_a0->read_subobject(reader, PACKAGE_TYPES_PROP, 0);

    struct ce_task_item *load_tasks = NULL;
    const uint64_t type_n = ce_cdb_a0->prop_count(types_obj);
    const uint64_t* types = ce_cdb_a0->prop_keys(types_obj);

    for (uint32_t j = 0; j < type_n; ++j) {
        const ce_cdb_obj_o * reader = ce_cdb_a0->read(types_obj);
        uint64_t type_obj = ce_cdb_a0->read_subobject(reader, types[j], 0);

        struct ce_task_item item = {
                .name = "package_task",
                .work = package_load_task,
                .data = (void *) type_obj,
        };

        ce_array_push(load_tasks, item, _G.allocator);
    }

    struct ce_task_counter_t *counter = NULL;
    ce_task_a0->add(load_tasks, type_n, &counter);
    ce_task_a0->wait_for_counter(counter, 0);

    CE_FREE(_G.allocator, task_data);
}

struct ce_task_counter_t *package_load(uint64_t name) {
    struct package_task_data *task_data =
            CE_ALLOC(_G.allocator,
                     struct package_task_data,
                     sizeof(struct package_task_data));

    task_data->name = name;

    struct ce_task_item item = {
            .name = "package_task",
            .work = package_task,
            .data = task_data,
    };

    struct ce_task_counter_t *counter = NULL;
    ce_task_a0->add(&item, 1, &counter);

    return counter;
}

void package_unload(uint64_t name) {
    struct ct_resource_id rid = (struct ct_resource_id) {
            .type = PACKAGE_TYPE,
            .name = name
    };

    uint64_t obj = ct_resource_a0->get(rid);

    const ce_cdb_obj_o * reader = ce_cdb_a0->read(obj);

    uint64_t types_obj = ce_cdb_a0->read_subobject(reader, PACKAGE_TYPES_PROP, 0);
    const ce_cdb_obj_o * types_reader = ce_cdb_a0->read(types_obj);


    const uint64_t type_n = ce_cdb_a0->prop_count(types_obj);
    const uint64_t* types = ce_cdb_a0->prop_keys(types_obj);

    for (uint32_t j = 0; j < type_n; ++j) {
        uint64_t type_obj = ce_cdb_a0->read_subobject(types_reader, types[j], 0);

        const uint64_t name_n = ce_cdb_a0->prop_count(type_obj);
        const uint64_t* names = ce_cdb_a0->prop_keys(type_obj);

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

    const ce_cdb_obj_o * reader = ce_cdb_a0->read(obj);

    uint64_t types_obj = ce_cdb_a0->read_subobject(reader, PACKAGE_TYPES_PROP, 0);

    const ce_cdb_obj_o * types_reader = ce_cdb_a0->read(types_obj);

    const uint64_t type_n = ce_cdb_a0->prop_count(types_obj);
    const uint64_t* types = ce_cdb_a0->prop_keys(types_obj);

    for (uint32_t j = 0; j < type_n; ++j) {
        uint64_t type_obj = ce_cdb_a0->read_subobject(types_reader, types[j], 0);

        const uint64_t name_n = ce_cdb_a0->prop_count(type_obj);
        const uint64_t* names = ce_cdb_a0->prop_keys(type_obj);

        if (!ct_resource_a0->can_get_all(types[j], names, name_n)) {
            return 0;
        }
    }

    return 1;
}

void package_flush(struct ce_task_counter_t *counter) {
    ce_task_a0->wait_for_counter(counter, 0);
}