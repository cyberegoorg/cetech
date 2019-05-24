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

#define LOG_WHERE "rectangle_renderer"

#define RECTANGLE_RENDERER_COMPONENT \
    CE_ID64_0("rectangle_renderer", 0x908634ea0cd1ce5dULL)

#define _G rectangle_render_global

typedef struct rectangle_renderer_t {
    uint64_t material;
} rectangle_renderer_t;

static struct _G {
    ce_alloc_t0 *allocator;
} _G;

typedef struct rectangle_render_data {
    uint8_t viewid;
    uint64_t layer_name;
} rectangle_render_data;

struct pt_vertex {
    float m_x;
    float m_y;
    float m_z;
    float m_u;
    float m_v;
};
static bgfx_vertex_decl_t pt_vertex_decl;

static const struct pt_vertex _cube_vertices[4] = {
        {-0.5f, -0.5f, 0,  0, 1},
        {0.5f,  0.5f,  0,  1, 0},
        {-0.5f, 0.5f,  0,  0, 0},
        {0.5f,  -0.5f, 0,  1, 1},
};

static const uint16_t cube_indices[6] = {
        0, 1, 3,
        0, 2, 1,
};

static bgfx_vertex_buffer_handle_t cube_vbh;
static bgfx_index_buffer_handle_t cube_ibh;

void foreach_primitive_mesh(ct_world_t0 world,
                            struct ct_entity_t0 *entities,
                            ct_entity_storage_o0 *item,
                            uint32_t n,
                            ct_ecs_cmd_buffer_t *buff,
                            void *_data) {
    rectangle_render_data *data = _data;

    ct_transform_comp *transforms = ct_ecs_a0->get_all(TRANSFORM_COMPONENT, item);
    rectangle_component *rectangles = ct_ecs_a0->get_all(RECTANGLE_COMPONENT, item);
    rectangle_renderer_t *mesh_renderers = ct_ecs_a0->get_all(RECTANGLE_RENDERER_COMPONENT, item);

    for (int i = 0; i < n; ++i) {
        ct_transform_comp t_c = transforms[i];
        rectangle_renderer_t p_c = mesh_renderers[i];
        rectangle_component r_c = rectangles[i];

        if (!p_c.material) {
            continue;
        }

        ce_mat4_t scale = {};
        ce_mat4_scale(scale.m, r_c.half_size.x*2, r_c.half_size.y*2, 0.0f);

        ce_mat4_t w  = {};
        ce_mat4_mul(w.m, scale.m, t_c.world.m);

        ct_gfx_a0->bgfx_set_transform(w.m, 1);
        ct_gfx_a0->bgfx_set_vertex_buffer(0, cube_vbh, 0, CE_ARRAY_LEN(_cube_vertices));
        ct_gfx_a0->bgfx_set_index_buffer(cube_ibh, 0, CE_ARRAY_LEN(cube_indices));

        ct_material_a0->submit(p_c.material, data->layer_name, data->viewid);
    }
}

static uint64_t rectangle_renderer_cdb_type() {
    return ce_id_a0->id64("rectangle_renderer");
}

static const char *rectangle_renderer_display_name() {
    return ICON_FA_HOUZZ " Rectangle renderer";
}

static struct ct_property_editor_i0 rectangle_renderer_property_editor_api = {
        .cdb_type = cdb_type,
};

static void render(ct_world_t0 world,
                   struct ct_rg_builder_t0 *builder) {

    uint8_t viewid = builder->get_layer_viewid(builder, _GBUFFER);


    rectangle_render_data render_data = {
            .viewid = viewid,
            .layer_name = _GBUFFER,
    };

    ct_ecs_a0->process_serial(world,
                              ct_ecs_a0->mask(RECTANGLE_RENDERER_COMPONENT) |
                              ct_ecs_a0->mask(TRANSFORM_COMPONENT),
                              foreach_primitive_mesh, &render_data);
}


static struct ct_renderer_component_i0 ct_renderer_component_i = {
        .render = render
};

static void *rectangle_renderer_get_interface(uint64_t name_hash) {
    if (CT_RENDERER_COMPONENT_I == name_hash) {
        return &ct_renderer_component_i;
    }

    return NULL;
}


static uint64_t rectangle_render_size() {
    return sizeof(ct_primitive_mesh);
}

static void _prim_mesh_on_spawn(uint64_t obj,
                                void *data) {
    rectangle_renderer_t *c = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(rectangle_renderer_t));
}

static struct ct_ecs_component_i0 ct_component_api = {
        .display_name = rectangle_renderer_display_name,
        .cdb_type = rectangle_renderer_cdb_type,
        .get_interface = rectangle_renderer_get_interface,
        .size = rectangle_render_size,
        .on_spawn = _prim_mesh_on_spawn,
        .on_change = _prim_mesh_on_spawn,
};


static ce_cdb_prop_def_t0 rectangle_render_prop[] = {
        {
                .name = "material",
                .type = CE_CDB_TYPE_REF,
                .obj_type = MATERIAL_TYPE,
        },
};

void CE_MODULE_LOAD(rectangle_render)(struct ce_api_a0 *api,
                                    int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_material_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ct_renderer_a0);
    CE_INIT_API(api, ct_gfx_a0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->add_impl(CT_ECS_COMPONENT_I, &ct_component_api, sizeof(ct_component_api));
    api->add_impl(CT_PROPERTY_EDITOR_I, &rectangle_renderer_property_editor_api,
                      sizeof(rectangle_renderer_property_editor_api));

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

    ce_cdb_a0->reg_obj_type(RECTANGLE_RENDERER_COMPONENT,
                            rectangle_render_prop, CE_ARRAY_LEN(rectangle_render_prop));
}

void CE_MODULE_UNLOAD(rectangle_render)(struct ce_api_a0 *api,
                                      int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}