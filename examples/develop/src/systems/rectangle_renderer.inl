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
#include <cetech/debugdraw/dd.h>

#define LOG_WHERE "rectangle_renderer"

#define RECTANGLE_RENDERER_COMPONENT \
    CE_ID64_0("rectangle_renderer", 0x908634ea0cd1ce5dULL)

#define _G rectangle_render_global

typedef struct rectangle_renderer_t {
    uint64_t material;
} rectangle_renderer_t;

static struct _G {
    ce_alloc_t0 *allocator;
    bgfx_vertex_buffer_handle_t cube_vbh;
    bgfx_index_buffer_handle_t cube_ibh;
} _G;

typedef struct rectangle_render_data {
    uint8_t viewid;
    uint64_t layer_name;
} rectangle_render_data;

struct pt_vertex {
    float x;
    float y;
    float z;
    float u;
    float v;
};
static bgfx_vertex_layout_t pt_vertex_decl;

static const struct pt_vertex _vertices[4] = {
        {-0.5f, -0.5f, 0, 0, 1},
        {0.5f,  0.5f,  0, 1, 0},
        {-0.5f, 0.5f,  0, 0, 0},
        {0.5f,  -0.5f, 0, 1, 1},
};

static const uint16_t _indices[6] = {
        0, 1, 3,
        0, 2, 1,
};


void render_rectangles(ct_world_t0 world,
                       struct ct_entity_t0 *entities,
                       ct_ecs_ent_chunk_o0 *item,
                       uint32_t n,
                       void *_data) {
    rectangle_render_data *data = _data;

    ct_local_to_world_c *transforms = ct_ecs_c_a0->get_all(world, LOCAL_TO_WORLD_COMPONENT, item);
    rectangle_component *rectangles = ct_ecs_c_a0->get_all(world, RECTANGLE_COMPONENT, item);
    rectangle_renderer_t *mesh_renderers = ct_ecs_c_a0->get_all(world, RECTANGLE_RENDERER_COMPONENT,
                                                                item);

    for (int i = 0; i < n; ++i) {
        ct_local_to_world_c t_c = transforms[i];
        rectangle_renderer_t p_c = mesh_renderers[i];
        rectangle_component r_c = rectangles[i];

        if (!p_c.material) {
            continue;
        }

        ce_mat4_t scale = {};
        ce_mat4_scale(scale.m, r_c.half_size.x * 2, r_c.half_size.y * 2, 1.0f);

        ce_mat4_t w = {};
        ce_mat4_mul(w.m, scale.m, t_c.world.m);

        ct_gfx_a0->bgfx_set_transform(w.m, 1);
        ct_gfx_a0->bgfx_set_vertex_buffer(0, _G.cube_vbh, 0, CE_ARRAY_LEN(_vertices));
        ct_gfx_a0->bgfx_set_index_buffer(_G.cube_ibh, 0, CE_ARRAY_LEN(_indices));

        ct_material_a0->submit(p_c.material, data->layer_name, data->viewid);

//        ct_dd_a0->set_transform_mtx(t_c.world.m);
//        ct_dd_a0->draw_axis(0, 0, 0, 100.0f, CT_DD_AXIS_NONE, 0.0f);
//        ct_dd_a0->set_transform_mtx(NULL);
    }
}

static uint64_t rectangle_renderer_cdb_type() {
    return ce_id_a0->id64("rectangle_renderer");
}

static const char *rectangle_renderer_display_name() {
    return ICON_FA_HOUZZ " Rectangle renderer";
}

//static void draw_ui(ce_cdb_t0 db,
//                    uint64_t obj,
//                    uint64_t context,
//                    const char *filter) {
//    ct_debugui_a0->LabelText("ffffff", "%s","dddddddddddasdsadasdasdas");
//}

static struct ct_property_editor_i0 rectangle_renderer_property_editor_api = {
        .cdb_type = rectangle_renderer_cdb_type,
//        .draw_ui = draw_ui,
};

static void render(ct_world_t0 world,
                   struct ct_rg_builder_t0 *builder) {

    uint8_t viewid = builder->get_layer_viewid(builder, _GBUFFER);

    rectangle_render_data render_data = {
            .viewid = viewid,
            .layer_name = _GBUFFER,
    };

    ct_dd_a0->begin(viewid);

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(RECTANGLE_RENDERER_COMPONENT,
                                                                RECTANGLE_COMPONENT,
                                                                LOCAL_TO_WORLD_COMPONENT),
                                }, 0,
                                render_rectangles, &render_data);

    ct_dd_a0->end();
}


static struct ct_renderer_component_i0 rectangle_renderer_i = {
        .render = render
};

static void _rectangle_renderer_on_spawn(ct_world_t0 world,
                                         ce_cdb_t0 db,
                                         uint64_t obj,
                                         void *data) {
    rectangle_renderer_t *c = data;
    ce_cdb_a0->read_to(db, obj, c, sizeof(rectangle_renderer_t));
}

static struct ct_ecs_component_i0 ct_component_api = {
        .display_name = rectangle_renderer_display_name,
        .cdb_type = RECTANGLE_RENDERER_COMPONENT,
        .size = sizeof(rectangle_renderer_t),
        .from_cdb_obj = _rectangle_renderer_on_spawn,
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
    api->add_impl(CT_RENDERER_COMPONENT_I, &rectangle_renderer_i, sizeof(rectangle_renderer_i));
    api->add_impl(CT_PROPERTY_EDITOR_I, &rectangle_renderer_property_editor_api,
                  sizeof(rectangle_renderer_property_editor_api));

    ct_gfx_a0->bgfx_vertex_layout_begin(&pt_vertex_decl,
                                      ct_gfx_a0->bgfx_get_renderer_type());

    ct_gfx_a0->bgfx_vertex_layout_add(&pt_vertex_decl,
                                    BGFX_ATTRIB_POSITION, 3,
                                    BGFX_ATTRIB_TYPE_FLOAT, false, false);

    ct_gfx_a0->bgfx_vertex_layout_add(&pt_vertex_decl,
                                    BGFX_ATTRIB_TEXCOORD0, 2,
                                    BGFX_ATTRIB_TYPE_FLOAT, false, false);

    ct_gfx_a0->bgfx_vertex_layout_end(&pt_vertex_decl);

    _G.cube_vbh = ct_gfx_a0->bgfx_create_vertex_buffer(
            ct_gfx_a0->bgfx_make_ref(_vertices, sizeof(_vertices)),
            &pt_vertex_decl, BGFX_BUFFER_NONE);

    _G.cube_ibh = ct_gfx_a0->bgfx_create_index_buffer(
            ct_gfx_a0->bgfx_make_ref(_indices, sizeof(_indices)),
            BGFX_BUFFER_NONE);

    ce_cdb_a0->reg_obj_type(RECTANGLE_RENDERER_COMPONENT,
                            rectangle_render_prop, CE_ARRAY_LEN(rectangle_render_prop));
}

void CE_MODULE_UNLOAD(rectangle_render)(struct ce_api_a0 *api,
                                        int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    api->remove_impl(CT_RENDERER_COMPONENT_I, &rectangle_renderer_i);
    api->remove_impl(CT_PROPERTY_EDITOR_I, &rectangle_renderer_property_editor_api);
    api->remove_impl(CT_ECS_COMPONENT_I, &ct_component_api);
}