//==============================================================================
// Include
//==============================================================================

#include <celib/module.h>
#include <cetech/scene/scene.h>
#include <celib/array.inl>
#include <celib/hash.inl>
#include "celib/hashlib.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include <celib/cdb.h>
#include <celib/os.h>

#include "cetech/machine/machine.h"
#include "cetech/resource/resource.h"
#include "cetech/ecs/ecs.h"
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/icons_font_awesome.h>


int scenecompiler_init(struct ce_api_a0 *api);

//==============================================================================
// Structs
//==============================================================================

//==============================================================================
// GLobals
//==============================================================================

#define _G SceneResourceGlobals
static struct _G {
    struct ce_cdb_t db;
    struct ce_alloc *allocator;
} _G;


//==============================================================================
// Resource
//==============================================================================

static void online(uint64_t name,
                   uint64_t obj) {
    CE_UNUSED(name);

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

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

    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
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
        ce_cdb_obj_o *geom_writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(),
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


bool scene_compiler(struct ce_cdb_t db, uint64_t k);

static const char* display_icon() {
    return ICON_FA_SHARE_ALT_SQUARE;
}

static struct ct_resource_i0 ct_resource_i0 = {
        .cdb_type = cdb_type,
        .display_icon = display_icon,
        .online = online,
        .offline = offline,
        .compilator = scene_compiler,
};


//==============================================================================
// Interface
//==============================================================================
int sceneinit(struct ce_api_a0 *api) {
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_os_a0);
    CE_INIT_API(api, ce_id_a0);

    _G = (struct _G) {
            .allocator=ce_memory_a0->system,

    };

    ce_api_a0->register_api(RESOURCE_I, &ct_resource_i0);

    scenecompiler_init(api);

    return 1;
}

static void shutdown() {

}

static uint64_t resource_data(uint64_t name) {
    struct ct_resource_id rid = (struct ct_resource_id) {
            .uid = name,
    };

    return rid.uid;
}


static uint64_t get_mesh_node(uint64_t scene,
                              uint64_t mesh) {
    uint64_t res = resource_data(scene);

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), res);

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
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(),res);

    *geometries = (char *) (ce_cdb_a0->read_blob(reader, SCENE_GEOM_STR, NULL,
                                                 NULL));
    *count = ce_cdb_a0->read_uint64(reader, SCENE_GEOM_COUNT, 0);
}

static void get_all_nodes(uint64_t scene,
                          char **geometries,
                          uint32_t *count) {
    uint64_t res = resource_data(scene);
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(),res);

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
    api->register_api(CT_SCENE_API, &scene_api);
}

CE_MODULE_DEF(
        scene,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ct_resource_a0);
            CE_INIT_API(api, ce_os_a0);
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