//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/resource.h>
#include <cetech/kernel/task.h>
#include <cetech/kernel/thread.h>
#include <cetech/kernel/vio.h>
#include <cetech/kernel/hashlib.h>
#include <cetech/kernel/ydb.h>
#include <cetech/kernel/blob.h>

#include "celib/allocator.h"
#include "celib/array.inl"

#include "resource_package.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_task_a0);
CETECH_DECL_API(ct_thread_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yng_a0);

using namespace celib;

//==============================================================================
// Public interface
//==============================================================================

struct package_task_data {
    uint64_t name;
};

#define _G PackageGlobals
struct G {
    uint64_t package_typel;
} _G = {};

//==============================================================================
// Resource compiler
//==============================================================================

struct package_compile_data {
    Array<uint64_t> types;
    Array<uint64_t> name;
    Array<uint32_t> name_count;
    Array<uint32_t> offset;
};

void _package_compiler(const char *filename,
                      struct ct_blob *output,
                      ct_compilator_api *compilator_api) {
    CEL_UNUSED(compilator_api);

    struct package_compile_data compile_data = {};

    compile_data.types.init(ct_memory_a0.main_allocator());
    compile_data.name.init(ct_memory_a0.main_allocator());
    compile_data.offset.init(ct_memory_a0.main_allocator());
    compile_data.name_count.init(ct_memory_a0.main_allocator());

    uint64_t tmp_keys = 0;

    uint64_t type_keys[32] = {};
    uint32_t type_keys_count = 0;
    ct_ydb_a0.get_map_keys(filename,
                           &tmp_keys, 1,
                           type_keys, CETECH_ARRAY_LEN(type_keys),
                           &type_keys_count);

    for (uint32_t i = 0; i < type_keys_count; ++i) {
        uint64_t type_id = type_keys[i];

        array::push_back(compile_data.types, type_id);
        array::push_back(compile_data.offset, array::size(compile_data.name));

        uint64_t name_keys[32] = {};
        uint32_t name_keys_count = 0;
        ct_ydb_a0.get_map_keys(filename,
                               &type_keys[i], 1,
                               name_keys, CETECH_ARRAY_LEN(name_keys),
                               &name_keys_count);

        array::push_back(compile_data.name_count, name_keys_count);

        for (uint32_t j = 0; j < name_keys_count; ++j) {
            array::push_back(compile_data.name, name_keys[j]);
        }
    }

    struct package_resource resource = {};
    resource.type_count = array::size(compile_data.types);
    resource.type_offset = sizeof(resource);

    resource.name_count_offset = resource.type_offset +
                                 (array::size(compile_data.types) *
                                  sizeof(uint64_t));

    resource.name_offset = resource.name_count_offset +
                           (array::size(compile_data.name_count) *
                            sizeof(uint32_t));

    resource.offset_offset = resource.name_offset +
                             (array::size(compile_data.name) *
                              sizeof(uint64_t));


    output->push(output->inst, &resource, sizeof(resource));
    output->push(output->inst, array::begin(compile_data.types),
                 sizeof(uint64_t) * array::size(compile_data.types));
    output->push(output->inst, array::begin(compile_data.name_count),
                 sizeof(uint32_t) * array::size(compile_data.name_count));
    output->push(output->inst, array::begin(compile_data.name),
                 sizeof(uint64_t) * array::size(compile_data.name));
    output->push(output->inst, array::begin(compile_data.offset),
                 sizeof(uint32_t) * array::size(compile_data.offset));
}

int package_init(ct_api_a0 *api) {
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_resource_a0);
    CETECH_GET_API(api, ct_task_a0);
    CETECH_GET_API(api, ct_thread_a0);
    CETECH_GET_API(api, ct_vio_a0);
    CETECH_GET_API(api, ct_hash_a0);
    CETECH_GET_API(api, ct_ydb_a0);
    CETECH_GET_API(api, ct_yng_a0);

    _G = (struct G) {};

    _G.package_typel = ct_hash_a0.id64_from_str("package");

    ct_resource_a0.compiler_register(_G.package_typel, _package_compiler, true);


    return 1;
}

void package_shutdown() {

}


void package_task(void *data) {
    struct package_task_data *task_data = (package_task_data *) data;
    struct package_resource *package = (package_resource *) ct_resource_a0.get(
            _G.package_typel,
            task_data->name);

    const uint32_t task_count = package->type_count;
    for (uint32_t j = 0; j < task_count; ++j) {
        ct_resource_a0.load_now(package_type(package)[j],
                                &package_name(package)[package_offset(
                                        package)[j]],
                                package_name_count(package)[j]);
    }

    CEL_FREE(ct_memory_a0.main_allocator(), task_data);
}

void package_load(uint64_t name) {

    struct package_task_data *task_data =
            CEL_ALLOCATE(ct_memory_a0.main_allocator(),
                         struct package_task_data,
                         sizeof(struct package_task_data));

    task_data->name = name;

    ct_task_item item = {
            .name = "package_task",
            .work = package_task,
            .data = task_data,
            .affinity = TASK_AFFINITY_NONE
    };

    ct_task_a0.add(&item, 1);
}

void package_unload(uint64_t name) {
    package_resource *package = (package_resource *) ct_resource_a0.get(
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
    const uint64_t package_type = ct_hash_a0.id64_from_str("package");
    package_resource *package = (package_resource *) ct_resource_a0.get(
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