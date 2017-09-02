//==============================================================================
// Includes
//==============================================================================

#include <cetech/kernel/api_system.h>
#include <cetech/modules/entity/entity.h>
#include <cetech/kernel/resource.h>
#include <cetech/modules/transform/transform.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/vio.h>
#include <cetech/kernel/hashlib.h>
#include <cetech/modules/level/level.h>

#include <cetech/kernel/module.h>
#include <cetech/kernel/blob.h>
#include <cetech/kernel/ydb.h>
#include <cetech/kernel/macros.h>
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
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yamlng_a0);

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

namespace level_resource_compiler {
    struct foreach_entities_data {
        const char *filename;
        ct_compilator_api *capi;
        Array<uint64_t> *id;
        Array<uint32_t> *offset;
        ct_blob *data;
        ct_entity_compile_output *output;
    };


    int compiler(const char *filename,
                 ct_vio *source_vio,
                 ct_vio *build_vio,
                 ct_compilator_api *compilator_api) {
        CEL_UNUSED(source_vio);

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


        uint64_t tmp_keys = 0;

        uint64_t groups_keys[32] = {};
        uint32_t groups_keys_count = 0;

        ct_ydb_a0.get_map_keys(filename,
                               &tmp_keys,1,
                               groups_keys,CETECH_ARRAY_LEN(groups_keys),
                               &groups_keys_count);

        for (uint32_t i = 0; i < groups_keys_count; ++i) {
            uint64_t entities_keys[32] = {};
            uint32_t entities_keys_count = 0;

            ct_ydb_a0.get_map_keys(filename,
                                   &groups_keys[i], 1,
                                   entities_keys,CETECH_ARRAY_LEN(entities_keys),
                                   &entities_keys_count);

            for (uint32_t j = 0; j < entities_keys_count; ++j) {
                array::push_back(*entity_data.id, entities_keys[j]);
                array::push_back(*entity_data.offset,
                                 ct_entity_a0.compiler_ent_counter(entity_data.output));

                uint64_t keys[] = {
                        groups_keys[i],
                        entities_keys[j],
                };

                ct_entity_a0.compiler_compile_entity(
                        entity_data.output,
                        keys, 2,
                        filename,
                        entity_data.capi);
            }
        }


        level_blob::blob_t res = {
                .entities_count = (uint32_t) array::size(id)
        };

        ct_entity_a0.compiler_write_to_build(output, filename, entity_data.data);

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

        ct_resource_a0.compiler_register(_G.level_type,
                                         level_resource_compiler::compiler);

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
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_yamlng_a0);
        },
        {
            CEL_UNUSED(reload);
            level_module::_init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);

            level_module::_shutdown();

        }
)