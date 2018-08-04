//==============================================================================
// Include
//==============================================================================

#include <corelib/module.h>
#include <cetech/gfx/scene.h>
#include <corelib/array.inl>
#include <corelib/hash.inl>

#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "cetech/machine/machine.h"


#include "cetech/resource/resource.h"
#include "cetech/ecs/ecs.h"

#include "cetech/scenegraph/scenegraph.h"

#include <corelib/os.h>
#include <cetech/gfx/renderer.h>
#include <corelib/cdb.h>


int scenecompiler_init(struct ct_api_a0 *api);

//==============================================================================
// Structs
//==============================================================================

//==============================================================================
// GLobals
//==============================================================================

#define _G SceneResourceGlobals
static struct _G {
    struct ct_cdb_t db;
    struct ct_alloc *allocator;
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

    ct_cdb_a0->load(ct_cdb_a0->db(), data, obj, _G.allocator);

    uint64_t geom_count = ct_cdb_a0->read_uint64(obj, SCENE_GEOM_COUNT, 0);
    ct_render_vertex_decl_t *vb_decl = (ct_cdb_a0->read_blob(obj, SCENE_VB_DECL,
                                                             NULL, NULL));
    uint64_t *geom_name = (ct_cdb_a0->read_blob(obj, SCENE_GEOM_NAME,
                                                NULL, NULL));
    uint32_t *ib_offset = (ct_cdb_a0->read_blob(obj, SCENE_IB_OFFSET,
                                                NULL, NULL));
    uint32_t *vb_offset = (ct_cdb_a0->read_blob(obj, SCENE_VB_OFFSET,
                                                NULL, NULL));
    uint32_t *ib_size = (ct_cdb_a0->read_blob(obj, SCENE_IB_SIZE, NULL, NULL));
    uint32_t *vb_size = (ct_cdb_a0->read_blob(obj, SCENE_VB_SIZE, NULL, NULL));
    uint32_t *ib = (ct_cdb_a0->read_blob(obj, SCENE_IB_PROP, NULL, NULL));
    uint8_t *vb = (ct_cdb_a0->read_blob(obj, SCENE_VB_PROP, NULL, NULL));

    ct_cdb_obj_o *writer = ct_cdb_a0->write_begin(obj);
    for (uint32_t i = 0; i < geom_count; ++i) {
        const ct_render_memory_t *vb_mem;
        vb_mem = ct_renderer_a0->make_ref((const void *) &vb[vb_offset[i]],
                                          vb_size[i]);

        const ct_render_memory_t *ib_mem;
        ib_mem = ct_renderer_a0->make_ref((const void *) &ib[ib_offset[i]],
                                          sizeof(uint32_t) * ib_size[i]);

        ct_render_vertex_buffer_handle_t bv_handle;
        bv_handle = ct_renderer_a0->create_vertex_buffer(vb_mem,
                                                         &vb_decl[i],
                                                         CT_RENDER_BUFFER_NONE);

        ct_render_index_buffer_handle_t ib_handle;
        ib_handle = ct_renderer_a0->create_index_buffer(ib_mem,
                                                        CT_RENDER_BUFFER_INDEX32);

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

static uint64_t cdb_type() {
    return SCENE_TYPE;
}


void scene_compiler(const char *filename,
                    char **output_blob);

static struct ct_resource_i0 ct_resource_i0 = {
        .cdb_type = cdb_type,
        .online = online,
        .offline = offline,
        .compilator = scene_compiler,
};


//==============================================================================
// Interface
//==============================================================================
int sceneinit(struct ct_api_a0 *api) {
    CT_INIT_API(api, ct_memory_a0);
    CT_INIT_API(api, ct_resource_a0);
    CT_INIT_API(api, ct_scenegprah_a0);
    CT_INIT_API(api, ct_os_a0);
    CT_INIT_API(api, ct_hashlib_a0);

    _G = (struct _G) {
            .allocator=ct_memory_a0->system,

    };

    ct_api_a0->register_api(RESOURCE_I_NAME, &ct_resource_i0);

    scenecompiler_init(api);

    return 1;
}

static void shutdown() {

}

static uint64_t resource_data(uint64_t name) {
    struct ct_resource_id rid = (struct ct_resource_id) {
            .type = SCENE_TYPE,
            .name = name,
    };

    return ct_resource_a0->get(rid);
}

static void create_graph(struct ct_world world,
                         struct ct_entity entity,
                         uint64_t scene) {

    uint64_t res = resource_data(scene);

    uint64_t *node_name = (uint64_t *) (ct_cdb_a0->read_blob(res,
                                                             SCENE_NODE_NAME,
                                                             NULL, NULL));
    
    uint32_t *node_parent = (uint32_t *) (ct_cdb_a0->read_blob(res,
                                                               SCENE_NODE_PARENT,
                                                               NULL, NULL));
    float *node_pose = (float *) (ct_cdb_a0->read_blob(res,
                                                       SCENE_NODE_POSE,
                                                       NULL, NULL));

    uint64_t node_count = ct_cdb_a0->read_uint64(res, SCENE_NODE_COUNT, 0);

    ct_scenegprah_a0->create(world,
                             entity,
                             node_name,
                             node_parent,
                             node_pose,
                             node_count);
}

static uint64_t get_mesh_node(uint64_t scene,
                              uint64_t mesh) {
    uint64_t res = resource_data(scene);

    uint64_t *geom_name = (uint64_t *) (ct_cdb_a0->read_blob(res,
                                                             SCENE_GEOM_NAME,
                                                             NULL, NULL));
    uint64_t *geom_node = (uint64_t *) (ct_cdb_a0->read_blob(res,
                                                             SCENE_NODE_GEOM,
                                                             NULL, NULL));
    uint64_t geom_count = ct_cdb_a0->read_uint64(res, SCENE_GEOM_COUNT, 0);

    for (uint32_t i = 0; i < geom_count; ++i) {
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
    uint64_t res = resource_data(scene);

    *geometries = (char *) (ct_cdb_a0->read_blob(res, SCENE_GEOM_STR, NULL,
                                                 NULL));
    *count = ct_cdb_a0->read_uint64(res, SCENE_GEOM_COUNT, 0);
}

static void get_all_nodes(uint64_t scene,
                          char **geometries,
                          uint32_t *count) {
    uint64_t res = resource_data(scene);

    *geometries = (char *) (ct_cdb_a0->read_blob(res, SCENE_NODE_STR,
                                                 NULL, NULL));
    *count = ct_cdb_a0->read_uint64(res, SCENE_NODE_COUNT, 0);

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
            CT_INIT_API(api, ct_memory_a0);
            CT_INIT_API(api, ct_resource_a0);
            CT_INIT_API(api, ct_scenegprah_a0);
            CT_INIT_API(api, ct_os_a0);
            CT_INIT_API(api, ct_hashlib_a0);
            CT_INIT_API(api, ct_cdb_a0);
            CT_INIT_API(api, ct_renderer_a0);
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