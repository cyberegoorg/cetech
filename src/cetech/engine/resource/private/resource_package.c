//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <cetech/core/api/api_system.h>
#include <cetech/core/memory/memory.h>
#include <cetech/engine/resource/resource.h>
#include <cetech/core/task/task.h>
#include <cetech/core/os/thread.h>
#include <cetech/core/os/vio.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/macros.h>
#include <cetech/core/containers/array.h>

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_task_a0);
CETECH_DECL_API(ct_thread_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yng_a0);

struct package_resource {
    uint32_t type_count;
    uint32_t name_count_offset;
    uint32_t type_offset;
    uint32_t name_offset;
    uint32_t offset_offset;
};

#define package_name_count(resource_ptr) ((uint32_t*)((void*)(((char*)(resource_ptr)) + (resource_ptr)->name_count_offset)))
#define package_offset(resource_ptr) ((uint32_t*)((void*)(((char*)(resource_ptr)) + (resource_ptr)->offset_offset)))
#define package_type(resource_ptr) ((uint64_t*)((void*)(((char*)(resource_ptr)) + (resource_ptr)->type_offset)))
#define package_name(resource_ptr) ((uint64_t*)((void*)(((char*)(resource_ptr)) + (resource_ptr)->name_offset)))


//==============================================================================
// Public interface
//==============================================================================

struct package_task_data {
    uint64_t name;
};

#define _G PackageGlobals
struct _G {
    uint64_t package_typel;
    struct ct_alloc *allocator;
} _G = {};

void *loader(struct ct_vio *input,
             struct ct_alloc *allocator) {

    const int64_t size = input->size(input);
    char *data = CT_ALLOC(allocator, char, size);
    input->read(input, data, 1, size);

    return data;
}

void unloader(void *new_data,
              struct ct_alloc *allocator) {
    CT_FREE(allocator, new_data);
}

void online(uint64_t name,
            void *data) {
    CT_UNUSED(name, data);
}

void offline(uint64_t name,
             void *data) {
    CT_UNUSED(name, data);
}

void *reloader(uint64_t name,
               void *old_data,
               void *new_data,
               struct ct_alloc *allocator) {
    CT_UNUSED(name);

    CT_FREE(allocator, old_data);
    return new_data;
}

static const ct_resource_callbacks_t package_resource_callback = {
        .loader = loader,
        .unloader =unloader,
        .online =online,
        .offline =offline,
        .reloader = reloader
};


//==============================================================================
// Resource compiler
//==============================================================================

struct package_compile_data {
    uint64_t *types;
    uint64_t *name;
    uint32_t *name_count;
    uint32_t *offset;
};

void _package_compiler(const char *filename,
                       char **output,
                       struct ct_compilator_api *compilator_api) {
    CT_UNUSED(compilator_api);

    struct package_compile_data compile_data = {};

    uint64_t tmp_keys = 0;

    uint64_t type_keys[32] = {};
    uint32_t type_keys_count = 0;
    ct_ydb_a0.get_map_keys(filename,
                           &tmp_keys, 1,
                           type_keys, CETECH_ARRAY_LEN(type_keys),
                           &type_keys_count);

    for (uint32_t i = 0; i < type_keys_count; ++i) {
        uint64_t type_id = type_keys[i];

        ct_array_push(compile_data.types, type_id, _G.allocator);
        ct_array_push(compile_data.offset, ct_array_size(compile_data.name),
                       _G.allocator);

        uint64_t name_keys[32] = {};
        uint32_t name_keys_count = 0;
        ct_ydb_a0.get_map_keys(filename,
                               &type_keys[i], 1,
                               name_keys, CETECH_ARRAY_LEN(name_keys),
                               &name_keys_count);

        ct_array_push(compile_data.name_count, name_keys_count, _G.allocator);

        for (uint32_t j = 0; j < name_keys_count; ++j) {
            ct_array_push(compile_data.name, name_keys[j], _G.allocator);
        }
    }

    struct package_resource resource = {};
    resource.type_count = ct_array_size(compile_data.types);
    resource.type_offset = sizeof(resource);

    resource.name_count_offset = resource.type_offset +
                                 (ct_array_size(compile_data.types) *
                                  sizeof(uint64_t));

    resource.name_offset = resource.name_count_offset +
                           (ct_array_size(compile_data.name_count) *
                            sizeof(uint32_t));

    resource.offset_offset = resource.name_offset +
                             (ct_array_size(compile_data.name) *
                              sizeof(uint64_t));

    ct_array_push_n(*output, &resource, sizeof(resource), _G.allocator);
    ct_array_push_n(*output, compile_data.types,
                     sizeof(uint64_t) * ct_array_size(compile_data.types),
                     _G.allocator);
    ct_array_push_n(*output, compile_data.name_count,
                     sizeof(uint32_t) * ct_array_size(compile_data.name_count),
                     _G.allocator);
    ct_array_push_n(*output, compile_data.name,
                     sizeof(uint64_t) * ct_array_size(compile_data.name),
                     _G.allocator);
    ct_array_push_n(*output, compile_data.offset,
                     sizeof(uint32_t) * ct_array_size(compile_data.offset),
                     _G.allocator);
}

int package_init(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_resource_a0);
    CETECH_GET_API(api, ct_task_a0);
    CETECH_GET_API(api, ct_thread_a0);
    CETECH_GET_API(api, ct_vio_a0);
    CETECH_GET_API(api, ct_hash_a0);
    CETECH_GET_API(api, ct_ydb_a0);
    CETECH_GET_API(api, ct_yng_a0);

    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator(),
            .package_typel = CT_ID64_0("package"),
    };

    ct_resource_a0.register_type(_G.package_typel,
                                 package_resource_callback);

    ct_resource_a0.compiler_register(_G.package_typel, _package_compiler, true);

    return 1;
}

void package_shutdown() {

}


void package_task(void *data) {
    struct package_task_data *task_data = (struct package_task_data *) data;
    struct package_resource *package = (struct package_resource *) ct_resource_a0.get(
            _G.package_typel,
            task_data->name);

    const uint32_t task_count = package->type_count;
    for (uint32_t j = 0; j < task_count; ++j) {
        ct_resource_a0.load_now(package_type(package)[j],
                                &package_name(package)[package_offset(
                                        package)[j]],
                                package_name_count(package)[j]);
    }

    CT_FREE(ct_memory_a0.main_allocator(), task_data);
}

void package_load(uint64_t name) {

    struct package_task_data *task_data =
            CT_ALLOC(ct_memory_a0.main_allocator(),
                         struct package_task_data,
                         sizeof(struct package_task_data));

    task_data->name = name;

    struct ct_task_item item = {
            .name = "package_task",
            .work = package_task,
            .data = task_data,
            .affinity = TASK_AFFINITY_NONE
    };

    ct_task_a0.add(&item, 1);
}

void package_unload(uint64_t name) {
    struct package_resource *package = (struct package_resource *) ct_resource_a0.get(
            _G.package_typel,
            name);

    const uint32_t task_count = package->type_count;
    for (uint32_t j = 0; j < task_count; ++j) {
        ct_resource_a0.unload(package_type(package)[j],
                              &package_name(package)[package_offset(
                                      package)[j]],
                              package_name_count(package)[j]);
    }
}

int package_is_loaded(uint64_t name) {
    const uint64_t package_type = CT_ID64_0("package");
    struct package_resource *package = (struct package_resource *) ct_resource_a0.get(
            package_type, name);

    if (package == NULL) {
        return 0;
    }

    const uint32_t task_count = package->type_count;

    for (uint32_t i = 0; i < task_count; ++i) {
        if (!ct_resource_a0.can_get_all(package_type(package)[i],
                                        &package_name(package)[package_offset(
                                                package)[i]],
                                        package_name_count(package)[i])) {
            return 0;
        }
    }

    return 1;
}

void package_flush(uint64_t name) {
    while (!package_is_loaded(name)) {
        if (!ct_task_a0.do_work()) {
            ct_thread_a0.yield();
        }
    }
}