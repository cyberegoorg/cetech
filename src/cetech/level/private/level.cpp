//==============================================================================
// Includes
//==============================================================================

#include <cetech/api/api_system.h>
#include <cetech/entity/entity.h>
#include <cetech/resource/resource.h>
#include <cetech/transform/transform.h>
#include <cetech/memory/memory.h>
#include <cetech/os/vio.h>
#include <cetech/hashlib/hashlib.h>
#include <cetech/level/level.h>
#include <cetech/yaml/yaml.h>
#include <cetech/module/module.h>
#include <cetech/blob/blob.h>
#include <cetech/macros.h>
#include "celib/array.inl"
#include "celib/map.inl"

#include "level_blob.h"

using namespace celib;

CETECH_DECL_API(ct_entity_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_blob_a0);
CETECH_DECL_API(ct_world_a0);

//==============================================================================
// Globals
//==============================================================================

namespace {
    struct level_instance {
        ct_entity level_entity;
        Map<ct_entity> spawned_entity_map;
        ct_entity *spawned_entity;
        uint32_t spawned_entity_count;
    };


#define _G LevelGlobals
    static struct LevelGlobals {
        uint64_t level_type;
        Array<struct level_instance> level_instance;
    } LevelGlobals;

    void _init_level_instance(struct level_instance *instance,
                              ct_entity level_entity) {
        instance->level_entity = level_entity;
        instance->spawned_entity_map.init(ct_memory_a0.main_allocator());
    }

    void _destroy_level_instance(struct level_instance *instance) {
        instance->spawned_entity_map.destroy();
    }


    ct_level _new_level(ct_entity level_entity) {
        uint32_t idx = array::size(_G.level_instance);
        array::push_back(_G.level_instance, {});

        level_instance *instance = &_G.level_instance[idx];

        _init_level_instance(instance, level_entity);

        return (ct_level) {.idx = idx};
    }

    struct level_instance *get_level_instance(ct_level level) {
        return &_G.level_instance[level.idx];
    }

};

//==============================================================================
// Resource
//==============================================================================

namespace level_resource {
    void *loader(ct_vio *input,
                 cel_alloc *allocator) {
        const int64_t size = input->size(input->inst);
        char *data = CEL_ALLOCATE(allocator, char, size);
        input->read(input->inst, data, 1, size);
        return data;
    }

    void unloader(void *new_data,
                  cel_alloc *allocator) {
        CEL_FREE(allocator, new_data);
    }

    void online(uint64_t name,
                void *data) {
        CEL_UNUSED(data, name);
    }

    void resource_offline(uint64_t name,
                          void *data) {
        CEL_UNUSED(name, data);
    }

    void *resource_reloader(uint64_t name,
                            void *old_data,
                            void *new_data,
                            cel_alloc *allocator) {
        resource_offline(name, old_data);
        online(name, new_data);

        CEL_FREE(allocator, old_data);

        return new_data;
    }

    static const ct_resource_callbacks_t callback = {
            .loader = loader,
            .unloader = unloader,
            .online = online,
            .offline = resource_offline,
            .reloader = resource_reloader
    };
}

#ifdef CETECH_CAN_COMPILE
namespace level_resource_compiler {
    struct foreach_entities_data {
        const char *filename;
        ct_compilator_api *capi;
        Array<uint64_t> *id;
        Array<uint32_t> *offset;
        ct_blob *data;
        ct_entity_compile_output *output;
    };

    void forach_entities_clb(yaml_node_t key,
                             yaml_node_t value,
                             void *_data) {
        struct foreach_entities_data *data = (foreach_entities_data *) _data;


        char name[128] = {};
        yaml_as_string(key, name, CETECH_ARRAY_LEN(name));

        array::push_back(*data->id, ct_hash_a0.id64_from_str(name));
        array::push_back(*data->offset,
                         ct_entity_a0.compiler_ent_counter(data->output));

        ct_entity_a0.compiler_compile_entity(data->output, value,
                                             data->filename,
                                             data->capi);
    }

    int compiler(const char *filename,
                 ct_vio *source_vio,
                 ct_vio *build_vio,
                 ct_compilator_api *compilator_api) {

        char source_data[source_vio->size(source_vio->inst) + 1];
        memset(source_data, 0, source_vio->size(source_vio->inst) + 1);
        source_vio->read(source_vio->inst, source_data, sizeof(char),
                         source_vio->size(source_vio->inst));

        yaml_document_t h;
        yaml_node_t root = yaml_load_str(source_data, &h);

        yaml_node_t entities = yaml_get_node(root, "entities");

        Array<uint64_t> id(ct_memory_a0.main_allocator());
        Array<uint32_t> offset(ct_memory_a0.main_allocator());
        ct_blob *data = ct_blob_a0.create(ct_memory_a0.main_allocator());

        ct_entity_compile_output *output = ct_entity_a0.compiler_create_output();
        foreach_entities_data entity_data = {
                .id = &id,
                .offset = &offset,
                .data = data,
                .capi = compilator_api,
                .filename = filename,
                .output = output
        };

        yaml_node_foreach_dict(entities, forach_entities_clb, &entity_data);

        level_blob::blob_t res = {
                .entities_count = (uint32_t) array::size(id)
        };

        ct_entity_a0.compiler_write_to_build(output, entity_data.data);

        build_vio->write(build_vio->inst, &res, sizeof(level_blob::blob_t), 1);
        build_vio->write(build_vio->inst, array::begin(id), sizeof(uint64_t),
                         array::size(id));
        build_vio->write(build_vio->inst, array::begin(offset),
                         sizeof(uint32_t),
                         array::size(offset));
        build_vio->write(build_vio->inst, data->data(data->inst),
                         sizeof(uint8_t),
                         data->size(data->inst));

        ct_blob_a0.destroy(data);
        ct_entity_a0.compiler_destroy_output(output);

        return 1;
    }
}
#endif

//==============================================================================
// Public interface
//==============================================================================

namespace level {

    ct_level load(ct_world world,
                  uint64_t name) {

        auto res = level_blob::get(ct_resource_a0.get(_G.level_type, name));

        uint64_t *id = level_blob::names(res);
        uint32_t *offset = level_blob::offset(res);
        uint8_t *data = level_blob::data(res);

        ct_entity level_ent = ct_entity_a0.create();
        ct_transform_a0.create(world,
                               level_ent,
                               {UINT32_MAX},
                               (float[3]) {0.0f},
                               (float[4]) {0.0f, 0.0f, 0.0f, 1.0f},
                               (float[3]) {1.0f, 1.0f, 1.0f});

        ct_level level = _new_level(level_ent);
        struct level_instance *instance = get_level_instance(level);

        ct_entity_a0.spawn_from_resource(world, data,
                                         &instance->spawned_entity,
                                         &instance->spawned_entity_count);

        for (uint32_t i = 0; i < level_blob::entities_count(res); ++i) {
            ct_entity e = instance->spawned_entity[offset[i]];
            map::set(instance->spawned_entity_map, id[i], e);

            if (ct_transform_a0.has(world, e)) {
                ct_transform_a0.link(world, level_ent, e);
            }
        }

        return level;
    }

    void destroy(ct_world world,
                 ct_level level) {
        struct level_instance *instance = get_level_instance(level);

        ct_entity_a0.destroy(world, instance->spawned_entity,
                             instance->spawned_entity_count);
        ct_entity_a0.destroy(world, &instance->level_entity, 1);

        CEL_FREE(ct_memory_a0.main_allocator(),
                 instance->spawned_entity);

        _destroy_level_instance(instance);
    }

    ct_entity entity_by_id(ct_level level,
                           uint64_t id) {
        struct level_instance *instance = get_level_instance(level);
        return map::get(instance->spawned_entity_map, id, {});
    }

    ct_entity entity(ct_level level) {
        struct level_instance *instance = get_level_instance(level);
        return instance->level_entity;
    }
}


//==============================================================================
// Module interface
//==============================================================================

namespace level_module {
    static ct_level_a0 _api = {
            .load_level = level::load,
            .destroy = level::destroy,
            .entity_by_id = level::entity_by_id,
            .entity = level::entity
    };

    void _init_api(ct_api_a0 *api) {
        api->register_api("ct_level_a0", &_api);
    }


    void _init(ct_api_a0 *api) {
        _init_api(api);

        _G = {};

        _G.level_type = ct_hash_a0.id64_from_str("level");

        _G.level_instance.init(ct_memory_a0.main_allocator());

        ct_resource_a0.register_type(_G.level_type, level_resource::callback);

#ifdef CETECH_CAN_COMPILE
        ct_resource_a0.compiler_register(_G.level_type,
                                         level_resource_compiler::compiler);
#endif

    }

    void _shutdown() {
        _G.level_instance.destroy();
    }

}

CETECH_MODULE_DEF(
        level,
        {
            CETECH_GET_API(api, ct_entity_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_blob_a0);
            CETECH_GET_API(api, ct_world_a0);
        },
        {
            level_module::_init(api);
        },
        {
            CEL_UNUSED(api);

            level_module::_shutdown();

        }
)