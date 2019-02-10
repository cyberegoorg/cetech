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
    struct ce_cdb_t0 db;
    struct ce_alloc_t0 *allocator;
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


bool scene_compiler(ce_cdb_t0 db, uint64_t k);

static const char* display_icon() {
    return ICON_FA_SHARE_ALT_SQUARE;
}


static struct ct_entity_t0 load(uint64_t resource,
                             ct_world_t0 world) {

    char *items = NULL;
    uint32_t items_count = 0;
    ct_scene_a0->get_all_geometries(resource, &items, &items_count);

    struct ct_entity_t0 ent[items_count+1];
    ct_ecs_a0->create(world, ent, items_count+1);

    ct_ecs_a0->add(
            world, ent[0],
            (uint64_t[]) {
                    TRANSFORM_COMPONENT,
            }, 1,
            (void *[]) {
                    &(ct_transform_comp) {
                            .t.scl = CE_TRANFORM_INIT.scl,
                            .t.rot  = CE_TRANFORM_INIT.rot
                    },
            }
    );

    for (int i = 0; i < items_count; ++i) {
        uint64_t geom = ce_id_a0->id64(&items[i*128]);
        ct_ecs_a0->add(
                world, ent[i+1],
                (uint64_t[]) {
                        TRANSFORM_COMPONENT,
                        MESH_RENDERER_COMPONENT,
                }, 2,
                (void *[]) {
                        &(ct_transform_comp) {
                                .t.pos = {0.0f, 0.0f, 100.0f},
                                .t.scl = CE_TRANFORM_INIT.scl,
                                .t.rot  = CE_TRANFORM_INIT.rot
                        },

                        &(ct_mesh_component) {
                                .material = 0x24c9413e88ebaaa8,
                                .scene = resource,
                                .mesh = geom,
                        }}
        );

        ct_ecs_a0->link(world, ent[0], ent[i+1]);
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
    struct ct_resource_id_t0 rid = (ct_resource_id_t0) {
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
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(),res);

    *geometries = (char *) (ce_cdb_a0->read_blob(reader, SCENE_GEOM_STR, NULL,
                                                 NULL));
    *count = ce_cdb_a0->read_uint64(reader, SCENE_GEOM_COUNT, 0);
}

static void get_all_nodes(uint64_t scene,
                          char **geometries,
                          uint32_t *count) {
    uint64_t res = resource_data(scene);
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(),res);

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

CE_MODULE_DEF(
        scene,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ct_resource_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ct_renderer_a0);
        },
        {
            CE_UNUSED(reload);
            _init_api(api);
            sceneinit(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);

            shutdown();
        }
)