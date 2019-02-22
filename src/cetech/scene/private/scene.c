//==============================================================================
// Include
//==============================================================================

#include <celib/module.h>
#include <cetech/scene/scene.h>
#include <celib/memory/allocator.h>
#include <celib/macros.h>
#include <celib/containers/array.h>
#include <celib/containers/hash.h>
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include <celib/cdb.h>


#include "cetech/machine/machine.h"
#include "cetech/resource/resource.h"
#include "cetech/ecs/ecs.h"
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/editor/resource_preview.h>
#include <cetech/transform/transform.h>
#include <cetech/mesh/primitive_mesh.h>
#include <cetech/mesh/mesh_renderer.h>
#include <celib/macros.h>


int scenecompiler_init(struct ce_api_a0 *api);

//==============================================================================
// Structs
//==============================================================================

//==============================================================================
// GLobals
//==============================================================================

#define _G SceneResourceGlobals
static struct _G {
    ce_cdb_t0 db;
    ce_alloc_t0 *allocator;
} _G;


//==============================================================================
// Resource
//==============================================================================

static void online(uint64_t name,
                   uint64_t obj) {
    CE_UNUSED(name);

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    uint64_t geom_count = ce_cdb_a0->read_uint64(reader, SCENE_GEOM_COUNT, 0);
    bgfx_vertex_decl_t *vb_decl = (ce_cdb_a0->read_blob(reader,
                                                        SCENE_VB_DECL,
                                                        NULL, NULL));
    uint64_t *geom_name = (ce_cdb_a0->read_blob(reader, SCENE_GEOM_NAME,
                                                NULL, NULL));
    uint32_t *ib_offset = (ce_cdb_a0->read_blob(reader, SCENE_IB_OFFSET,
                                                NULL, NULL));
    uint32_t *vb_offset = (ce_cdb_a0->read_blob(reader, SCENE_VB_OFFSET,
                                                NULL, NULL));
    uint32_t *ib_size = (ce_cdb_a0->read_blob(reader, SCENE_IB_SIZE, NULL,
                                              NULL));
    uint32_t *vb_size = (ce_cdb_a0->read_blob(reader, SCENE_VB_SIZE, NULL,
                                              NULL));
    uint32_t *ib = (ce_cdb_a0->read_blob(reader, SCENE_IB_PROP, NULL, NULL));
    uint8_t *vb = (ce_cdb_a0->read_blob(reader, SCENE_VB_PROP, NULL, NULL));

    ce_cdb_obj_o0 *writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
    for (uint32_t i = 0; i < geom_count; ++i) {
        const bgfx_memory_t *vb_mem;
        vb_mem = ct_gfx_a0->bgfx_make_ref((const void *) &vb[vb_offset[i]],
                                          vb_size[i]);

        const bgfx_memory_t *ib_mem;
        ib_mem = ct_gfx_a0->bgfx_make_ref((const void *) &ib[ib_offset[i]],
                                          sizeof(uint32_t) * ib_size[i]);

        bgfx_vertex_buffer_handle_t bv_handle;
        bv_handle = ct_gfx_a0->bgfx_create_vertex_buffer(vb_mem,
                                                         &vb_decl[i],
                                                         BGFX_BUFFER_NONE);

        bgfx_index_buffer_handle_t ib_handle;
        ib_handle = ct_gfx_a0->bgfx_create_index_buffer(ib_mem,
                                                        BGFX_BUFFER_INDEX32);

        uint64_t geom_obj = ce_cdb_a0->create_object(_G.db, 0);
        ce_cdb_obj_o0 *geom_writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(),
                                                            geom_obj);
        ce_cdb_a0->set_uint64(geom_writer, SCENE_IB_PROP, ib_handle.idx);
        ce_cdb_a0->set_uint64(geom_writer, SCENE_VB_PROP, bv_handle.idx);
        ce_cdb_a0->set_uint64(geom_writer, SCENE_IB_SIZE, ib_size[i]);
        ce_cdb_a0->set_uint64(geom_writer, SCENE_VB_SIZE, vb_size[i]);
        ce_cdb_a0->write_commit(geom_writer);

        ce_cdb_a0->set_ref(writer, geom_name[i], geom_obj);
    }

    ce_cdb_a0->write_commit(writer);
}

static void offline(uint64_t name,
                    uint64_t obj) {
    CE_UNUSED(name, obj);

}

static uint64_t cdb_type() {
    return SCENE_TYPE;
}


bool scene_compiler(ce_cdb_t0 db,
                    uint64_t k);

static const char *display_icon() {
    return ICON_FA_SHARE_ALT_SQUARE;
}


static struct ct_entity_t0 load(uint64_t resource,
                                ct_world_t0 world) {

    char *items = NULL;
    uint32_t items_count = 0;
    ct_scene_a0->get_all_geometries(resource, &items, &items_count);

    ct_entity_t0 ent[items_count + 1];
    ct_ecs_a0->create(world, ent, items_count + 1);

    ct_ecs_a0->add(world, ent[0],
                   (uint64_t[]) {
                           ce_cdb_a0->create_object(ce_cdb_a0->db(), TRANSFORM_COMPONENT)
                   }, 1);

    for (int i = 0; i < items_count; ++i) {
        const char *geom = &items[i * 128];

        uint64_t transform = ce_cdb_a0->create_object(ce_cdb_a0->db(), TRANSFORM_COMPONENT);
        const ce_cdb_obj_o0 *tr = ce_cdb_a0->read(ce_cdb_a0->db(), transform);
        uint64_t pos = ce_cdb_a0->read_subobject(tr, PROP_POSITION, 0);
        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), pos);
        ce_cdb_a0->set_float(w, PROP_POSITION_Z, 100.0f);
        ce_cdb_a0->write_commit(w);

        uint64_t mesh = ce_cdb_a0->create_object(ce_cdb_a0->db(), MESH_RENDERER_COMPONENT);
        ce_cdb_obj_o0 *mesh_w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), mesh);
        ce_cdb_a0->set_ref(mesh_w, PROP_SCENE_ID, resource);
        ce_cdb_a0->set_str(mesh_w, PROP_MESH, geom);
        ce_cdb_a0->set_ref(mesh_w, PROP_MATERIAL, 0x24c9413e88ebaaa8);
        ce_cdb_a0->write_commit(mesh_w);

        ct_ecs_a0->add(world, ent[i + 1],
                       (uint64_t[]) {transform, mesh}, 2);

        ct_ecs_a0->link(world, ent[0], ent[i + 1]);
    }

    return ent[0];
}


static struct ct_resource_preview_i0 ct_resource_preview_api = {
        .load = load,
};

static void *get_interface(uint64_t name_hash) {
    if (name_hash == RESOURCE_PREVIEW_I) {
        return &ct_resource_preview_api;
    }
    return NULL;
}

static struct ct_resource_i0 ct_resource_api = {
        .cdb_type = cdb_type,
        .display_icon = display_icon,
        .online = online,
        .offline = offline,
        .compilator = scene_compiler,
        .get_interface = get_interface,
};


//==============================================================================
// Interface
//==============================================================================
int sceneinit(struct ce_api_a0 *api) {
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_id_a0);

    _G = (struct _G) {
            .allocator=ce_memory_a0->system,

    };

    ce_api_a0->register_api(RESOURCE_I, &ct_resource_api, sizeof(ct_resource_api));

    scenecompiler_init(api);

    return 1;
}

static void shutdown() {

}

static uint64_t resource_data(uint64_t name) {
    ct_resource_id_t0 rid = (ct_resource_id_t0) {
            .uid = name,
    };

    return rid.uid;
}


static uint64_t get_mesh_node(uint64_t scene,
                              uint64_t mesh) {
    uint64_t res = resource_data(scene);

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), res);

    uint64_t *geom_name = (uint64_t *) (ce_cdb_a0->read_blob(reader,
                                                             SCENE_GEOM_NAME,
                                                             NULL, NULL));
    uint64_t *geom_node = (uint64_t *) (ce_cdb_a0->read_blob(reader,
                                                             SCENE_NODE_GEOM,
                                                             NULL, NULL));
    uint64_t geom_count = ce_cdb_a0->read_uint64(reader, SCENE_GEOM_COUNT, 0);

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
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), res);

    *geometries = (char *) (ce_cdb_a0->read_blob(reader, SCENE_GEOM_STR, NULL,
                                                 NULL));
    *count = ce_cdb_a0->read_uint64(reader, SCENE_GEOM_COUNT, 0);
}

static void get_all_nodes(uint64_t scene,
                          char **geometries,
                          uint32_t *count) {
    uint64_t res = resource_data(scene);
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), res);

    *geometries = (char *) (ce_cdb_a0->read_blob(reader, SCENE_NODE_STR,
                                                 NULL, NULL));
    *count = ce_cdb_a0->read_uint64(reader, SCENE_NODE_COUNT, 0);

}

static struct ct_scene_a0 scene_api = {
        .get_mesh_node =get_mesh_node,
        .get_all_geometries =get_all_geometries,
        .get_all_nodes = get_all_nodes,
};

struct ct_scene_a0 *ct_scene_a0 = &scene_api;

static void _init_api(struct ce_api_a0 *api) {
    api->register_api(CT_SCENE_API, &scene_api, sizeof(scene_api));
}

void CE_MODULE_LOAD(scene)(struct ce_api_a0 *api,
                           int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_renderer_a0);
    _init_api(api);
    sceneinit(api);
}

void CE_MODULE_UNLOAD(scene)(struct ce_api_a0 *api,
                             int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    shutdown();
}
