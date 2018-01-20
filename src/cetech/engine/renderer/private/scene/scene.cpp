//==============================================================================
// Include
//==============================================================================

#include <cetech/core/module/module.h>
#include <cetech/engine/renderer/scene.h>
#include <cetech/core/containers/array.h>
#include <cetech/core/containers/hash.h>
#include "cetech/core/containers/map.inl"

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/engine/machine/machine.h"


#include "cetech/engine/resource/resource.h"
#include "cetech/engine/entity/entity.h"

#include "cetech/engine/scenegraph/scenegraph.h"

#include "scene_blob.h"
#include "cetech/core/os/path.h"
#include "cetech/core/os/thread.h"
#include "cetech/core/os/vio.h"

using namespace celib;

int scenecompiler_init(ct_api_a0 *api);

//==============================================================================
// Structs
//==============================================================================

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_scenegprah_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_thread_a0);
CETECH_DECL_API(ct_coredb_a0);


struct scene_instance {
    uint64_t scene;
    ct_hash_t geom_map;
    uint32_t *size;
    bgfx::VertexBufferHandle *vb;
    bgfx::IndexBufferHandle *ib;
};


//==============================================================================
// GLobals
//==============================================================================

#define _G SceneResourceGlobals
static struct _G {
    uint64_t type;
    ct_hash_t scene_instance_map;
    scene_instance *scene_instance_array;
    ct_alloc *allocator;
} _G;

static struct scene_instance *_init_scene_instance(uint64_t scene) {
    uint32_t idx = ct_array_size(_G.scene_instance_array);
    ct_array_push(_G.scene_instance_array, {}, _G.allocator);

    scene_instance *instance = &_G.scene_instance_array[idx];
    instance->scene = scene;

    ct_hash_add(&_G.scene_instance_map, scene, idx, _G.allocator);

    return instance;
}

static void _destroy_scene_instance(uint64_t scene) {
    uint32_t idx = (uint32_t) ct_hash_lookup(&_G.scene_instance_map, scene,
                                              UINT32_MAX);
    uint32_t size = ct_array_size(_G.scene_instance_array);

    scene_instance *instance = &_G.scene_instance_array[idx];
    scene_instance *last_instance = &_G.scene_instance_array[size - 1];

    ct_hash_free(&instance->geom_map, _G.allocator);

    ct_array_free(instance->size, _G.allocator);
    ct_array_free(instance->vb, _G.allocator);
    ct_array_free(instance->ib, _G.allocator);

    *instance = *last_instance;
    ct_hash_remove(&_G.scene_instance_map, scene);
    ct_hash_add(&_G.scene_instance_map, last_instance->scene, idx,
                 _G.allocator);
    ct_array_pop_back(_G.scene_instance_array);
}

static struct scene_instance *_get_scene_instance(uint64_t scene) {
    uint32_t idx = (uint32_t) ct_hash_lookup(&_G.scene_instance_map, scene,
                                              UINT32_MAX);

    if (idx == UINT32_MAX) {
        ct_resource_a0.get_obj(_G.type, scene);
        idx = (uint32_t) ct_hash_lookup(&_G.scene_instance_map, scene,
                                         UINT32_MAX);
    }

    return &_G.scene_instance_array[idx];
}

//==============================================================================
// Compiler private
//==============================================================================


//==============================================================================
// Resource
//==============================================================================

#define SCENE_PROP CT_ID64_0("scene")

static void online(uint64_t name,
                   struct ct_vio* input,struct ct_cdb_object_t* obj) {
    const uint64_t size = input->size(input);
    char *data = CT_ALLOC(_G.allocator, char, size);
    input->read(input, data, 1, size);

    ct_cdb_writer_t* writer = ct_coredb_a0.write_begin(obj);
    ct_coredb_a0.set_ptr(writer, SCENE_PROP, data);
    ct_coredb_a0.write_commit(writer);

    auto resource = scene_blob::get(data);

    bgfx::VertexDecl *vb_decl = scene_blob::vb_decl(resource);
    uint64_t *geom_name = scene_blob::geom_name(resource);
    uint32_t *ib_offset = scene_blob::ib_offset(resource);
    uint32_t *vb_offset = scene_blob::vb_offset(resource);
    uint32_t *ib_size = scene_blob::ib_size(resource);
    uint32_t *vb_size = scene_blob::vb_size(resource);
    uint32_t *ib = scene_blob::ib(resource);
    uint8_t *vb = scene_blob::vb(resource);

    scene_instance *instance = _init_scene_instance(name);


    for (uint32_t i = 0; i < resource->geom_count; ++i) {
        auto vb_mem = bgfx::makeRef((const void *) &vb[vb_offset[i]],
                                    vb_size[i]);

        auto ib_mem = bgfx::makeRef((const void *) &ib[ib_offset[i]],
                                    sizeof(uint32_t) * ib_size[i]);

        auto bv_handle = bgfx::createVertexBuffer(vb_mem, vb_decl[i],
                                                  BGFX_BUFFER_NONE);

        auto ib_handle = bgfx::createIndexBuffer(ib_mem,
                                                 BGFX_BUFFER_INDEX32);

        uint8_t idx = (uint8_t) ct_array_size(instance->vb);
        ct_hash_add(&instance->geom_map, geom_name[i], idx, _G.allocator);

        ct_array_push(instance->size, ib_size[i], _G.allocator);
        ct_array_push(instance->vb, bv_handle, _G.allocator);
        ct_array_push(instance->ib, ib_handle, _G.allocator);
    }
}

static void offline(uint64_t name,
                    struct ct_cdb_object_t* obj) {
    CT_UNUSED(obj);

    _destroy_scene_instance(name);
}

static const ct_resource_callbacks_t callback = {
        .online = online,
        .offline = offline,
};


//==============================================================================
// Interface
//==============================================================================
int sceneinit(ct_api_a0 *api) {
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_resource_a0);
    CETECH_GET_API(api, ct_scenegprah_a0);
    CETECH_GET_API(api, ct_path_a0);
    CETECH_GET_API(api, ct_vio_a0);
    CETECH_GET_API(api, ct_hash_a0);
    CETECH_GET_API(api, ct_thread_a0);

    _G = {
            .allocator=ct_memory_a0.main_allocator(),
            .type = _G.type = CT_ID64_0("scene"),

    };

    ct_resource_a0.register_type(_G.type, callback);

    scenecompiler_init(api);

    return 1;
}

static void shutdown() {
    ct_array_free(_G.scene_instance_array, _G.allocator);
    ct_hash_free(&_G.scene_instance_map, _G.allocator);
}

static void setVBIB(uint64_t scene,
                    uint64_t geom_name) {
    scene_instance *instance = _get_scene_instance(scene);

    if (instance == NULL) {
        return;
    }

    uint8_t idx = (uint8_t) ct_hash_lookup(&instance->geom_map, geom_name,
                                            UINT8_MAX);

    if (idx == UINT8_MAX) {
        return;
    }

    bgfx::setVertexBuffer(0, instance->vb[idx], 0, instance->size[idx]);
    bgfx::setIndexBuffer(instance->ib[idx], 0, instance->size[idx]);
}

static scene_blob::blob_t* resource_data(uint64_t name) {
    auto object = ct_resource_a0.get_obj(_G.type, name);
    return (scene_blob::blob_t*)(ct_coredb_a0.read_ptr(object, SCENE_PROP, NULL));
}

static void create_graph(ct_world world,
                         ct_entity entity,
                         uint64_t scene) {

    auto *res = resource_data(scene);

    uint64_t *node_name = scene_blob::node_name(res);
    uint32_t *node_parent = scene_blob::node_parent(res);
    float *node_pose = scene_blob::node_pose(res);

    ct_scenegprah_a0.create(world,
                            entity,
                            node_name,
                            node_parent,
                            node_pose,
                            res->node_count);
}

static uint64_t get_mesh_node(uint64_t scene,
                              uint64_t mesh) {
    auto *res = resource_data(scene);

    uint64_t *geom_name = scene_blob::geom_name(res);
    uint64_t *geom_node = scene_blob::geom_node(res);

    for (uint32_t i = 0; i < res->geom_count; ++i) {
        if (geom_name[i] != mesh) {
            continue;
        }

        return geom_node[i];
    }

    return 0;
}

static void get_all_geometries(uint64_t scene,
                               char **geometries,
                               uint32_t *count) {
    auto *res = resource_data(scene);

    *geometries = scene_blob::geom_str(res);
    *count = scene_blob::geom_count(res);
}

static void get_all_nodes(uint64_t scene,
                          char **geometries,
                          uint32_t *count) {
    auto *res = resource_data(scene);

    *geometries = scene_blob::node_str(res);
    *count = scene_blob::node_count(res);
}

static ct_scene_a0 scene_api = {
        .setVBIB = setVBIB,
        .create_graph =create_graph,
        .get_mesh_node =get_mesh_node,
        .get_all_geometries =get_all_geometries,
        .get_all_nodes = get_all_nodes,
};

static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_scene_a0", &scene_api);
}

CETECH_MODULE_DEF(
        scene,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_scenegprah_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_thread_a0);
            CETECH_GET_API(api, ct_coredb_a0);
        },
        {
            CT_UNUSED(reload);
            _init_api(api);
            sceneinit(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            shutdown();
        }
)