//==============================================================================
// Includes
//==============================================================================

#include <engine/world/world.h>
#include <celib/filesystem/vio.h>
#include "engine/resource/types.h"
#include <engine/world/unit.h>
#include <engine/world/transform.h>
#include <celib/math/quatf.h>
#include <engine/memory/memsys.h>
#include <engine/module/module_api.h>

#include "engine/world/level.h"

IMPORT_API(EntComSystemApi, 0);
IMPORT_API(ResourceApi, 0);
IMPORT_API(TransformApi, 0);
IMPORT_API(MemSysApi, 0);
IMPORT_API(UnitApi, 0);

//==============================================================================
// Typedefs
//==============================================================================

ARRAY_T(world_t);
ARRAY_PROTOTYPE(world_callbacks_t);
ARRAY_PROTOTYPE(stringid64_t);

//==============================================================================
// Globals
//==============================================================================

struct level_instance {
    entity_t level_entity;
    MAP_T(entity_t) spawned_entity_map;
    ARRAY_T(entity_t) *spawned_entity;
};

ARRAY_PROTOTYPE_N(struct level_instance, level_instance);
MAP_PROTOTYPE_N(struct level_instance, level_instance);

#define _G WorldGlobals
static struct G {
    stringid64_t level_type;

    ARRAY_T(level_instance) level_instance;
} _G = {0};

void _init_level_instance(struct level_instance *instance,
                          entity_t level_entity) {
    instance->level_entity = level_entity;
    MAP_INIT(entity_t, &instance->spawned_entity_map,
             MemSysApiV0.main_allocator());
}

void _destroy_level_instance(struct level_instance *instance) {
    MAP_DESTROY(entity_t, &instance->spawned_entity_map);
}


level_t _new_level(entity_t level_entity) {
    u32 idx = ARRAY_SIZE(&_G.level_instance);

    ARRAY_PUSH_BACK(level_instance, &_G.level_instance,
                    (struct level_instance) {0});

    struct level_instance *instance = &ARRAY_AT(&_G.level_instance, idx);

    _init_level_instance(instance, level_entity);

    return (level_t) {.idx = idx};
}

struct level_instance *_level_instance(level_t level) {
    return &ARRAY_AT(&_G.level_instance, level.idx);
}

//==============================================================================
// Resource
//==============================================================================

void *level_resource_loader(struct vio *input,
                            struct cel_allocator *allocator) {
    const i64 size = cel_vio_size(input);
    char *data = CEL_ALLOCATE(allocator, char, size);
    cel_vio_read(input, data, 1, size);

    return data;
}

void level_resource_unloader(void *new_data,
                             struct cel_allocator *allocator) {
    CEL_DEALLOCATE(allocator, new_data);
}

void level_resource_online(stringid64_t name,
                           void *data) {
}

void level_resource_offline(stringid64_t name,
                            void *data) {
}

void *level_resource_reloader(stringid64_t name,
                              void *old_data,
                              void *new_data,
                              struct cel_allocator *allocator) {
    level_resource_offline(name, old_data);
    level_resource_online(name, new_data);

    CEL_DEALLOCATE(allocator, old_data);

    return new_data;
}

static const resource_callbacks_t _level_resource_defs = {
        .loader = level_resource_loader,
        .unloader = level_resource_unloader,
        .online = level_resource_online,
        .offline = level_resource_offline,
        .reloader = level_resource_reloader
};

struct foreach_units_data {
    const char *filename;
    struct compilator_api *capi;
    ARRAY_T(stringid64_t) *id;
    ARRAY_T(u32) *offset;
    ARRAY_T(u8) *data;
    struct entity_compile_output *output;
};

void forach_units_clb(yaml_node_t key,
                      yaml_node_t value,
                      void *_data) {
    struct foreach_units_data *data = _data;

    char name[128] = {0};
    yaml_as_string(key, name, CEL_ARRAY_LEN(name));
    ARRAY_PUSH_BACK(stringid64_t, data->id, stringid64_from_string(name));
    ARRAY_PUSH_BACK(u32, data->offset,
                    UnitApiV0.compiler_ent_counter(data->output));

    UnitApiV0.compiler_compile_unit(data->output, value, data->filename,
                                    data->capi);
}

struct level_blob {
    u32 units_count;
    // res  + 1             : stringid64_t names[units_count]
    // names  + units_count : u32 offset[units_count]
    // offset + units_count : u8 data[*]
};


#define level_blob_id(r) ((stringid64_t*) ((r) + 1))
#define level_blob_offset(r) ((u32*) ((level_blob_id(r)) + (r)->units_count))
#define level_blob_data(r)   ((u8*) ((level_blob_offset(r)) + (r)->units_count))

int _level_resource_compiler(const char *filename,
                             struct vio *source_vio,
                             struct vio *build_vio,
                             struct compilator_api *compilator_api) {

    char source_data[cel_vio_size(source_vio) + 1];
    memory_set(source_data, 0, cel_vio_size(source_vio) + 1);
    cel_vio_read(source_vio, source_data, sizeof(char),
                 cel_vio_size(source_vio));

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

    yaml_node_t units = yaml_get_node(root, "units");

    ARRAY_T(stringid64_t) id;
    ARRAY_T(u32) offset;
    ARRAY_T(u8) data;

    ARRAY_INIT(stringid64_t, &id, MemSysApiV0.main_allocator());
    ARRAY_INIT(u32, &offset, MemSysApiV0.main_allocator());
    ARRAY_INIT(u8, &data, MemSysApiV0.main_allocator());

    struct entity_compile_output *output = UnitApiV0.compiler_create_output();


    struct foreach_units_data unit_data = {
            .id = &id,
            .offset = &offset,
            .data = &data,
            .capi = compilator_api,
            .filename = filename,
            .output = output
    };

    yaml_node_foreach_dict(units, forach_units_clb, &unit_data);

    struct level_blob res = {
            .units_count = ARRAY_SIZE(&id)
    };

    UnitApiV0.compiler_write_to_build(output, unit_data.data);

    cel_vio_write(build_vio, &res, sizeof(struct level_blob), 1);
    cel_vio_write(build_vio, &ARRAY_AT(&id, 0), sizeof(stringid64_t),
                  ARRAY_SIZE(&id));
    cel_vio_write(build_vio, &ARRAY_AT(&offset, 0), sizeof(u32),
                  ARRAY_SIZE(&offset));
    cel_vio_write(build_vio, &ARRAY_AT(&data, 0), sizeof(u8),
                  ARRAY_SIZE(&data));

    ARRAY_DESTROY(stringid64_t, &id);
    ARRAY_DESTROY(u32, &offset);
    ARRAY_DESTROY(u8, &data);

    UnitApiV0.compiler_destroy_output(output);

    return 1;
}


//==============================================================================
// Public interface
//==============================================================================

static void _init(get_api_fce_t get_engine_api) {
    INIT_API(EntComSystemApi, ENTCOM_API_ID, 0);
    INIT_API(MemSysApi, MEMORY_API_ID, 0);
    INIT_API(ResourceApi, RESOURCE_API_ID, 0);
    INIT_API(TransformApi, TRANSFORM_API_ID, 0);
    INIT_API(UnitApi, UNIT_API_ID, 0);

    _G = (struct G) {0};
    _G.level_type = stringid64_from_string("level");

    ARRAY_INIT(level_instance, &_G.level_instance,
               MemSysApiV0.main_allocator());

    ResourceApiV0.register_type(_G.level_type, _level_resource_defs);
    ResourceApiV0.compiler_register(_G.level_type, _level_resource_compiler);
}

static void _shutdown() {
    ARRAY_DESTROY(level_instance, &_G.level_instance);
    _G = (struct G) {0};
}


level_t world_load_level(world_t world,
                         stringid64_t name) {
    struct level_blob *res = ResourceApiV0.get(_G.level_type, name);

    stringid64_t *id = level_blob_id(res);
    u32 *offset = level_blob_offset(res);
    u8 *data = level_blob_data(res);

    entity_t level_ent = EntComSystemApiV0.entity_manager_create();
    transform_t t = TransformApiV0.create(world, level_ent,
                                          (entity_t) {UINT32_MAX},
                                          (cel_vec3f_t) {0}, QUATF_IDENTITY,
                                          (cel_vec3f_t) {{1.0f, 1.0f, 1.0f}});

    level_t level = _new_level(level_ent);
    struct level_instance *instance = _level_instance(level);

    ARRAY_T(entity_t) *spawned = UnitApiV0.spawn_from_resource(world, data);
    instance->spawned_entity = spawned;

    for (int i = 0; i < res->units_count; ++i) {
        entity_t e = ARRAY_AT(spawned, offset[i]);
        MAP_SET(entity_t, &instance->spawned_entity_map, id[i].id, e);

        if (TransformApiV0.has(world, e)) {
            TransformApiV0.link(world, level_ent, e);
        }
    }

    return level;
}

void level_destroy(world_t world,
                   level_t level) {
    struct level_instance *instance = _level_instance(level);

    UnitApiV0.destroy(world, &instance->spawned_entity->data[0], 1);
    EntComSystemApiV0.entity_manager_destroy(instance->level_entity);
}

entity_t level_unit_by_id(level_t level,
                          stringid64_t id) {
    struct level_instance *instance = _level_instance(level);
    return MAP_GET(entity_t, &instance->spawned_entity_map, id.id,
                   (entity_t) {0});
}

entity_t level_unit(level_t level) {
    struct level_instance *instance = _level_instance(level);
    return instance->level_entity;
}

void *level_get_module_api(int api,
                           int version) {

    switch (api) {
        case PLUGIN_EXPORT_API_ID:
            switch (version) {
                case 0: {
                    static struct module_api_v0 module = {0};

                    module.init = _init;
                    module.shutdown = _shutdown;

                    return &module;
                }

                default:
                    return NULL;
            };
        case LEVEL_API_ID:
            switch (version) {
                case 0: {
                    static struct LevelApiV0 api = {0};

                    api.load_level = world_load_level;
                    api.destroy = level_destroy;
                    api.unit_by_id = level_unit_by_id;
                    api.unit = level_unit;


                    return &api;
                }

                default:
                    return NULL;
            };

        default:
            return NULL;
    }
}