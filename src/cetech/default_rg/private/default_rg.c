//==============================================================================
// includes
//==============================================================================

#include <celib/memory/allocator.h>
#include <celib/api.h>
#include <celib/memory/memory.h>
#include <celib/module.h>
#include <celib/id.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/resource/resource.h>
#include <cetech/material/material.h>
#include <cetech/debugui/debugui.h>
#include <cetech/ecs/ecs.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/camera/camera.h>
#include <cetech/mesh/static_mesh.h>
#include <string.h>
#include <celib/math/math.h>
#include <celib/macros.h>


#include "cetech/default_rg/default_rg.h"


#define _G render_graph_global

//==============================================================================
// GLobals
//==============================================================================

static struct _G {
    ce_alloc_t0 *alloc;
} _G;

//==============================================================================
// output pass
//==============================================================================
typedef struct PosTexCoord0Vertex {
    float m_x;
    float m_y;
    float m_z;
    float m_u;
    float m_v;
} PosTexCoord0Vertex;

static bgfx_vertex_decl_t ms_decl;

static void init_decl() {
    ct_gfx_a0->bgfx_vertex_decl_begin(&ms_decl,
                                      ct_gfx_a0->bgfx_get_renderer_type());
    ct_gfx_a0->bgfx_vertex_decl_add(&ms_decl,
                                    BGFX_ATTRIB_POSITION, 3,
                                    BGFX_ATTRIB_TYPE_FLOAT, false, false);

    ct_gfx_a0->bgfx_vertex_decl_add(&ms_decl,
                                    BGFX_ATTRIB_TEXCOORD0, 2,
                                    BGFX_ATTRIB_TYPE_FLOAT, false, false);

    ct_gfx_a0->bgfx_vertex_decl_end(&ms_decl);
}





static void output_pass_on_setup(void *inst,
                                 struct ct_rg_builder_t0 *builder) {

    builder->create(builder,
                    RG_OUTPUT_TEXTURE,
                    (ct_rg_attachment_t0) {
                            .format = BGFX_TEXTURE_FORMAT_RGB8,
                            .ratio = BGFX_BACKBUFFER_RATIO_EQUAL
                    }
    );

    builder->read(builder, _COLOR);
    builder->add_pass(builder, inst, 0);
}


static void output_pass_on_pass(void *inst,
                                uint8_t viewid,
                                uint64_t layer,
                                ct_camera_data_t0 *main_camera,
                                struct ct_rg_builder_t0 *builder) {

    ct_gfx_a0->bgfx_set_view_clear(viewid,
                                   BGFX_CLEAR_COLOR |
                                   BGFX_CLEAR_DEPTH,
                                   0x66CCFFff, 1.0f, 0);

    uint16_t size[2] = {};
    builder->get_size(builder, size);

    ct_gfx_a0->bgfx_set_view_rect(viewid,
                                  0, 0,
                                  size[0], size[1]);

    float proj[16];
    ce_mat4_ortho(proj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f,
                  ct_gfx_a0->bgfx_get_caps()->homogeneousDepth);

    ct_gfx_a0->bgfx_set_view_transform(viewid, NULL, proj);

    uint64_t copy_material = 0xe27880f9fbb28b8d;

    bgfx_texture_handle_t th;
    th = builder->get_texture(builder, _COLOR);

    ct_material_a0->set_texture_handler(copy_material,
                                        _DEFAULT,
                                        "s_input_texture",
                                        th);

    ct_rg_a0->screenspace_quad(size[0], size[1], 0, 1.f, 1.0f);

    ct_material_a0->submit(copy_material, _DEFAULT, viewid);
}

static void gbuffer_pass_on_setup(void *inst,
                                  struct ct_rg_builder_t0 *builder) {

    builder->create(builder, _COLOR,
                    (ct_rg_attachment_t0) {
                            .format = BGFX_TEXTURE_FORMAT_RGB8,
                            .ratio = BGFX_BACKBUFFER_RATIO_EQUAL
                    }
    );

    builder->create(builder, _DEPTH,
                    (ct_rg_attachment_t0) {
                            .format = BGFX_TEXTURE_FORMAT_D24,
                            .ratio = BGFX_BACKBUFFER_RATIO_EQUAL
                    }
    );

    builder->add_pass(builder, inst, _GBUFFER);
}

typedef struct gbuffer_pass {
    ct_rg_pass_t0 pass;
} gbuffer_pass;

static void gbuffer_pass_on_pass(void *inst,
                                 uint8_t viewid,
                                 uint64_t layer,
                                 ct_camera_data_t0 *main_camera,
                                 ct_rg_builder_t0 *builder) {

    ct_gfx_a0->bgfx_set_view_clear(viewid,
                                   BGFX_CLEAR_COLOR |
                                   BGFX_CLEAR_DEPTH,
                                   0x66CCFFff, 1.0f, 0);

    uint16_t size[2] = {};
    builder->get_size(builder, size);

    ct_gfx_a0->bgfx_set_view_rect(viewid, 0, 0, size[0], size[1]);

    float view_matrix[16];
    float proj_matrix[16];

    ct_camera_a0->get_project_view(main_camera->world,
                                   main_camera->camera,
                                   proj_matrix,
                                   view_matrix,
                                   size[0],
                                   size[1]);

    ct_gfx_a0->bgfx_set_view_transform(viewid, view_matrix,
                                       proj_matrix);
}


static void feed_module(ct_rg_module_t0 *m1) {
    ct_rg_module_t0 *gm = m1->add_extension_point(m1->inst, _GBUFFER);

    gm->add_pass(gm->inst, &(gbuffer_pass) {
            .pass = {
                    .on_setup = gbuffer_pass_on_setup,
                    .on_pass = gbuffer_pass_on_pass,
            }
    }, sizeof(gbuffer_pass));

    m1->add_pass(m1->inst, &(ct_rg_pass_t0) {
            .on_pass = output_pass_on_pass,
            .on_setup = output_pass_on_setup
    }, sizeof(ct_rg_pass_t0));
}

static struct ct_default_rg_a0 default_render_graph_api = {
        .feed_module= feed_module,
};

struct ct_default_rg_a0 *ct_default_rg_a0 = &default_render_graph_api;

void CE_MODULE_LOAD(default_render_graph)(struct ce_api_a0 *api,
                                          int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_renderer_a0);
    CE_INIT_API(api, ct_rg_a0);
    CE_INIT_API(api, ct_debugui_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ct_camera_a0);
    CE_INIT_API(api, ct_material_a0);
    _G = (struct _G) {
            .alloc = ce_memory_a0->system,
    };

    init_decl();

    api->register_api(CT_DEFAULT_RG_API, &default_render_graph_api,
                      sizeof(default_render_graph_api));
}

void CE_MODULE_UNLOAD(default_render_graph)(struct ce_api_a0 *api,
                                            int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _G = (struct _G) {};
}
