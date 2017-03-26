//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <include/assimp/cimport.h>

#include <celib/string/stringid.h>
#include <engine/world/scenegraph.h>
#include "celib/filesystem/vio.h"
#include <engine/memory/api.h>
#include <engine/module/module.h>

#include "engine/resource/api.h"
#include "scene_blob.h"

//==============================================================================
// Structs
//==============================================================================

static struct MemSysApiV0 MemSysApiV0;
static struct ResourceApiV0 ResourceApiV0;
static struct SceneGprahApiV0 SceneGprahApiV0;

ARRAY_PROTOTYPE(bgfx_texture_handle_t)

ARRAY_PROTOTYPE(bgfx_vertex_buffer_handle_t);
ARRAY_PROTOTYPE(bgfx_index_buffer_handle_t);
ARRAY_PROTOTYPE(cel_mat44f_t);

MAP_PROTOTYPE(bgfx_texture_handle_t)

struct scene_instance {
    MAP_T(u8) geom_map;
    ARRAY_T(u32) size;
    ARRAY_T(bgfx_vertex_buffer_handle_t) vb;
    ARRAY_T(bgfx_index_buffer_handle_t) ib;
};

void _init_scene_instance(struct scene_instance *instance) {
    MAP_INIT(u8, &instance->geom_map, MemSysApiV0.main_allocator());
    ARRAY_INIT(u32, &instance->size, MemSysApiV0.main_allocator());
    ARRAY_INIT(bgfx_vertex_buffer_handle_t, &instance->vb,
               MemSysApiV0.main_allocator());
    ARRAY_INIT(bgfx_index_buffer_handle_t, &instance->ib,
               MemSysApiV0.main_allocator());
}

void _destroy_scene_instance(struct scene_instance *instance) {
    MAP_DESTROY(u8, &instance->geom_map);
    ARRAY_DESTROY(u32, &instance->size);
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
#include "scene_compiler.h"

//==============================================================================
// Resource
//==============================================================================
#include "scene_resource.h"

//==============================================================================
// Interface
//==============================================================================

int scene_init() {
    _G = (struct G) {0};

    MemSysApiV0 = *(struct MemSysApiV0 *) module_get_engine_api(MEMORY_API_ID,
                                                                0);
    ResourceApiV0 = *(struct ResourceApiV0 *) module_get_engine_api(
            RESOURCE_API_ID, 0);
    SceneGprahApiV0 = *(struct SceneGprahApiV0 *) module_get_engine_api(
            SCENEGRAPH_API_ID, 0);

    _G.type = stringid64_from_string("scene");

    MAP_INIT(scene_instance, &_G.scene_instance, MemSysApiV0.main_allocator());

    ResourceApiV0.compiler_register(_G.type, _scene_resource_compiler);
    ResourceApiV0.register_type(_G.type, scene_resource_callback);

    return 1;
}

void scene_shutdown() {
    MAP_DESTROY(scene_instance, &_G.scene_instance);
    _G = (struct G) {0};
}

void scene_submit(stringid64_t scene,
                  stringid64_t geom_name) {
    ResourceApiV0.get(_G.type, scene);
    struct scene_instance *instance = _get_scene_instance(scene);

    if (instance == NULL) {
        return;
    }

    u8 idx = MAP_GET(u8, &instance->geom_map, geom_name.id, UINT8_MAX);

    if (idx == UINT8_MAX) {
        return;
    }

    bgfx_set_vertex_buffer(instance->vb.data[idx], 0, instance->size.data[idx]);
    bgfx_set_index_buffer(instance->ib.data[idx], 0, instance->size.data[idx]);
}

void scene_create_graph(world_t world,
                        entity_t entity,
                        stringid64_t scene) {
    struct scene_blob *res = ResourceApiV0.get(_G.type, scene);

    stringid64_t *node_name = scene_blob_node_name(res);
    u32 *node_parent = scene_blob_node_parent(res);
    cel_mat44f_t *node_pose = scene_blob_node_pose(res);

    SceneGprahApiV0.create(world, entity, node_name, node_parent, node_pose,
                           res->node_count);
}

stringid64_t scene_get_mesh_node(stringid64_t scene,
                                 stringid64_t mesh) {
    struct scene_blob *res = ResourceApiV0.get(_G.type, scene);

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