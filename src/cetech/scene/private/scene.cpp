//==============================================================================
// Include
//==============================================================================

#include <corelib/module.h>
#include <cetech/scene/scene.h>
#include <corelib/array.inl>
#include <corelib/hash.inl>

#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "cetech/machine/machine.h"


#include "cetech/resource/resource.h"
#include "cetech/ecs/ecs.h"

#include "cetech/scenegraph/scenegraph.h"

#include "scene_blob.h"
#include <corelib/os.h>


int scenecompiler_init(struct ct_api_a0 *api);

//==============================================================================
// Structs
//==============================================================================

//==============================================================================
// GLobals
//==============================================================================

#define _G SceneResourceGlobals
static struct _G {
    ct_cdb_t db;
    uint32_t type;
    ct_alloc *allocator;
} _G;


//==============================================================================
// Resource
//==============================================================================

static void online(uint64_t name,
                   struct ct_vio *input,
                   uint64_t obj) {
    CT_UNUSED(name);

    const uint64_t size = input->size(input);
    char *data = CT_ALLOC(_G.allocator, char, size);
    input->read(input, data, 1, size);

    auto resource = scene_blob::get(data);

    ct_render_vertex_decl_t *vb_decl = scene_blob::vb_decl(resource);
    uint64_t *geom_name = scene_blob::geom_name(resource);
    uint32_t *ib_offset = scene_blob::ib_offset(resource);
    uint32_t *vb_offset = scene_blob::vb_offset(resource);
    uint32_t *ib_size = scene_blob::ib_size(resource);
    uint32_t *vb_size = scene_blob::vb_size(resource);
    uint32_t *ib = scene_blob::ib(resource);
    uint8_t *vb = scene_blob::vb(resource);

    ct_cdb_obj_o *writer = ct_cdb_a0->write_begin(obj);
    ct_cdb_a0->set_ptr(writer, SCENE_PROP, data);

    for (uint32_t i = 0; i < resource->geom_count; ++i) {
        auto vb_mem = ct_renderer_a0->make_ref((const void *) &vb[vb_offset[i]],
                                               vb_size[i]);

        auto ib_mem = ct_renderer_a0->make_ref((const void *) &ib[ib_offset[i]],
                                               sizeof(uint32_t) * ib_size[i]);

        auto bv_handle = ct_renderer_a0->create_vertex_buffer(vb_mem,
                                                              &vb_decl[i],
                                                              BGFX_BUFFER_NONE);

        auto ib_handle = ct_renderer_a0->create_index_buffer(ib_mem,
                                                             BGFX_BUFFER_INDEX32);

        uint64_t geom_obj = ct_cdb_a0->create_object(_G.db, 0);
        ct_cdb_obj_o *geom_writer = ct_cdb_a0->write_begin(geom_obj);
        ct_cdb_a0->set_uint64(geom_writer, SCENE_IB_PROP, ib_handle.idx);
        ct_cdb_a0->set_uint64(geom_writer, SCENE_VB_PROP, bv_handle.idx);
        ct_cdb_a0->set_uint64(geom_writer, SCENE_SIZE_PROP, size);
        ct_cdb_a0->write_commit(geom_writer);

        ct_cdb_a0->set_ref(writer, geom_name[i], geom_obj);
    }

    ct_cdb_a0->write_commit(writer);
}

static void offline(uint64_t name,
                    uint64_t obj) {
    CT_UNUSED(name, obj);

}

static const ct_resource_type_t callback = {
        .online = online,
        .offline = offline,
};


//==============================================================================
// Interface
//==============================================================================
int sceneinit(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_resource_a0);
    CETECH_GET_API(api, ct_scenegprah_a0);
    CETECH_GET_API(api, ct_os_a0);
    CETECH_GET_API(api, ct_hashlib_a0);

    _G = {
            .allocator=ct_memory_a0->main_allocator(),
            .type = _G.type = CT_ID32_0("scene"),

    };

    ct_resource_a0->register_type("scene", callback);

    scenecompiler_init(api);

    return 1;
}

static void shutdown() {

}

static scene_blob::blob_t *resource_data(uint32_t name) {
    struct ct_resource_id rid = (struct ct_resource_id) {
            .type = _G.type,
            .name = name,
    };

    auto object = ct_resource_a0->get(rid);
    return (scene_blob::blob_t *) (ct_cdb_a0->read_ptr(object, SCENE_PROP,
                                                       NULL));
}

static void create_graph(ct_world world,
                         ct_entity entity,
                         uint64_t scene) {

    auto *res = resource_data(scene);

    uint64_t *node_name = scene_blob::node_name(res);
    uint32_t *node_parent = scene_blob::node_parent(res);
    float *node_pose = scene_blob::node_pose(res);

    ct_scenegprah_a0->create(world,
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

static struct ct_scene_a0 scene_api = {
        .create_graph =create_graph,
        .get_mesh_node =get_mesh_node,
        .get_all_geometries =get_all_geometries,
        .get_all_nodes = get_all_nodes,
};

struct ct_scene_a0 *ct_scene_a0 = &scene_api;

static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_scene_a0", &scene_api);
}

CETECH_MODULE_DEF(
        scene,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_scenegprah_a0);
            CETECH_GET_API(api, ct_os_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_renderer_a0);
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