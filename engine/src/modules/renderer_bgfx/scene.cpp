//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <cetech/celib/allocator.h>
#include <cetech/celib/map.inl>
#include <cetech/celib/map2.inl>

#include <cetech/core/hash.h>
#include <cetech/core/memory.h>
#include <cetech/core/module.h>
#include <cetech/core/api.h>
#include <cetech/core/path.h>
#include <cetech/core/vio.h>

#include <cetech/modules/resource.h>
#include <cetech/modules/entity.h>
#include <cetech/modules/world.h>
#include "cetech/modules/scenegraph.h"

#include "scene_blob.h"

using namespace cetech;

//==============================================================================
// Structs
//==============================================================================

IMPORT_API(memory_api_v0);
IMPORT_API(resource_api_v0);
IMPORT_API(scenegprah_api_v0);
IMPORT_API(path_v0);
IMPORT_API(vio_api_v0);
IMPORT_API(hash_api_v0);

//ARRAY_PROTOTYPE(bgfx_texture_handle_t)
//
//ARRAY_PROTOTYPE(bgfx_vertex_buffer_handle_t);
//ARRAY_PROTOTYPE(bgfx_index_buffer_handle_t);
//ARRAY_PROTOTYPE(mat44f_t);
//
//MAP_PROTOTYPE(bgfx_texture_handle_t)

struct scene_instance {
    Map<uint8_t> geom_map;
    Array<uint32_t> size;
    Array<bgfx_vertex_buffer_handle_t> vb;
    Array<bgfx_index_buffer_handle_t> ib;
};


//==============================================================================
// GLobals
//==============================================================================

static struct SceneResourceGlobals {
    uint64_t type;
    Map<uint32_t> scene_instance_map;
    Array<scene_instance> scene_instance_array;
} _G = {0};

struct scene_instance *_init_scene_instance(uint64_t scene) {

    uint32_t idx = array::size(_G.scene_instance_array);
    array::push_back(_G.scene_instance_array, {0});

    scene_instance *instance = &_G.scene_instance_array[idx];

    instance->geom_map.init(memory_api_v0.main_allocator());
    instance->size.init(memory_api_v0.main_allocator());
    instance->vb.init(memory_api_v0.main_allocator());
    instance->ib.init(memory_api_v0.main_allocator());

    map::set(_G.scene_instance_map, scene, idx);

    return instance;
}

void _destroy_scene_instance(uint64_t scene) {
    uint32_t idx = map::get(_G.scene_instance_map, scene, UINT32_MAX);
    scene_instance *instance = &_G.scene_instance_array[idx];

    instance->geom_map.destroy();
    instance->size.destroy();
    instance->vb.destroy();
    instance->ib.destroy();

    uint32_t size = array::size(_G.scene_instance_array);

    _G.scene_instance_array[idx] = _G.scene_instance_array[size - 1];
    array::pop_back(_G.scene_instance_array);
    map::remove(_G.scene_instance_map, scene);
}

struct scene_instance *_get_scene_instance(uint64_t scene) {
    uint32_t idx = map::get(_G.scene_instance_map, scene, UINT32_MAX);

    if(idx == UINT32_MAX){
        return NULL;
    }

    return &_G.scene_instance_array[idx];
}

//==============================================================================
// Compiler private
//==============================================================================
#ifdef CETECH_CAN_COMPILE

#include "scene_compiler.h"

#endif

//==============================================================================
// Resource
//==============================================================================
#include "scene_resource.h"

//==============================================================================
// Interface
//==============================================================================

int scene_init(struct api_v0 *api) {
    GET_API(api, memory_api_v0);
    GET_API(api, resource_api_v0);
    GET_API(api, scenegprah_api_v0);
    GET_API(api, path_v0);
    GET_API(api, vio_api_v0);
    GET_API(api, hash_api_v0);

    _G = {0};

    _G.type = hash_api_v0.id64_from_str("scene");

    _G.scene_instance_array.init(memory_api_v0.main_allocator());
    _G.scene_instance_map.init(memory_api_v0.main_allocator());

    resource_api_v0.register_type(_G.type, scene_resource_callback);

#ifdef CETECH_CAN_COMPILE
    resource_api_v0.compiler_register(_G.type, _scene_resource_compiler);
#endif

    return 1;
}

void scene_shutdown() {
    _G = {0};
}

void scene_submit(uint64_t scene,
                  uint64_t geom_name) {
    resource_api_v0.get(_G.type, scene);

    scene_instance *instance = _get_scene_instance(scene);

    if (instance == NULL) {
        return;
    }

    uint8_t idx = map::get<uint8_t>(instance->geom_map, geom_name, UINT8_MAX);

    if (idx == UINT8_MAX) {
        return;
    }

    bgfx_set_vertex_buffer(instance->vb[idx], 0, instance->size[idx]);
    bgfx_set_index_buffer(instance->ib[idx], 0, instance->size[idx]);
}

void scene_create_graph(world_t world,
                        entity_t entity,
                        uint64_t scene) {
    struct scene_blob *res = (scene_blob *) resource_api_v0.get(_G.type, scene);

    uint64_t *node_name = scene_blob_node_name(res);
    uint32_t *node_parent = scene_blob_node_parent(res);
    mat44f_t *node_pose = scene_blob_node_pose(res);

    scenegprah_api_v0.create(world, entity, node_name, node_parent, node_pose,
                             res->node_count);
}

uint64_t scene_get_mesh_node(uint64_t scene,
                             uint64_t mesh) {
    struct scene_blob *res = (scene_blob *) resource_api_v0.get(_G.type, scene);

    uint64_t *geom_node = scene_blob_geom_node(res);
    uint64_t *geom_name = scene_blob_geom_name(res);

    for (int i = 0; i < res->geom_count; ++i) {
        if (geom_name[i] != mesh) {
            continue;
        }

        return geom_node[i];
    }

    return (uint64_t) {0};
}