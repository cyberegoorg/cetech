//==============================================================================
// includes
//==============================================================================

#include <celib/memory/allocator.h>
#include <celib/api.h>
#include <celib/memory/memory.h>
#include <celib/module.h>
#include <celib/id.h>
#include <celib/macros.h>
#include <celib/containers/array.h>
#include <celib/containers/hash.h>


#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>

#include <cetech/ecs/ecs.h>
#include <cetech/transform/transform.h>
#include <celib/cdb.h>
#include "cetech/render_graph/render_graph.h"


#define _G render_graph_global

//==============================================================================
// GLobals
//==============================================================================

typedef struct render_graph_module_inst {
    uint8_t *pass;
    ct_rg_module_t0 **modules;
    ce_hash_t extension_points;
} render_graph_module_inst;

typedef struct render_graph_builder_pass {
    ct_rg_pass_t0 *pass;
    uint8_t viewid;
    uint64_t layer;
    bgfx_frame_buffer_handle_t fb;
} render_graph_builder_pass;

#define MAX_ATTACHMENTS 8+2
typedef struct render_graph_builder_inst {
    render_graph_builder_pass *pass;

    ce_hash_t texture_map;

    ce_hash_t layer_map;

    uint16_t size[2];

    uint8_t attachemnt_used;
    bgfx_texture_handle_t attachemnt[MAX_ATTACHMENTS];
} render_graph_builder_inst;

static struct _G {
    ce_alloc_t0 *alloc;

    ct_rg_module_t0 **module_free;
    render_graph_module_inst **module_inst_free;

    ct_rg_builder_t0 **builder_free;
    render_graph_builder_inst **builder_inst_free;
} _G;

#include "celib_module.inl"
#include "render_builder.inl"

typedef struct PosTexCoord0Vertex {
    float x;
    float y;
    float z;
    float u;
    float v;
} PosTexCoord0Vertex;
static bgfx_vertex_decl_t pt_decl;

static void init_decl() {
    ct_gfx_a0->bgfx_vertex_decl_begin(&pt_decl,
                                      ct_gfx_a0->bgfx_get_renderer_type());
    ct_gfx_a0->bgfx_vertex_decl_add(&pt_decl,
                                    BGFX_ATTRIB_POSITION, 3,
                                    BGFX_ATTRIB_TYPE_FLOAT, false, false);

    ct_gfx_a0->bgfx_vertex_decl_add(&pt_decl,
                                    BGFX_ATTRIB_TEXCOORD0, 2,
                                    BGFX_ATTRIB_TYPE_FLOAT, false, false);

    ct_gfx_a0->bgfx_vertex_decl_end(&pt_decl);
}

void screenspace_quad(float texture_width,
                      float texture_height,
                      float texel_half,
                      float width,
                      float height) {
    if (3 == ct_gfx_a0->bgfx_get_avail_transient_vertex_buffer(3, &pt_decl)) {
        bgfx_transient_vertex_buffer_t vb;
        ct_gfx_a0->bgfx_alloc_transient_vertex_buffer(&vb, 3, &pt_decl);
        struct PosTexCoord0Vertex *vertex = (PosTexCoord0Vertex *) vb.data;

        const float minx = -width;
        const float maxx = width;
        const float miny = 0.0f;
        const float maxy = height * 2.0f;

        const float texelHalfW = texel_half / texture_width;
        const float texelHalfH = texel_half / texture_height;
        const float minu = -1.0f + texelHalfW;
        const float maxu = 1.0f + texelHalfH;

        const float zz = 0.0f;

        float minv = texelHalfH;
        float maxv = 2.0f + texelHalfH;

        if (ct_gfx_a0->bgfx_get_caps()->homogeneousDepth) {
            float temp = minv;
            minv = maxv;
            maxv = temp;

            minv -= 1.0f;
            maxv -= 1.0f;
        }

        vertex[0].x = minx;
        vertex[0].y = miny;
        vertex[0].z = zz;
        vertex[0].u = minu;
        vertex[0].v = minv;

        vertex[1].x = maxx;
        vertex[1].y = miny;
        vertex[1].z = zz;
        vertex[1].u = maxu;
        vertex[1].v = minv;

        vertex[2].x = maxx;
        vertex[2].y = maxy;
        vertex[2].z = zz;
        vertex[2].u = maxu;
        vertex[2].v = maxv;

        ct_gfx_a0->bgfx_set_transient_vertex_buffer(0, &vb, 0, 3);
    }
}

static struct ct_rg_a0 render_graph_api = {
        .create_module = create_module,
        .destroy_module = destroy_module,
        .create_builder = create_render_builder,
        .destroy_builder = destroy_render_builder,
        .screenspace_quad = screenspace_quad,
};


struct ct_rg_a0 *ct_rg_a0 = &render_graph_api;

void CE_MODULE_LOAD(render_graph)(struct ce_api_a0 *api,
                                  int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_renderer_a0);

    _G = (struct _G) {
            .alloc = ce_memory_a0->system,
    };

    api->add_api(CT_RG_API, &render_graph_api, sizeof(render_graph_api));

    init_decl();
}

void CE_MODULE_UNLOAD(render_graph)(struct ce_api_a0 *api,
                                    int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    uint32_t module_free_n = ce_array_size(_G.module_free);
    for (int i = 0; i < module_free_n; ++i) {
        CE_FREE(_G.alloc, _G.module_free[i]);
    }

    uint32_t module_inst_free_n = ce_array_size(_G.module_inst_free);
    for (int i = 0; i < module_inst_free_n; ++i) {
        _free_render_graph_module_inst(_G.module_inst_free[i], _G.alloc);
    }

    uint32_t builder_free_n = ce_array_size(_G.builder_free);
    for (int i = 0; i < builder_free_n; ++i) {
        CE_FREE(_G.alloc, _G.builder_free[i]);
    }

    uint32_t builder_inst_n = ce_array_size(_G.builder_inst_free);
    for (int i = 0; i < builder_inst_n; ++i) {
        _free_render_graph_builder_inst(_G.builder_inst_free[i],  _G.alloc);
    }

    ce_array_free(_G.module_free, _G.alloc);
    ce_array_free(_G.module_inst_free, _G.alloc);
    ce_array_free(_G.builder_free, _G.alloc);
    ce_array_free(_G.builder_inst_free, _G.alloc);

    _G = (struct _G) {};
}
