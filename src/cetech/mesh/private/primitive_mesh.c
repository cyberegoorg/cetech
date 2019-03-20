#include <string.h>
#include <stdlib.h>

#include "celib/id.h"
#include "celib/config.h"
#include "celib/memory/memory.h"
#include <celib/module.h>
#include <celib/math/math.h>
#include <celib/macros.h>
#include <celib/log.h>

#include "celib/api.h"
#include "cetech/resource/resource.h"
#include "cetech/ecs/ecs.h"
#include "cetech/transform/transform.h"
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/material/material.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/editor/editor_ui.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/default_rg/default_rg.h>
#include <cetech/mesh/primitive_mesh.h>
#include <cetech/mesh/static_mesh.h>

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

static const struct pt_vertex _cube_vertices[24] = {
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

static const uint16_t cube_indices[36] = {
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

void foreach_primitive_mesh(ct_world_t0 world,
                            struct ct_entity_t0 *entities,
                            ct_entity_storage_o0 *item,
                            uint32_t n,
                            void *_data) {
    mesh_render_data *data = _data;

    ct_transform_comp *transforms = ct_ecs_a0->get_all(TRANSFORM_COMPONENT, item);
    ct_primitive_mesh *mesh_renderers = ct_ecs_a0->get_all(PRIMITIVE_MESH_COMPONENT, item);

    for (int i = 0; i < n; ++i) {
        ct_transform_comp t_c = transforms[i];
        ct_primitive_mesh p_c = mesh_renderers[i];

        if (!p_c.material) {
            continue;
        }

        ct_gfx_a0->bgfx_set_transform(t_c.world.m, 1);
        ct_gfx_a0->bgfx_set_vertex_buffer(0, cube_vbh, 0, CE_ARRAY_LEN(_cube_vertices));
        ct_gfx_a0->bgfx_set_index_buffer(cube_ibh, 0, CE_ARRAY_LEN(cube_indices));

        ct_material_a0->submit(p_c.material, data->layer_name, data->viewid);
    }
}

static uint64_t cdb_type() {
    return PRIMITIVE_MESH_COMPONENT;
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

    ct_ecs_a0->process_serial(world,
                              ct_ecs_a0->mask(PRIMITIVE_MESH_COMPONENT) |
                              ct_ecs_a0->mask(TRANSFORM_COMPONENT),
                              foreach_primitive_mesh, &render_data);
}


static struct ct_renderer_component_i0 ct_renderer_component_i = {
        .render = render
};

static void *get_interface(uint64_t name_hash) {
    if (EDITOR_COMPONENT == name_hash) {
        static struct ct_editor_component_i0 ct_editor_component_i0 = {
                .display_name = display_name,
        };

        return &ct_editor_component_i0;
    } else if (CT_RENDERER_COMPONENT_I == name_hash) {
        return &ct_renderer_component_i;
    }

    return NULL;
}


static uint64_t primitive_mesh_size() {
    return sizeof(ct_primitive_mesh);
}

static void _prim_mesh_on_spawn(uint64_t obj,
                                void *data) {
    ct_primitive_mesh *c = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(ct_primitive_mesh));
}

static struct ct_component_i0 ct_component_api = {
        .cdb_type = cdb_type,
        .get_interface = get_interface,
        .size = primitive_mesh_size,
        .on_spawn = _prim_mesh_on_spawn,
        .on_change = _prim_mesh_on_spawn,
};


static ce_cdb_prop_def_t0 primitive_mesh_prop[] = {
        {
                .name = "material",
                .type = CDB_TYPE_REF,
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

    api->register_api(CT_COMPONENT_INTERFACE, &ct_component_api, sizeof(ct_component_api));
    api->register_api(CT_PROPERTY_EDITOR_INTERFACE, &property_editor_api,
                      sizeof(property_editor_api));

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
            ct_gfx_a0->bgfx_make_ref(_cube_vertices, sizeof(_cube_vertices)),
            &pt_vertex_decl, BGFX_BUFFER_NONE);

    cube_ibh = ct_gfx_a0->bgfx_create_index_buffer(
            ct_gfx_a0->bgfx_make_ref(cube_indices, sizeof(cube_indices)),
            BGFX_BUFFER_NONE);

    ce_cdb_a0->reg_obj_type(PRIMITIVE_MESH_COMPONENT,
                            primitive_mesh_prop, CE_ARRAY_LEN(primitive_mesh_prop));
}

void CE_MODULE_UNLOAD(primitive_mesh)(struct ce_api_a0 *api,
                                      int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}