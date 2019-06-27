#include <string.h>

#include "celib/id.h"
#include "celib/config.h"
#include "celib/memory/memory.h"
#include <celib/module.h>
#include <celib/macros.h>

#include "celib/api.h"
#include "cetech/resource/resource.h"
#include "cetech/ecs/ecs.h"
#include "cetech/transform/transform.h"
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/material/material.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/default_rg/default_rg.h>
#include <cetech/mesh/primitive_mesh.h>

#define LOG_WHERE "primitive_mesh"

#define _G primitice_mesh_global

static struct _G {
    ce_alloc_t0 *allocator;
} _G;

typedef struct mesh_render_data {
    uint8_t viewid;
    uint64_t layer_name;
} mesh_render_data;


struct pt_vertex {
    float m_x;
    float m_y;
    float m_z;
    float m_u;
    float m_v;
};
static bgfx_vertex_decl_t pt_vertex_decl;

static const struct pt_vertex _vertices[24] = {
        // Back
        {-1, -1, 1,  1, 1},
        {1,  1,  1,  0, 0},
        {-1, 1,  1,  1, 0},
        {1,  -1, 1,  0, 1},

        // Front
        {-1, -1, -1, 0, 1},
        {1,  1,  -1, 1, 0},
        {-1, 1,  -1, 0, 0},
        {1,  -1, -1, 1, 1},

        // Right
        {1,  -1, -1, 0, 1},
        {1,  1,  1,  1, 0},
        {1,  -1, 1,  1, 1},
        {1,  1,  -1, 0, 0},

        // Left
        {-1, -1, -1, 1, 1},
        {-1, 1,  1,  0, 0},
        {-1, -1, 1,  0, 1},
        {-1, 1,  -1, 1, 0},

        // Top
        {-1, 1,  -1, 0, 1},
        {1,  1,  1,  1, 0},
        {-1, 1,  1,  0, 0},
        {1,  1,  -1, 1, 1},

        // Bottom
        {-1, -1, -1, 0, 0},
        {1,  -1, 1,  1, 1},
        {-1, -1, 1,  0, 1},
        {1,  -1, -1, 1, 0},
};

static const uint16_t _indices[36] = {
        0, 1, 2,
        0, 3, 1,
        4, 6, 5,
        4, 5, 7,

        8, 9, 10,
        8, 11, 9,
        12, 14, 13,
        12, 13, 15,

        16, 18, 17,
        16, 17, 19,
        20, 21, 22,
        20, 23, 21,
};

static bgfx_vertex_buffer_handle_t cube_vbh;
static bgfx_index_buffer_handle_t cube_ibh;

void render_primitives(ct_world_t0 world,
                       struct ct_entity_t0 *entities,
                       ct_ecs_ent_chunk_o0 *item,
                       uint32_t n,
                       void *_data) {
    mesh_render_data *data = _data;

    ct_local_to_world_c *transforms = ct_ecs_c_a0->get_all(world, LOCAL_TO_WORLD_COMPONENT, item);
    ct_primitive_mesh_c *mesh_renderers = ct_ecs_c_a0->get_all(world, PRIMITIVE_MESH_COMPONENT,
                                                               item);

    for (int i = 0; i < n; ++i) {
        ct_local_to_world_c t_c = transforms[i];
        ct_primitive_mesh_c p_c = mesh_renderers[i];

        if (!p_c.material) {
            continue;
        }

        ct_gfx_a0->bgfx_set_transform(t_c.world.m, 1);
        ct_gfx_a0->bgfx_set_vertex_buffer(0, cube_vbh, 0, CE_ARRAY_LEN(_vertices));
        ct_gfx_a0->bgfx_set_index_buffer(cube_ibh, 0, CE_ARRAY_LEN(_indices));

        ct_material_a0->submit(p_c.material, data->layer_name, data->viewid);
    }
}

static uint64_t cdb_type() {
    return ce_id_a0->id64("primitive_mesh");
}

static const char *display_name() {
    return ICON_FA_HOUZZ " Primitive mesh";
}

static struct ct_property_editor_i0 property_editor_api = {
        .cdb_type = cdb_type,
};

static void render(ct_world_t0 world,
                   struct ct_rg_builder_t0 *builder) {

    uint8_t viewid = builder->get_layer_viewid(builder, _GBUFFER);


    mesh_render_data render_data = {
            .viewid = viewid,
            .layer_name = _GBUFFER,
    };

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(PRIMITIVE_MESH_COMPONENT,
                                                                LOCAL_TO_WORLD_COMPONENT),
                                }, 0,
                                render_primitives, &render_data);
}


static struct ct_renderer_component_i0 ct_renderer_component_i = {
        .render = render
};

static void _rectangle_renderer_on_spawn(ct_world_t0 world,
                                         ce_cdb_t0 db,
                                         uint64_t obj,
                                         void *data) {
    ct_primitive_mesh_c *c = data;
    ce_cdb_a0->read_to(db, obj, c, sizeof(ct_primitive_mesh_c));
}

static struct ct_ecs_component_i0 ct_component_api = {
        .display_name = display_name,
        .cdb_type = PRIMITIVE_MESH_COMPONENT,
        .size = sizeof(ct_primitive_mesh_c),
        .from_cdb_obj = _rectangle_renderer_on_spawn,
};

static ce_cdb_prop_def_t0 primitive_mesh_prop[] = {
        {
                .name = "material",
                .type = CE_CDB_TYPE_REF,
                .obj_type = MATERIAL_TYPE,
        },
};

void CE_MODULE_LOAD(primitive_mesh)(struct ce_api_a0 *api,
                                    int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_material_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ct_renderer_a0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->add_impl(CT_ECS_COMPONENT_I, &ct_component_api, sizeof(ct_component_api));
    api->add_impl(CT_PROPERTY_EDITOR_I, &property_editor_api, sizeof(property_editor_api));

    api->add_impl(CT_RENDERER_COMPONENT_I,
                  &ct_renderer_component_i, sizeof(ct_renderer_component_i));

    ct_gfx_a0->bgfx_vertex_decl_begin(&pt_vertex_decl,
                                      ct_gfx_a0->bgfx_get_renderer_type());

    ct_gfx_a0->bgfx_vertex_decl_add(&pt_vertex_decl,
                                    BGFX_ATTRIB_POSITION, 3,
                                    BGFX_ATTRIB_TYPE_FLOAT, false, false);

    ct_gfx_a0->bgfx_vertex_decl_add(&pt_vertex_decl,
                                    BGFX_ATTRIB_TEXCOORD0, 2,
                                    BGFX_ATTRIB_TYPE_FLOAT, false, false);

    ct_gfx_a0->bgfx_vertex_decl_end(&pt_vertex_decl);

    cube_vbh = ct_gfx_a0->bgfx_create_vertex_buffer(
            ct_gfx_a0->bgfx_make_ref(_vertices, sizeof(_vertices)),
            &pt_vertex_decl, BGFX_BUFFER_NONE);

    cube_ibh = ct_gfx_a0->bgfx_create_index_buffer(
            ct_gfx_a0->bgfx_make_ref(_indices, sizeof(_indices)),
            BGFX_BUFFER_NONE);

    ce_cdb_a0->reg_obj_type(PRIMITIVE_MESH_COMPONENT,
                            primitive_mesh_prop, CE_ARRAY_LEN(primitive_mesh_prop));
}

void CE_MODULE_UNLOAD(primitive_mesh)(struct ce_api_a0 *api,
                                      int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}