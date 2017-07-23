//==============================================================================
// Include
//==============================================================================

#include <celib/allocator.h>
#include <celib/map.inl>

#include <cetech/hash.h>
#include <cetech/memory.h>
#include <cetech/api_system.h>
#include <cetech/machine/machine.h>


#include <cetech/resource.h>
#include <cetech/entity.h>

#include <cetech/yaml.h>
#include <cetech/scenegraph.h>

#include "scene_blob.h"
#include <cetech/os/path.h>
#include <cetech/os/thread.h>
#include <cetech/os/vio.h>

using namespace celib;

namespace scene_resource_compiler {
    int init(ct_api_a0 *api);
}

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


struct scene_instance {
    uint64_t scene;
    Map<uint8_t> geom_map;
    Array<uint32_t> size;
    Array<bgfx::VertexBufferHandle> vb;
    Array<bgfx::IndexBufferHandle> ib;
};


//==============================================================================
// GLobals
//==============================================================================

#define  _G SceneResourceGlobals
static struct SceneResourceGlobals {
    uint64_t type;
    Map<uint32_t> scene_instance_map;
    Array<scene_instance> scene_instance_array;
} SceneResourceGlobals;

struct scene_instance *_init_scene_instance(uint64_t scene) {
    uint32_t idx = array::size(_G.scene_instance_array);
    array::push_back(_G.scene_instance_array, {0});

    scene_instance *instance = &_G.scene_instance_array[idx];
    instance->scene = scene;

    instance->geom_map.init(ct_memory_a0.main_allocator());
    instance->size.init(ct_memory_a0.main_allocator());
    instance->vb.init(ct_memory_a0.main_allocator());
    instance->ib.init(ct_memory_a0.main_allocator());

    map::set(_G.scene_instance_map, scene, idx);

    return instance;
}

void _destroy_scene_instance(uint64_t scene) {
    Map<uint32_t> map1(ct_memory_a0.main_allocator());
    map::set<uint32_t>(map1, 1111, 111);
    map::remove(map1, 1111);

    uint32_t idx = map::get(_G.scene_instance_map, scene, UINT32_MAX);
    uint32_t size = array::size(_G.scene_instance_array);

    scene_instance *instance = &_G.scene_instance_array[idx];
    scene_instance *last_instance = &_G.scene_instance_array[size - 1];

    instance->geom_map.destroy();
    instance->size.destroy();
    instance->vb.destroy();
    instance->ib.destroy();

    *instance = *last_instance;
    map::remove(_G.scene_instance_map, scene);
    map::set(_G.scene_instance_map, last_instance->scene, idx);
    array::pop_back(_G.scene_instance_array);
}

struct scene_instance *_get_scene_instance(uint64_t scene) {
    uint32_t idx = map::get(_G.scene_instance_map, scene, UINT32_MAX);

    if (idx == UINT32_MAX) {
        ct_resource_a0.get(_G.type, scene);
        idx = map::get(_G.scene_instance_map, scene, UINT32_MAX);
    }

    return &_G.scene_instance_array[idx];
}

//==============================================================================
// Compiler private
//==============================================================================


//==============================================================================
// Resource
//==============================================================================
namespace scene_resource {

    static const bgfx::TextureHandle null_texture = {0};


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

        for (int i = 0; i < resource->geom_count; ++i) {
            auto vb_mem = bgfx::makeRef((const void *) &vb[vb_offset[i]],
                                        vb_size[i]);

            auto ib_mem = bgfx::makeRef((const void *) &ib[ib_offset[i]],
                                        sizeof(uint32_t) * ib_size[i]);

            auto bv_handle = bgfx::createVertexBuffer(vb_mem, vb_decl[i],
                                                      BGFX_BUFFER_NONE);

            auto ib_handle = bgfx::createIndexBuffer(ib_mem,
                                                     BGFX_BUFFER_INDEX32);

            uint8_t idx = (uint8_t) array::size(instance->vb);
            map::set(instance->geom_map, geom_name[i], idx);

            array::push_back(instance->size, ib_size[i]);
            array::push_back(instance->vb, bv_handle);
            array::push_back(instance->ib, ib_handle);
        }
    }

    void offline(uint64_t name,
                 void *data) {
        _destroy_scene_instance(name);
    }

    void *reloader(uint64_t name,
                   void *old_data,
                   void *new_data,
                   cel_alloc *allocator) {
        offline(name, old_data);
        online(name, new_data);

        CEL_FREE(allocator, old_data);

        return new_data;
    }

    static const ct_resource_callbacks_t callback = {
            .loader = loader,
            .unloader = unloader,
            .online = online,
            .offline = offline,
            .reloader = reloader
    };
}

//==============================================================================
// Interface
//==============================================================================
namespace scene {
    int init(ct_api_a0 *api) {
        CETECH_GET_API(api, ct_memory_a0);
        CETECH_GET_API(api, ct_resource_a0);
        CETECH_GET_API(api, ct_scenegprah_a0);
        CETECH_GET_API(api, ct_path_a0);
        CETECH_GET_API(api, ct_vio_a0);
        CETECH_GET_API(api, ct_hash_a0);
        CETECH_GET_API(api, ct_thread_a0);

        _G = {0};

        _G.type = ct_hash_a0.id64_from_str("scene");

        _G.scene_instance_array.init(ct_memory_a0.main_allocator());
        _G.scene_instance_map.init(ct_memory_a0.main_allocator());

        ct_resource_a0.register_type(_G.type, scene_resource::callback);

#ifdef CETECH_CAN_COMPILE
        scene_resource_compiler::init(api);
#endif

        return 1;
    }

    void shutdown() {
        _G.scene_instance_array.destroy();
        _G.scene_instance_map.destroy();
    }

    void submit(uint64_t scene,
                uint64_t geom_name) {
        scene_instance *instance = _get_scene_instance(scene);

        if (instance == NULL) {
            return;
        }

        uint8_t idx = map::get<uint8_t>(instance->geom_map, geom_name,
                                        UINT8_MAX);

        if (idx == UINT8_MAX) {
            return;
        }

        bgfx::setVertexBuffer(0, instance->vb[idx], 0, instance->size[idx]);
        bgfx::setIndexBuffer(instance->ib[idx], 0, instance->size[idx]);
    }

    void create_graph(ct_world world,
                      ct_entity entity,
                      uint64_t scene) {
        auto *res = scene_blob::get(ct_resource_a0.get(_G.type, scene));

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

    uint64_t get_mesh_node(uint64_t scene,
                           uint64_t mesh) {
        auto *res = scene_blob::get(ct_resource_a0.get(_G.type, scene));

        uint64_t *geom_node = scene_blob::geom_node(res);
        uint64_t *geom_name = scene_blob::geom_name(res);

        for (int i = 0; i < res->geom_count; ++i) {
            if (geom_name[i] != mesh) {
                continue;
            }

            return geom_node[i];
        }

        return 0;
    }
}