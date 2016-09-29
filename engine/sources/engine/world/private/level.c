//==============================================================================
// Includes
//==============================================================================

#include <engine/world/world.h>
#include <celib/os/vio.h>
#include <celib/stringid/types.h>
#include <engine/core/resource.h>
#include <celib/stringid/stringid.h>
#include <engine/develop/resource_compiler.h>
#include <engine/world/unit.h>
#include <engine/core/entcom.h>
#include <engine/world/transform.h>
#include <celib/math/quatf.h>
#include "engine/core/memory.h"
#include "celib/containers/map.h"

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
    MAP_T(entity_t) spawned_entity;
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
    MAP_INIT(entity_t, &instance->spawned_entity, memsys_main_allocator());
}

void _destroy_level_instance(struct level_instance *instance) {
    MAP_DESTROY(entity_t, &instance->spawned_entity);
}


level_t _new_level(entity_t level_entity) {
    u32 idx = ARRAY_SIZE(&_G.level_instance);

    ARRAY_PUSH_BACK(level_instance, &_G.level_instance, (struct level_instance) {0});

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
                            struct allocator *allocator) {
    const i64 size = vio_size(input);
    char *data = CE_ALLOCATE(allocator, char, size);
    vio_read(input, data, 1, size);

    return data;
}

void level_resource_unloader(void *new_data,
                             struct allocator *allocator) {
    CE_DEALLOCATE(allocator, new_data);
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
                              struct allocator *allocator) {
    level_resource_offline(name, old_data);
    level_resource_online(name, new_data);

    CE_DEALLOCATE(allocator, old_data);

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
    yaml_as_string(key, name, CE_ARRAY_LEN(name));
    ARRAY_PUSH_BACK(stringid64_t, data->id, stringid64_from_string(name));
    ARRAY_PUSH_BACK(u32, data->offset, unit_compiler_ent_counter(data->output));

    unit_compiler_compile_unit(data->output, value, data->filename, data->capi);
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

    char source_data[vio_size(source_vio) + 1];
    memory_set(source_data, 0, vio_size(source_vio) + 1);
    vio_read(source_vio, source_data, sizeof(char), vio_size(source_vio));

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

    yaml_node_t units = yaml_get_node(root, "units");

    ARRAY_T(stringid64_t) id;
    ARRAY_T(u32) offset;
    ARRAY_T(u8) data;

    ARRAY_INIT(stringid64_t, &id, memsys_main_allocator());
    ARRAY_INIT(u32, &offset, memsys_main_allocator());
    ARRAY_INIT(u8, &data, memsys_main_allocator());

    struct entity_compile_output *output = unit_compiler_create_output();


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

    unit_compiler_write_to_build(output, unit_data.data);

    vio_write(build_vio, &res, sizeof(struct level_blob), 1);
    vio_write(build_vio, &ARRAY_AT(&id, 0), sizeof(stringid64_t), ARRAY_SIZE(&id));
    vio_write(build_vio, &ARRAY_AT(&offset, 0), sizeof(u32), ARRAY_SIZE(&offset));
    vio_write(build_vio, &ARRAY_AT(&data, 0), sizeof(u8), ARRAY_SIZE(&data));

    ARRAY_DESTROY(stringid64_t, &id);
    ARRAY_DESTROY(u32, &offset);
    ARRAY_DESTROY(u8, &data);

    unit_compiler_destroy_output(output);

    return 1;
}


//==============================================================================
// Public interface
//==============================================================================

int level_init(int stage) {
    if (stage == 0) {
        return 1;
    }

    _G = (struct G) {0};
    _G.level_type = stringid64_from_string("level");

    ARRAY_INIT(level_instance, &_G.level_instance, memsys_main_allocator());

    resource_register_type(_G.level_type, _level_resource_defs);
    resource_compiler_register(_G.level_type, _level_resource_compiler);

    return 1;
}

void level_shutdown() {
    ARRAY_DESTROY(level_instance, &_G.level_instance);
    _G = (struct G) {0};
}

level_t world_load_level(world_t world,
                         stringid64_t name) {
    struct level_blob *res = resource_get(_G.level_type, name);

    stringid64_t *id = level_blob_id(res);
    u32 *offset = level_blob_offset(res);
    u8 *data = level_blob_data(res);

    entity_t level_ent = entity_manager_create();
    transform_t t = transform_create(world, level_ent, (entity_t) {UINT32_MAX}, (vec3f_t) {0}, QUATF_IDENTITY,
                                     (vec3f_t) {{1.0f, 1.0f, 1.0f}});

    level_t level = _new_level(level_ent);
    struct level_instance *instance = _level_instance(level);

    ARRAY_T(entity_t) *spawned = unit_spawn_from_resource(world, data);

    for (int i = 0; i < res->units_count; ++i) {
        entity_t e = ARRAY_AT(spawned, offset[i]);
        MAP_SET(entity_t, &instance->spawned_entity, id[i].id, e);

        if (transform_has(world, e)) {
            transform_link(world, level_ent, e);
        }
    }

    return level;
}

entity_t level_unit_by_id(level_t level,
                          stringid64_t id) {
    struct level_instance *instance = _level_instance(level);
    return MAP_GET(entity_t, &instance->spawned_entity, id.id, (entity_t) {0});
}

entity_t level_unit(level_t level) {
    struct level_instance *instance = _level_instance(level);
    return instance->level_entity;
}
