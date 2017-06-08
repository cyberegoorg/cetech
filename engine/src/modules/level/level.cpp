//==============================================================================
// Includes
//==============================================================================

#include <cetech/celib/allocator.h>
#include <cetech/modules/world.h>
#include <cetech/core/config.h>
#include <cetech/modules/resource.h>
#include <cetech/core/vio.h>
#include <cetech/modules/entity.h>
#include <cetech/modules/transform.h>
#include <cetech/celib/quatf.inl>
#include <cetech/core/memory.h>
#include <cetech/core/module.h>

#include "cetech/modules/level.h"
#include <cetech/celib/map.inl>
#include <cetech/core/yaml.h>
#include "level_blob.h"
#include <cetech/core/hash.h>
#include <cetech/core/api.h>

IMPORT_API(entity_api_v0);
IMPORT_API(resource_api_v0);
IMPORT_API(transform_api_v0);
IMPORT_API(memory_api_v0);
IMPORT_API(vio_api_v0);
IMPORT_API(hash_api_v0);
IMPORT_API(blob_api_v0);

//==============================================================================
// Typedefs
//==============================================================================

ARRAY_T(world_t);
ARRAY_PROTOTYPE(world_callbacks_t);
ARRAY_PROTOTYPE(entity_t);
MAP_PROTOTYPE(entity_t);

//==============================================================================
// Globals
//==============================================================================

namespace {
    struct level_instance {
        entity_t level_entity;
        MAP_T(entity_t) spawned_entity_map;
        entity_t *spawned_entity;
        uint32_t spawned_entity_count;
    };

    ARRAY_PROTOTYPE_N(struct level_instance, level_instance);
    MAP_PROTOTYPE_N(struct level_instance, level_instance);

    static struct LevelGlobals {
        uint64_t level_type;

        ARRAY_T(level_instance) level_instance;
    } _G = {0};

    void _init_level_instance(struct level_instance *instance,
                              entity_t level_entity) {
        instance->level_entity = level_entity;
        MAP_INIT(entity_t, &instance->spawned_entity_map,
                 memory_api_v0.main_allocator());
    }

    void _destroy_level_instance(struct level_instance *instance) {
        MAP_DESTROY(entity_t, &instance->spawned_entity_map);
    }


    level_t _new_level(entity_t level_entity) {
        uint32_t idx = ARRAY_SIZE(&_G.level_instance);

        ARRAY_PUSH_BACK(level_instance, &_G.level_instance,
                        (struct level_instance) {0});

        struct level_instance *instance = &ARRAY_AT(&_G.level_instance, idx);

        _init_level_instance(instance, level_entity);

        return (level_t) {.idx = idx};
    }

    struct level_instance *_level_instance(level_t level) {
        return &ARRAY_AT(&_G.level_instance, level.idx);
    }

};

//==============================================================================
// Resource
//==============================================================================

void *level_resource_loader(struct vio *input,
                            struct allocator *allocator) {
    const int64_t size = vio_api_v0.size(input);
    char *data = CETECH_ALLOCATE(allocator, char, size);
    vio_api_v0.read(input, data, 1, size);
    return data;
}

void level_resource_unloader(void *new_data,
                             struct allocator *allocator) {
    CETECH_DEALLOCATE(allocator, new_data);
}

void level_resource_online(uint64_t name,
                           void *data) {
}

void level_resource_offline(uint64_t name,
                            void *data) {
}

void *level_resource_reloader(uint64_t name,
                              void *old_data,
                              void *new_data,
                              struct allocator *allocator) {
    level_resource_offline(name, old_data);
    level_resource_online(name, new_data);

    CETECH_DEALLOCATE(allocator, old_data);

    return new_data;
}

static const resource_callbacks_t _level_resource_defs = {
        .loader = level_resource_loader,
        .unloader = level_resource_unloader,
        .online = level_resource_online,
        .offline = level_resource_offline,
        .reloader = level_resource_reloader
};

#ifdef CETECH_CAN_COMPILE
struct foreach_entities_data {
    const char *filename;
    struct compilator_api *capi;
    ARRAY_T(uint64_t) *id;
    ARRAY_T(uint32_t) *offset;
    blob_v0 *data;
    struct entity_compile_output *output;
};

void forach_entities_clb(yaml_node_t key,
                         yaml_node_t value,
                         void *_data) {
    struct foreach_entities_data *data = (foreach_entities_data *) _data;

    char name[128] = {0};
    yaml_as_string(key, name, CETECH_ARRAY_LEN(name));
    ARRAY_PUSH_BACK(uint64_t, data->id, hash_api_v0.id64_from_str(name));
    ARRAY_PUSH_BACK(uint32_t, data->offset,
                    entity_api_v0.compiler_ent_counter(data->output));

    entity_api_v0.compiler_compile_entity(data->output, value, data->filename,
                                          data->capi);
}

int _level_resource_compiler(const char *filename,
                             struct vio *source_vio,
                             struct vio *build_vio,
                             struct compilator_api *compilator_api) {

    char source_data[vio_api_v0.size(source_vio) + 1];
    memset(source_data, 0, vio_api_v0.size(source_vio) + 1);
    vio_api_v0.read(source_vio, source_data, sizeof(char),
                    vio_api_v0.size(source_vio));

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

    yaml_node_t entities = yaml_get_node(root, "entities");

    ARRAY_T(uint64_t) id;
    ARRAY_T(uint32_t) offset;
    blob_v0 *data = blob_api_v0.create(memory_api_v0.main_allocator());

    ARRAY_INIT(uint64_t, &id, memory_api_v0.main_allocator());
    ARRAY_INIT(uint32_t, &offset, memory_api_v0.main_allocator());

    struct entity_compile_output *output = entity_api_v0.compiler_create_output();
    struct foreach_entities_data entity_data = {
            .id = &id,
            .offset = &offset,
            .data = data,
            .capi = compilator_api,
            .filename = filename,
            .output = output
    };

    yaml_node_foreach_dict(entities, forach_entities_clb, &entity_data);

    struct level_blob res = {
            .entities_count = (uint32_t) ARRAY_SIZE(&id)
    };

    entity_api_v0.compiler_write_to_build(output, entity_data.data);

    vio_api_v0.write(build_vio, &res, sizeof(struct level_blob), 1);
    vio_api_v0.write(build_vio, &ARRAY_AT(&id, 0), sizeof(uint64_t),
                     ARRAY_SIZE(&id));
    vio_api_v0.write(build_vio, &ARRAY_AT(&offset, 0), sizeof(uint32_t),
                     ARRAY_SIZE(&offset));

    vio_api_v0.write(build_vio, data->data(data->inst), sizeof(uint8_t),
                     data->size(data->inst));

    ARRAY_DESTROY(uint64_t, &id);
    ARRAY_DESTROY(uint32_t, &offset);

    blob_api_v0.destroy(data);
    entity_api_v0.compiler_destroy_output(output);

    return 1;
}

#endif

//==============================================================================
// Public interface
//==============================================================================

namespace level {

    level_t load(world_t world,
                 uint64_t name) {
        struct level_blob *res = (level_blob *) resource_api_v0.get(
                _G.level_type,
                name);

        uint64_t *id = level_blob_names(res);
        uint32_t *offset = level_blob_offset(res);
        uint8_t *data = level_blob_data(res);

        entity_t level_ent = entity_api_v0.create();
        transform_t t = transform_api_v0.create(world, level_ent,
                                                (entity_t) {UINT32_MAX},
                                                (vec3f_t) {0}, QUATF_IDENTITY,
                                                (vec3f_t) {{1.0f, 1.0f, 1.0f}});

        level_t level = _new_level(level_ent);
        struct level_instance *instance = _level_instance(level);

        entity_api_v0.spawn_from_resource(world, data, &instance->spawned_entity, &instance->spawned_entity_count);

        for (int i = 0; i < res->entities_count; ++i) {
            entity_t e = instance->spawned_entity[offset[i]];
            MAP_SET(entity_t, &instance->spawned_entity_map, id[i], e);

            if (transform_api_v0.has(world, e)) {
                transform_api_v0.link(world, level_ent, e);
            }
        }

        return level;
    }

    void destroy(world_t world,
                 level_t level) {
        struct level_instance *instance = _level_instance(level);

        entity_api_v0.destroy(world, &instance->spawned_entity[0], 1);
        entity_api_v0.destroy(world, &instance->level_entity, 1);
    }

    entity_t entity_by_id(level_t level,
                          uint64_t id) {
        struct level_instance *instance = _level_instance(level);
        return MAP_GET(entity_t, &instance->spawned_entity_map, id,
                       (entity_t) {0});
    }

    entity_t entity(level_t level) {
        struct level_instance *instance = _level_instance(level);
        return instance->level_entity;
    }
}

namespace level_module {
    static struct level_api_v0 _api = {
            .load_level = level::load,
            .destroy = level::destroy,
            .entity_by_id = level::entity_by_id,
            .entity = level::entity
    };

    void _init_api(struct api_v0 *api) {
        api->register_api("level_api_v0", &_api);
    }


    void _init(struct api_v0 *api) {
        GET_API(api, entity_api_v0);
        GET_API(api, memory_api_v0);
        GET_API(api, resource_api_v0);
        GET_API(api, transform_api_v0);
        GET_API(api, vio_api_v0);
        GET_API(api, hash_api_v0);
        GET_API(api, blob_api_v0);


        _G = {0};

        _G.level_type = hash_api_v0.id64_from_str("level");

        ARRAY_INIT(level_instance, &_G.level_instance,
                   memory_api_v0.main_allocator());

        resource_api_v0.register_type(_G.level_type, _level_resource_defs);

#ifdef CETECH_CAN_COMPILE
        resource_api_v0.compiler_register(_G.level_type,
                                          _level_resource_compiler);
#endif

    }

    void _shutdown() {
        ARRAY_DESTROY(level_instance, &_G.level_instance);

        _G = {0};
    }


    extern "C" void *level_get_module_api(int api) {

        switch (api) {
            case PLUGIN_EXPORT_API_ID: {
                static struct module_export_api_v0 module = {0};

                module.init = _init;
                module.init_api = _init_api;
                module.shutdown = _shutdown;

                return &module;
            }

            default:
                return NULL;
        }
    }
}