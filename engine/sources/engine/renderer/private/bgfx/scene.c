//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <celib/string/stringid.h>
#include <engine/world/scenegraph.h>
#include "celib/filesystem/path.h"
#include "celib/filesystem/vio.h"
#include <engine/memory/memsys.h>
#include <engine/plugin/plugin.h>

#include "engine/resource/types.h"
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

void *scene_resource_loader(struct vio *input,
                            struct cel_allocator *allocator) {
    const i64 size = cel_vio_size(input);
    char *data = CEL_ALLOCATE(allocator, char, size);
    cel_vio_read(input, data, 1, size);

    return data;
}

void scene_resource_unloader(void *new_data,
                             struct cel_allocator *allocator) {
    CEL_DEALLOCATE(allocator, new_data);
}

void scene_resource_online(stringid64_t name,
                           void *data) {
    struct scene_blob *resource = data;

    stringid64_t *geom_name = scene_blob_geom_name(resource);
    u32 *ib_offset = scene_blob_ib_offset(resource);
    u32 *vb_offset = scene_blob_vb_offset(resource);
    bgfx_vertex_decl_t *vb_decl = scene_blob_vb_decl(resource);
    u32 *ib_size = scene_blob_ib_size(resource);
    u32 *vb_size = scene_blob_vb_size(resource);
    u32 *ib = scene_blob_ib(resource);
    u8 *vb = scene_blob_vb(resource);

    struct scene_instance instance = {0};
    _init_scene_instance(&instance);

    for (int i = 0; i < resource->geom_count; ++i) {
        bgfx_vertex_buffer_handle_t bvb = bgfx_create_vertex_buffer(
                bgfx_make_ref((const void *) &vb[vb_offset[i]], vb_size[i]),
                &vb_decl[i], BGFX_BUFFER_NONE);

        bgfx_index_buffer_handle_t bib = bgfx_create_index_buffer(
                bgfx_make_ref((const void *) &ib[ib_offset[i]],
                              sizeof(u32) * ib_size[i]), BGFX_BUFFER_INDEX32);

        u32 idx = ARRAY_SIZE(&instance.vb);
        MAP_SET(u8, &instance.geom_map, geom_name[i].id, idx);

        ARRAY_PUSH_BACK(u32, &instance.size, ib_size[i]);
        ARRAY_PUSH_BACK(bgfx_vertex_buffer_handle_t, &instance.vb, bvb);
        ARRAY_PUSH_BACK(bgfx_index_buffer_handle_t, &instance.ib, bib);
    }

    MAP_SET(scene_instance, &_G.scene_instance, name.id, instance);
}

static const bgfx_texture_handle_t null_texture = {0};

void scene_resource_offline(stringid64_t name,
                            void *data) {
    struct scene_instance instance = MAP_GET(scene_instance, &_G.scene_instance,
                                             name.id,
                                             (struct scene_instance) {0});
    _destroy_scene_instance(&instance);
    MAP_REMOVE(scene_instance, &_G.scene_instance, name.id);
}

void *scene_resource_reloader(stringid64_t name,
                              void *old_data,
                              void *new_data,
                              struct cel_allocator *allocator) {
    scene_resource_offline(name, old_data);
    scene_resource_online(name, new_data);

    CEL_DEALLOCATE(allocator, old_data);

    return new_data;
}

static const resource_callbacks_t scene_resource_callback = {
        .loader = scene_resource_loader,
        .unloader =scene_resource_unloader,
        .online =scene_resource_online,
        .offline =scene_resource_offline,
        .reloader = scene_resource_reloader
};


//==============================================================================
// Interface
//==============================================================================

int scene_resource_init() {
    _G = (struct G) {0};

    MemSysApiV0 = *(struct MemSysApiV0 *) plugin_get_engine_api(MEMORY_API_ID,
                                                                0);
    ResourceApiV0 = *(struct ResourceApiV0 *) plugin_get_engine_api(
            RESOURCE_API_ID, 0);
    SceneGprahApiV0 = *(struct SceneGprahApiV0 *) plugin_get_engine_api(
            SCENEGRAPH_API_ID, 0);

    _G.type = stringid64_from_string("scene");

    MAP_INIT(scene_instance, &_G.scene_instance, MemSysApiV0.main_allocator());

    ResourceApiV0.compiler_register(_G.type, _scene_resource_compiler);
    ResourceApiV0.register_type(_G.type, scene_resource_callback);

    return 1;
}

void scene_resource_shutdown() {
    MAP_DESTROY(scene_instance, &_G.scene_instance);
    _G = (struct G) {0};
}

void scene_resource_submit(stringid64_t scene,
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