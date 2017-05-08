//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

#include <cetech/allocator.h>
#include <cetech/stringid.h>
#include <cetech/task.h>
#include <cetech/vio.h>
#include <cetech/config.h>
#include <cetech/application.h>
#include <cetech/vio.h>
#include <cetech/resource.h>
#include <cetech/vio.h>
#include <cetech/yaml.h>
#include <cetech/thread.h>
#include <cetech/memory.h>
#include <cetech/module.h>
#include <cetech/array.inl>

#include "resource_package.h"


//==============================================================================
// Public interface
//==============================================================================

struct package_task_data {
    stringid64_t name;
};

#define _G PackageGlobals
struct G {
    stringid64_t package_typel;
} _G = {0};


IMPORT_API(memory_api_v0);
IMPORT_API(resource_api_v0);
IMPORT_API(task_api_v0);
IMPORT_API(thread_api_v0);

//==============================================================================
// Resource compiler
//==============================================================================

ARRAY_PROTOTYPE(stringid64_t);

struct package_compile_data {
    ARRAY_T(stringid64_t) types;
    ARRAY_T(stringid64_t) name;
    ARRAY_T(uint32_t) name_count;
    ARRAY_T(uint32_t) offset;
};

void forach_clb(yaml_node_t key,
                yaml_node_t value,
                void *data) {
    struct package_compile_data *compile_data = data;

    char type_str[128] = {0};
    char name_str[128] = {0};

    yaml_as_string(key, type_str, CETECH_ARRAY_LEN(type_str));

    ARRAY_PUSH_BACK(stringid64_t, &compile_data->types,
                    stringid64_from_string(type_str));
    ARRAY_PUSH_BACK(uint32_t, &compile_data->offset,
                    ARRAY_SIZE(&compile_data->name));

    const size_t name_count = yaml_node_size(value);
    ARRAY_PUSH_BACK(uint32_t, &compile_data->name_count, name_count);

    for (int i = 0; i < name_count; ++i) {
        yaml_node_t name_node = yaml_get_seq_node(value, i);
        yaml_as_string(name_node, name_str, CETECH_ARRAY_LEN(name_str));

        ARRAY_PUSH_BACK(stringid64_t, &compile_data->name,
                        stringid64_from_string(name_str));

        yaml_node_free(name_node);
    }
}

int _package_compiler(const char *filename,
                      struct vio *source_vio,
                      struct vio *build_vio,
                      struct compilator_api *compilator_api) {

    char source_data[vio_size(source_vio) + 1];
    memory_set(source_data, 0, vio_size(source_vio) + 1);
    vio_read(source_vio, source_data, sizeof(char),
                 vio_size(source_vio));

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

    struct package_compile_data compile_data = {0};
    ARRAY_INIT(stringid64_t, &compile_data.types, memory_api_v0.main_allocator());
    ARRAY_INIT(stringid64_t, &compile_data.name, memory_api_v0.main_allocator());
    ARRAY_INIT(uint32_t, &compile_data.offset, memory_api_v0.main_allocator());
    ARRAY_INIT(uint32_t, &compile_data.name_count, memory_api_v0.main_allocator());

    yaml_node_foreach_dict(root, forach_clb, &compile_data);

    struct package_resource resource = {0};
    resource.type_count = ARRAY_SIZE(&compile_data.types);
    resource.type_offset = sizeof(resource);
    resource.name_count_offset = resource.type_offset + (sizeof(stringid64_t) *
                                                         ARRAY_SIZE(
                                                                 &compile_data.types));
    resource.name_offset = resource.name_count_offset +
                           (sizeof(uint32_t) * ARRAY_SIZE(&compile_data.name_count));
    resource.offset_offset = resource.name_offset + (sizeof(stringid64_t) *
                                                     ARRAY_SIZE(
                                                             &compile_data.name));

    vio_write(build_vio, &resource, sizeof(resource), 1);
    vio_write(build_vio, ARRAY_BEGIN(&compile_data.types),
                  sizeof(stringid64_t), ARRAY_SIZE(&compile_data.types));
    vio_write(build_vio, ARRAY_BEGIN(&compile_data.name_count), sizeof(uint32_t),
                  ARRAY_SIZE(&compile_data.name_count));
    vio_write(build_vio, ARRAY_BEGIN(&compile_data.name),
                  sizeof(stringid64_t), ARRAY_SIZE(&compile_data.name));
    vio_write(build_vio, ARRAY_BEGIN(&compile_data.offset), sizeof(uint32_t),
                  ARRAY_SIZE(&compile_data.offset));

    ARRAY_DESTROY(stringid64_t, &compile_data.types);
    ARRAY_DESTROY(stringid64_t, &compile_data.name);
    ARRAY_DESTROY(uint32_t, &compile_data.offset);
    ARRAY_DESTROY(uint32_t, &compile_data.name_count);

    return 1;
}

int package_init(get_api_fce_t get_engine_api) {
    INIT_API(get_engine_api, memory_api_v0, MEMORY_API_ID);
    INIT_API(get_engine_api, resource_api_v0, RESOURCE_API_ID);
    INIT_API(get_engine_api, task_api_v0, TASK_API_ID);
    INIT_API(get_engine_api, thread_api_v0, OS_THREAD_API_ID);

    _G = (struct G) {0};

    _G.package_typel = stringid64_from_string("package");

#ifdef CETECH_CAN_COMPILE
    resource_api_v0.compiler_register(_G.package_typel, _package_compiler);
#endif

    return 1;
}

void package_shutdown() {

}


void package_task(void *data) {
    struct package_task_data *task_data = data;
    struct package_resource *package = resource_api_v0.get(_G.package_typel,
                                                         task_data->name);

    const uint32_t task_count = package->type_count;
    for (int j = 0; j < task_count; ++j) {
        resource_api_v0.load_now(package_type(package)[j],
                               &package_name(package)[package_offset(
                                       package)[j]],
                               package_name_count(package)[j]);
    }
}

void package_load(stringid64_t name) {

    struct package_task_data *task_data =
    CETECH_ALLOCATE(memory_api_v0.main_allocator(), struct package_task_data, 1);

    task_data->name = name;

    struct task_item item = {
            .name = "package_task",
            .work = package_task,
            .data = task_data,
            .affinity = TASK_AFFINITY_NONE
    };

    task_api_v0.add(&item, 1);
}

void package_unload(stringid64_t name) {
    struct package_resource *package = resource_api_v0.get(_G.package_typel,
                                                         name);

    const uint32_t task_count = package->type_count;
    for (int j = 0; j < task_count; ++j) {
        resource_api_v0.unload(package_type(package)[j],
                             &package_name(package)[package_offset(package)[j]],
                             package_name_count(package)[j]);
    }
}

int package_is_loaded(stringid64_t name) {
    const stringid64_t package_type = stringid64_from_string("package");

    struct package_resource *package = resource_api_v0.get(package_type, name);

    const uint32_t task_count = package->type_count;
    for (int i = 0; i < task_count; ++i) {
        if (!resource_api_v0.can_get_all(package_type(package)[i],
                                       &package_name(package)[package_offset(
                                               package)[i]],
                                       package_name_count(package)[i])) {
            return 0;
        }
    }

    return 1;
}

void package_flush(stringid64_t name) {
    while (!package_is_loaded(name)) {
        if (!task_api_v0.do_work()) {
            thread_api_v0.yield();
        }
    }
}