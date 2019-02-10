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
#include <cetech/mesh/mesh_renderer.h>
#include <string.h>
#include <celib/math/math.h>
#include <celib/macros.h>


#include "cetech/default_rg/default_rg.h"


#define _G render_graph_global

//==============================================================================
// GLobals
//==============================================================================

static struct _G {
    struct ce_alloc_t0 *alloc;
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


void screenspace_quad(float _textureWidth,
                      float _textureHeight,
                      float _texelHalf,
                      bool _originBottomLeft,
                      float _width,
                      float _height) {
    if (3 == ct_gfx_a0->bgfx_get_avail_transient_vertex_buffer(3, &ms_decl)) {
        bgfx_transient_vertex_buffer_t vb;
        ct_gfx_a0->bgfx_alloc_transient_vertex_buffer(&vb, 3, &ms_decl);
        struct PosTexCoord0Vertex *vertex = (PosTexCoord0Vertex *) vb.data;

        const float minx = -_width;
        const float maxx = _width;
        const float miny = 0.0f;
        const float maxy = _height * 2.0f;

        const float texelHalfW = _texelHalf / _textureWidth;
        const float texelHalfH = _texelHalf / _textureHeight;
        const float minu = -1.0f + texelHalfW;
        const float maxu = 1.0f + texelHalfH;

        const float zz = 0.0f;

        float minv = texelHalfH;
        float maxv = 2.0f + texelHalfH;

        if (_originBottomLeft) {
            float temp = minv;
            minv = maxv;
            maxv = temp;

            minv -= 1.0f;
            maxv -= 1.0f;
        }

        vertex[0].m_x = minx;
        vertex[0].m_y = miny;
        vertex[0].m_z = zz;
        vertex[0].m_u = minu;
        vertex[0].m_v = minv;

        vertex[1].m_x = maxx;
        vertex[1].m_y = miny;
        vertex[1].m_z = zz;
        vertex[1].m_u = maxu;
        vertex[1].m_v = minv;

        vertex[2].m_x = maxx;
        vertex[2].m_y = maxy;
        vertex[2].m_z = zz;
        vertex[2].m_u = maxu;
        vertex[2].m_v = maxv;

        ct_gfx_a0->bgfx_set_transient_vertex_buffer(0, &vb, 0, 3);
    }
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

static uint64_t copy_material = 0;

static void output_pass_on_pass(void *inst,
                                uint8_t viewid,
                                uint64_t layer,
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

    if (!copy_material) {
        copy_material = ct_material_a0->create(0xe27880f9fbb28b8d);
    }

    bgfx_texture_handle_t th;
    th = builder->get_texture(builder, _COLOR);

    ct_material_a0->set_texture_handler(copy_material,
                                        _DEFAULT,
                                        "s_input_texture",
                                        th);

    screenspace_quad(size[0], size[1], 0,
                     ct_gfx_a0->bgfx_get_caps()->originBottomLeft, 1.f, 1.0f);

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
    struct ct_rg_pass_t0 pass;
    struct ct_entity_t0 camera;
    ct_world_t0 world;
} gbuffer_pass;

static void gbuffer_pass_on_pass(void *inst,
                                 uint8_t viewid,
                                 uint64_t layer,
                                 struct ct_rg_builder_t0 *builder) {
    struct gbuffer_pass *pass = inst;

    ct_gfx_a0->bgfx_set_view_clear(viewid,
                                   BGFX_CLEAR_COLOR |
                                   BGFX_CLEAR_DEPTH,
                                   0x66CCFFff, 1.0f, 0);

    uint16_t size[2] = {};
    builder->get_size(builder, size);

    ct_gfx_a0->bgfx_set_view_rect(viewid, 0, 0, size[0], size[1]);

    float view_matrix[16];
    float proj_matrix[16];

    ct_camera_a0->get_project_view(pass->world,
                                   pass->camera,
                                   proj_matrix,
                                   view_matrix,
                                   size[0],
                                   size[1]);

    ct_gfx_a0->bgfx_set_view_transform(viewid, view_matrix,
                                       proj_matrix);
}


static void feed_module(ct_rg_module *m1,
                        ct_world_t0 world,
                        struct ct_entity_t0 camera) {
    struct ct_rg_module *gm = m1->add_extension_point(m1, _GBUFFER);

    gm->add_pass(gm, &(gbuffer_pass) {
            .camera = camera,
            .world = world,
            .pass = {
                    .on_setup = gbuffer_pass_on_setup,
                    .on_pass = gbuffer_pass_on_pass,
            }
    }, sizeof(gbuffer_pass));

    m1->add_pass(m1, &(ct_rg_pass_t0) {
            .on_pass = output_pass_on_pass,
            .on_setup = output_pass_on_setup
    }, sizeof(ct_rg_pass_t0));
}

static struct ct_default_rg_a0 default_render_graph_api = {
        .feed_module= feed_module,
};

struct ct_default_rg_a0 *ct_default_rg_a0 = &default_render_graph_api;


static void _init(struct ce_api_a0 *api) {
    CE_UNUSED(api);
    _G = (struct _G) {
            .alloc = ce_memory_a0->system,
    };

    init_decl();

    api->register_api(CT_DEFAULT_RG_API, &default_render_graph_api,
                      sizeof(default_render_graph_api));
}

static void _shutdown() {
    _G = (struct _G) {};
}

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
    _init(api);
}

void CE_MODULE_UNLOAD(default_render_graph)(struct ce_api_a0 *api,
                                            int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _shutdown();
}
