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
#include <cetech/modules/yaml/yaml.h>

#include "celib/allocator.h"
#include "celib/array.inl"

#include "resource_package.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_task_a0);
CETECH_DECL_API(ct_thread_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hash_a0);

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

void forach_clb(yaml_node_t key,
                yaml_node_t value,
                void *data) {
    struct package_compile_data *compile_data = (package_compile_data *) data;

    char type_str[128] = {};
    char name_str[128] = {};

    yaml_as_string(key, type_str, CETECH_ARRAY_LEN(type_str));

    const uint32_t name_count = yaml_node_size(value);

    array::push_back(compile_data->types, ct_hash_a0.id64_from_str(type_str));
    array::push_back(compile_data->offset, array::size(compile_data->name));
    array::push_back(compile_data->name_count, name_count);

    for (uint32_t i = 0; i < name_count; ++i) {
        yaml_node_t name_node = yaml_get_seq_node(value, i);
        yaml_as_string(name_node, name_str, CETECH_ARRAY_LEN(name_str));

        array::push_back(compile_data->name,
                         ct_hash_a0.id64_from_str(name_str));

        yaml_node_free(name_node);
    }
}

int _package_compiler(const char *filename,
                      ct_vio *source_vio,
                      ct_vio *build_vio,
                      ct_compilator_api *compilator_api) {

    CEL_UNUSED(filename, compilator_api);

    char source_data[source_vio->size(source_vio->inst) + 1];
    memset(source_data, 0, source_vio->size(source_vio->inst) + 1);
    source_vio->read(source_vio->inst, source_data, sizeof(char),
                     source_vio->size(source_vio->inst));

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

    struct package_compile_data compile_data = {};

    compile_data.types.init(ct_memory_a0.main_allocator());
    compile_data.name.init(ct_memory_a0.main_allocator());
    compile_data.offset.init(ct_memory_a0.main_allocator());
    compile_data.name_count.init(ct_memory_a0.main_allocator());

    yaml_node_foreach_dict(root, forach_clb, &compile_data);

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

    build_vio->write(build_vio->inst, &resource, sizeof(resource), 1);

    build_vio->write(build_vio->inst, array::begin(compile_data.types),
                     sizeof(uint64_t), array::size(compile_data.types));

    build_vio->write(build_vio->inst, array::begin(compile_data.name_count),
                     sizeof(uint32_t), array::size(compile_data.name_count));

    build_vio->write(build_vio->inst, array::begin(compile_data.name),
                     sizeof(uint64_t), array::size(compile_data.name));

    build_vio->write(build_vio->inst, array::begin(compile_data.offset),
                     sizeof(uint32_t), array::size(compile_data.offset));

    return 1;
}

int package_init(ct_api_a0 *api) {
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_resource_a0);
    CETECH_GET_API(api, ct_task_a0);
    CETECH_GET_API(api, ct_thread_a0);
    CETECH_GET_API(api, ct_vio_a0);
    CETECH_GET_API(api, ct_hash_a0);

    _G = (struct G) {};

    _G.package_typel = ct_hash_a0.id64_from_str("package");

    ct_resource_a0.compiler_register(_G.package_typel, _package_compiler);


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