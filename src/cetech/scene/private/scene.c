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
#include <cetech/resource/resource_preview.h>
#include <cetech/transform/transform.h>
#include <cetech/mesh/primitive_mesh.h>
#include <cetech/mesh/static_mesh.h>
#include <celib/macros.h>
#include <cetech/editor/editor_ui.h>
#include <cetech/property_editor/property_editor.h>
#include <celib/log.h>


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

typedef struct ct_scene_obj_t {
    uint64_t import;
    uint64_t geom_objs;
    uint64_t ib_len;
    uint64_t vb_len;
    uint64_t geom_count;
    uint64_t node_count;

    uint32_t *ib;
    uint8_t *vb;
    bgfx_vertex_decl_t *vb_decl;
    uint64_t *geom_name;
    uint32_t *ib_offset;
    uint32_t *vb_offset;
    uint32_t *ib_size;
    uint32_t *vb_size;

    void *geom_str;
    void *node_name;
    void *node_parent;
    void *node_pose;
    void *geom_node;
    void *node_str;
} ct_scene_obj_t;

//==============================================================================
// Resource
//==============================================================================

static void online(ce_cdb_t0 db,
                   uint64_t name,
                   uint64_t obj) {
    CE_UNUSED(name);

    ct_scene_obj_t so = {};
    ce_cdb_a0->read_to(db, obj, &so, sizeof(so));

    ce_cdb_obj_o0 *writer = ce_cdb_a0->write_begin(db, obj);
    for (uint32_t i = 0; i < so.geom_count; ++i) {
        const bgfx_memory_t *vb_mem;
        vb_mem = ct_gfx_a0->bgfx_make_ref((const void *) &so.vb[so.vb_offset[i]],
                                          so.vb_size[i]);

        const bgfx_memory_t *ib_mem;
        ib_mem = ct_gfx_a0->bgfx_make_ref((const void *) &so.ib[so.ib_offset[i]],
                                          sizeof(uint32_t) * so.ib_size[i]);

        bgfx_vertex_buffer_handle_t bv_handle;
        bv_handle = ct_gfx_a0->bgfx_create_vertex_buffer(vb_mem, &so.vb_decl[i], BGFX_BUFFER_NONE);

        bgfx_index_buffer_handle_t ib_handle;
        ib_handle = ct_gfx_a0->bgfx_create_index_buffer(ib_mem, BGFX_BUFFER_INDEX32);

        uint64_t geom_obj = ce_cdb_a0->create_object(db, SCENE_GEOM_OBJ_TYPE);
        ce_cdb_obj_o0 *geom_writer = ce_cdb_a0->write_begin(db, geom_obj);
        ce_cdb_a0->set_uint64(geom_writer, SCENE_IB_PROP, ib_handle.idx);
        ce_cdb_a0->set_uint64(geom_writer, SCENE_VB_PROP, bv_handle.idx);
        ce_cdb_a0->set_uint64(geom_writer, SCENE_IB_SIZE, so.ib_size[i]);
        ce_cdb_a0->set_uint64(geom_writer, SCENE_VB_SIZE, so.vb_size[i]);
        ce_cdb_a0->set_uint64(geom_writer, SCENE_GEOM_OBJS_NAME, so.geom_name[i]);
        ce_cdb_a0->write_commit(geom_writer);

        ce_cdb_a0->objset_add_obj(writer, SCENE_GEOM_OBJS, geom_obj);
    }

    ce_cdb_a0->write_commit(writer);
    ce_cdb_a0->log_obj("scene", db, obj);
}

uint64_t get_geom_obj(uint64_t obj,
                      uint64_t geom_name) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    uint64_t n = ce_cdb_a0->read_objset_num(r, SCENE_GEOM_OBJS);
    uint64_t objs[n];
    ce_cdb_a0->read_objset(r, SCENE_GEOM_OBJS, objs);

    for (int i = 0; i < n; ++i) {
        const ce_cdb_obj_o0 *geom_r = ce_cdb_a0->read(ce_cdb_a0->db(), objs[i]);
        uint64_t geom_key  = ce_cdb_a0->read_uint64(geom_r, SCENE_GEOM_OBJS_NAME, 0);

        if (geom_key == geom_name) {
            return objs[i];
        }
    }
    return 0;
}

static void offline(ce_cdb_t0 db,
                    uint64_t name,
                    uint64_t obj) {
    CE_UNUSED(name, obj);

}

static uint64_t cdb_type() {
    return SCENE_TYPE;
}


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

    ct_ecs_a0->add(world, ent[0], (ct_component_pair_t0[]) {
            {
                    .type = TRANSFORM_COMPONENT,
                    .data = &(ct_transform_comp) {
                            .scl = CE_VEC3_UNIT,
                            .world = CE_MAT4_IDENTITY
                    }
            }
    }, 1);

    for (int i = 0; i < items_count; ++i) {
        const char *geom = &items[i * 128];

        ct_ecs_a0->add(world, ent[0], (ct_component_pair_t0[]) {
                {
                        .type = TRANSFORM_COMPONENT,
                        .data = &(ct_transform_comp) {
                                .scl = CE_VEC3_UNIT,
                                .pos.z = 100.0f,
                                .world = CE_MAT4_IDENTITY
                        }
                },
                {
                        .type = MESH_RENDERER_COMPONENT,
                        .data = &(ct_mesh_component) {
                                .material = 0x24c9413e88ebaaa8,
                                .scene = resource,
                                .mesh = ce_id_a0->id64(geom)
                        }
                }
        }, 2);

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


bool scene_compiler(ce_cdb_t0 db,
                    uint64_t obj);

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
                                                             SCENE_GEOM_NAME, NULL, NULL));

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

    *geometries = (char *) (ce_cdb_a0->read_blob(reader, SCENE_GEOM_STR, NULL, NULL));
    *count = ce_cdb_a0->read_uint64(reader, SCENE_GEOM_COUNT, 0);
}

static void get_all_nodes(uint64_t scene,
                          char **geometries,
                          uint32_t *count) {
    uint64_t res = resource_data(scene);
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), res);

    *geometries = (char *) (ce_cdb_a0->read_blob(reader, SCENE_NODE_STR, NULL, NULL));
    *count = ce_cdb_a0->read_uint64(reader, SCENE_NODE_COUNT, 0);
}

static struct ct_scene_a0 scene_api = {
        .get_mesh_node =get_mesh_node,
        .get_all_geometries =get_all_geometries,
        .get_all_nodes = get_all_nodes,
        .get_geom_obj = get_geom_obj,
};

struct ct_scene_a0 *ct_scene_a0 = &scene_api;

static void _init_api(struct ce_api_a0 *api) {
    api->register_api(CT_SCENE_API, &scene_api, sizeof(scene_api));
}


static const ce_cdb_prop_def_t0 scene_import_prop[] = {
        {
                .name = "input",
                .type = CE_CDB_TYPE_STR,
        },
        {
                .name = "flip_uvs",
                .type = CE_CDB_TYPE_BOOL,
        },
};

static const ce_cdb_prop_def_t0 scene_prop[] = {
        {
                .name = "import",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = SCENE_IMPORT_TYPE,
        },
        {
                .name = "geom_objs",
                .type = CE_CDB_TYPE_SET_SUBOBJECT,
        },
        {
                .name = "ib_len",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "vb_len",
                .type = CE_CDB_TYPE_UINT64,

        },
        {
                .name = "geom_count",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "node_count",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "ib",
                .type = CE_CDB_TYPE_BLOB,
        },
        {
                .name = "vb",
                .type = CE_CDB_TYPE_BLOB,
        },
        {
                .name = "vb_decl",
                .type = CE_CDB_TYPE_BLOB,
        },
        {
                .name = "geom_name",
                .type = CE_CDB_TYPE_BLOB,
        },
        {
                .name = "ib_offset",
                .type = CE_CDB_TYPE_BLOB,

        },
        {
                .name = "vb_offset",
                .type = CE_CDB_TYPE_BLOB,
        },
        {
                .name = "ib_size",
                .type = CE_CDB_TYPE_BLOB,

        },
        {
                .name = "vb_size",
                .type = CE_CDB_TYPE_BLOB,
        },
        {
                .name = "geom_str",
                .type = CE_CDB_TYPE_BLOB,
        },
        {
                .name = "node_name",
                .type = CE_CDB_TYPE_BLOB,
        },
        {
                .name = "node_parent",
                .type = CE_CDB_TYPE_BLOB,
        },
        {
                .name = "node_pose",
                .type = CE_CDB_TYPE_BLOB,
        },
        {
                .name = "geom_node",
                .type = CE_CDB_TYPE_BLOB,
        },
        {
                .name = "node_str",
                .type = CE_CDB_TYPE_BLOB,
        },
};

static const ce_cdb_prop_def_t0 scene_geom_obj_prop[] = {
        {
                .name = "ib",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "vb",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "ib_size",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "vb_size",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "name",
                .type = CE_CDB_TYPE_UINT64,
        },
};

static void draw_property(uint64_t obj,
                          uint64_t context) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    uint64_t import = ce_cdb_a0->read_subobject(reader, SCENE_IMPORT_PROP, 0);

    if (!import) {
        return;
    }

    ct_editor_ui_a0->prop_filename(import, "Input", SCENE_INPUT_PROP, "gltf", 0);
}

static struct ct_property_editor_i0 property_editor_api = {
        .cdb_type = cdb_type,
        .draw_ui = draw_property,
};

void CE_MODULE_LOAD(scene)(struct ce_api_a0 *api,
                           int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_renderer_a0);
    _init_api(api);

    ce_cdb_a0->reg_obj_type(SCENE_TYPE, scene_prop, CE_ARRAY_LEN(scene_prop));

    ce_cdb_a0->reg_obj_type(SCENE_GEOM_OBJ_TYPE,
                            scene_geom_obj_prop, CE_ARRAY_LEN(scene_geom_obj_prop));

    ce_cdb_a0->reg_obj_type(SCENE_IMPORT_TYPE,
                            scene_import_prop, CE_ARRAY_LEN(scene_import_prop));

    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_id_a0);

    _G = (struct _G) {
            .allocator=ce_memory_a0->system,

    };

    ce_api_a0->add_impl(CT_RESOURCE_I, &ct_resource_api, sizeof(ct_resource_api));

    api->add_impl(CT_PROPERTY_EDITOR_I,
                  &property_editor_api, sizeof(property_editor_api));

    scenecompiler_init(api);
}

void CE_MODULE_UNLOAD(scene)(struct ce_api_a0 *api,
                             int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
