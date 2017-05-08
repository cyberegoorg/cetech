//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <include/assimp/cimport.h>

#include <cetech/allocator.h>
#include <cetech/stringid.h>
#include <cetech/scenegraph.h>
#include "cetech/vio.h"
#include <cetech/memory.h>
#include <cetech/application.h>
#include <cetech/config.h>
#include <cetech/vio.h>
#include <cetech/resource.h>
#include <cetech/entity.h>
#include <cetech/world.h>

#include <cetech/module.h>

#include <cetech/resource.h>
#include <cetech/array.inl>
#include <cetech/map.inl>
#include "scene_blob.h"

//==============================================================================
// Structs
//==============================================================================

IMPORT_API(memory_api_v0);
IMPORT_API(resource_api_v0);
IMPORT_API(scenegprah_api_v0);

ARRAY_PROTOTYPE(bgfx_texture_handle_t)

ARRAY_PROTOTYPE(bgfx_vertex_buffer_handle_t);
ARRAY_PROTOTYPE(bgfx_index_buffer_handle_t);
ARRAY_PROTOTYPE(mat44f_t);

MAP_PROTOTYPE(bgfx_texture_handle_t)

struct scene_instance {
    MAP_T(uint8_t) geom_map;
    ARRAY_T(uint32_t) size;
    ARRAY_T(bgfx_vertex_buffer_handle_t) vb;
    ARRAY_T(bgfx_index_buffer_handle_t) ib;
};

void _init_scene_instance(struct scene_instance *instance) {
    MAP_INIT(uint8_t, &instance->geom_map, memory_api_v0.main_allocator());
    ARRAY_INIT(uint32_t, &instance->size, memory_api_v0.main_allocator());
    ARRAY_INIT(bgfx_vertex_buffer_handle_t, &instance->vb,
               memory_api_v0.main_allocator());
    ARRAY_INIT(bgfx_index_buffer_handle_t, &instance->ib,
               memory_api_v0.main_allocator());
}

void _destroy_scene_instance(struct scene_instance *instance) {
    MAP_DESTROY(uint8_t, &instance->geom_map);
    ARRAY_DESTROY(uint32_t, &instance->size);
    ARRAY_DESTROY(bgfx_vertex_buffer_handle_t, &instance->vb);
    ARRAY_DESTROY(bgfx_index_buffer_handle_t, &instance->ib);
}


ARRAY_PROTOTYPE_N(struct scene_instance, scene_instance);
MAP_PROTOTYPE_N(struct scene_instance, scene_instance);

//==============================================================================
// GLobals
//==============================================================================

#define _G SceneResourceGlobals
static struct G {
    stringid64_t type;
    MAP_T(scene_instance) scene_instance;
} _G = {0};


struct scene_instance *_get_scene_instance(stringid64_t scene) {
    return MAP_GET_PTR(scene_instance, &_G.scene_instance, scene.id);
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
#include "../../application/module.h"

//==============================================================================
// Interface
//==============================================================================

int scene_init() {
    _G = (struct G) {0};

    memory_api_v0 = *(struct memory_api_v0 *) module_get_engine_api(MEMORY_API_ID);
    resource_api_v0 = *(struct resource_api_v0 *) module_get_engine_api(
            RESOURCE_API_ID);
    scenegprah_api_v0 = *(struct scenegprah_api_v0 *) module_get_engine_api(
            SCENEGRAPH_API_ID);

    _G.type = stringid64_from_string("scene");

    MAP_INIT(scene_instance, &_G.scene_instance, memory_api_v0.main_allocator());

    resource_api_v0.register_type(_G.type, scene_resource_callback);

#ifdef CETECH_CAN_COMPILE
    resource_api_v0.compiler_register(_G.type, _scene_resource_compiler);
#endif

    return 1;
}

void scene_shutdown() {
    MAP_DESTROY(scene_instance, &_G.scene_instance);
    _G = (struct G) {0};
}

void scene_submit(stringid64_t scene,
                  stringid64_t geom_name) {
    resource_api_v0.get(_G.type, scene);
    struct scene_instance *instance = _get_scene_instance(scene);

    if (instance == NULL) {
        return;
    }

    uint8_t idx = MAP_GET(uint8_t, &instance->geom_map, geom_name.id, UINT8_MAX);

    if (idx == UINT8_MAX) {
        return;
    }

    bgfx_set_vertex_buffer(instance->vb.data[idx], 0, instance->size.data[idx]);
    bgfx_set_index_buffer(instance->ib.data[idx], 0, instance->size.data[idx]);
}

void scene_create_graph(world_t world,
                        entity_t entity,
                        stringid64_t scene) {
    struct scene_blob *res = resource_api_v0.get(_G.type, scene);

    stringid64_t *node_name = scene_blob_node_name(res);
    uint32_t *node_parent = scene_blob_node_parent(res);
    mat44f_t *node_pose = scene_blob_node_pose(res);

    scenegprah_api_v0.create(world, entity, node_name, node_parent, node_pose,
                           res->node_count);
}

stringid64_t scene_get_mesh_node(stringid64_t scene,
                                 stringid64_t mesh) {
    struct scene_blob *res = resource_api_v0.get(_G.type, scene);

    stringid64_t *geom_node = scene_blob_geom_node(res);
    stringid64_t *geom_name = scene_blob_geom_name(res);

    for (int i = 0; i < res->geom_count; ++i) {
        if (geom_name[i].id != mesh.id) {
            continue;
        }

        return geom_node[i];
    }

    return (stringid64_t) {0};
}