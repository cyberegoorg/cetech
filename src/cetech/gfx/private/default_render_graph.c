//==============================================================================
// includes
//==============================================================================

#include <corelib/allocator.h>
#include <corelib/api_system.h>
#include <corelib/memory.h>
#include <corelib/module.h>
#include <corelib/hashlib.h>
#include <cetech/gfx/renderer.h>
#include <cetech/resource/resource.h>
#include <cetech/gfx/material.h>
#include <cetech/gfx/debugui.h>
#include <cetech/ecs/ecs.h>
#include <cetech/gfx/render_graph.h>
#include <cetech/camera/camera.h>
#include <cetech/gfx/debugdraw.h>
#include <cetech/gfx/mesh_renderer.h>
#include <string.h>
#include <corelib/fmath.inl>
#include <corelib/macros.h>


#include "cetech/gfx/default_render_graph.h"


#define _G render_graph_global

//==============================================================================
// GLobals
//==============================================================================

static struct _G {
    struct ct_alloc *alloc;
} _G;

struct geometry_pass {
    struct ct_render_graph_pass pass;
    struct ct_world world;
};

#define _COLOR \
    CT_ID64_0("color", 0x6776ddaf0290228ULL)

#define _DEPTH \
     CT_ID64_0("depth", 0x911ffdcbfa56fec1ULL)

#define _DEFAULT \
     CT_ID64_0("default", 0xf27605035974b5ecULL)

//==============================================================================
// Geometry pass
//==============================================================================

static void geometry_pass_on_setup(void *inst,
                                   struct ct_render_graph_builder *builder) {
    builder->call->create(builder, _COLOR,
                          (struct ct_render_graph_attachment) {
                                  .format = CT_RENDER_TEXTURE_FORMAT_RGBA8,
                                  .ratio = CT_RENDER_BACKBUFFER_RATIO_EQUAL
                          }
    );

    builder->call->create(builder, _DEPTH,
                          (struct ct_render_graph_attachment) {
                                  .format = CT_RENDER_TEXTURE_FORMAT_D24,
                                  .ratio = CT_RENDER_BACKBUFFER_RATIO_EQUAL
                          }
    );

    builder->call->add_pass(builder, inst, _DEFAULT);
}

struct cameras {
    struct ct_camera_component camera_data[32];
    struct ct_entity ent[32];
    uint32_t n;
};

void foreach_camera(struct ct_world world,
                    struct ct_entity *ent,
                    ct_entity_storage_t *item,
                    uint32_t n,
                    void *data) {
    CT_UNUSED(world);

    struct cameras *cameras = data;

    struct ct_camera_component *camera_data;
    camera_data = ct_ecs_a0->component->get_all(CAMERA_COMPONENT, item);

    for (uint32_t i = 1; i < n; ++i) {
        uint32_t idx = cameras->n++;

        cameras->ent[idx].h = ent[i].h;

        memcpy(cameras->camera_data + idx,
               camera_data + i,
               sizeof(struct ct_camera_component));
    }
}

static void geometry_pass_on_pass(void *inst,
                                  uint8_t viewid,
                                  uint64_t layer,
                                  struct ct_render_graph_builder *builder) {
    struct geometry_pass *pass = inst;

    ct_renderer_a0->set_view_clear(viewid,
                                   CT_RENDER_CLEAR_COLOR |
                                   CT_RENDER_CLEAR_DEPTH,
                                   0x66CCFFff, 1.0f, 0);


    uint16_t size[2] = {0};
    builder->call->get_size(builder, size);

    ct_renderer_a0->set_view_rect(viewid, 0, 0, size[0], size[1]);

    struct cameras cameras;
    memset(&cameras, 0, sizeof(struct cameras));

    ct_ecs_a0->system->process(pass->world,
                               ct_ecs_a0->component->mask(CAMERA_COMPONENT),
                               foreach_camera, &cameras);

    ct_dd_a0->begin(viewid);
    {

        for (int i = 0; i < cameras.n; ++i) {
            float view_matrix[16];
            float proj_matrix[16];

            ct_camera_a0->get_project_view(pass->world,
                                           cameras.ent[i],
                                           proj_matrix,
                                           view_matrix,
                                           size[0],
                                           size[1]);

            ct_renderer_a0->set_view_transform(viewid, view_matrix,
                                               proj_matrix);

            ct_mesh_renderer_a0->render_all(pass->world, viewid, layer);
        }
    }
    ct_dd_a0->end();
}


//==============================================================================
// output pass
//==============================================================================
struct PosTexCoord0Vertex {
    float m_x;
    float m_y;
    float m_z;
    float m_u;
    float m_v;
};

static ct_render_vertex_decl_t ms_decl;

static void init_decl() {
    ct_renderer_a0->vertex_decl_begin(&ms_decl,
                                      ct_renderer_a0->get_renderer_type());
    ct_renderer_a0->vertex_decl_add(&ms_decl,
                                    CT_RENDER_ATTRIB_POSITION, 3,
                                    CT_RENDER_ATTRIB_TYPE_FLOAT, false, false);

    ct_renderer_a0->vertex_decl_add(&ms_decl,
                                    CT_RENDER_ATTRIB_TEXCOORD0, 2,
                                    CT_RENDER_ATTRIB_TYPE_FLOAT, false, false);

    ct_renderer_a0->vertex_decl_end(&ms_decl);
}


void screenspace_quad(float _textureWidth,
                      float _textureHeight,
                      float _texelHalf,
                      bool _originBottomLeft,
                      float _width,
                      float _height) {
    if (3 == ct_renderer_a0->get_avail_transient_vertex_buffer(3, &ms_decl)) {
        ct_render_transient_vertex_buffer_t vb;
        ct_renderer_a0->alloc_transient_vertex_buffer(&vb, 3, &ms_decl);
        struct PosTexCoord0Vertex *vertex = (struct PosTexCoord0Vertex *) vb.data;

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

        ct_renderer_a0->set_transient_vertex_buffer(0, &vb, 0, 3);
    }
}


static void output_pass_on_setup(void *inst,
                                 struct ct_render_graph_builder *builder) {

    builder->call->create(builder,
                          RG_OUTPUT_TEXTURE,
                          (struct ct_render_graph_attachment) {
                                  .format = CT_RENDER_TEXTURE_FORMAT_RGBA8,
                                  .ratio = CT_RENDER_BACKBUFFER_RATIO_EQUAL
                          }
    );

    builder->call->read(builder, _COLOR);

    builder->call->add_pass(builder, inst, _DEFAULT);
}

static uint64_t copy_material = 0;

static void output_pass_on_pass(void *inst,
                                uint8_t viewid,
                                uint64_t layer,
                                struct ct_render_graph_builder *builder) {
    ct_renderer_a0->set_view_clear(viewid,
                                   CT_RENDER_CLEAR_COLOR |
                                   CT_RENDER_CLEAR_DEPTH,
                                   0x66CCFFff, 1.0f, 0);

    uint16_t size[2] = {0};
    builder->call->get_size(builder, size);

    ct_renderer_a0->set_view_rect(viewid,
                                  0, 0,
                                  size[0], size[1]);

    float proj[16];
    ct_mat4_ortho(proj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f,
                  ct_renderer_a0->get_caps()->homogeneousDepth);

    ct_renderer_a0->set_view_transform(viewid, NULL, proj);

    if (!copy_material) {
        copy_material = ct_material_a0->create(
                ct_hashlib_a0->id64("content/copy"));
    }

    ct_render_texture_handle_t th;
    th = builder->call->get_texture(builder, _COLOR);

    ct_material_a0->set_texture_handler(copy_material,
                                        layer,
                                        "s_input_texture",
                                        th);

    screenspace_quad(size[0], size[1], 0,
                     ct_renderer_a0->get_caps()->originBottomLeft, 1.f, 1.0f);

    ct_material_a0->submit(copy_material, layer, viewid);
}

static struct ct_render_graph_module *create(struct ct_world world) {
    struct ct_render_graph_module *m1 = ct_render_graph_a0->create_module();

    m1->call->add_pass(m1, &(struct geometry_pass) {
            .world = world,
            .pass = (struct ct_render_graph_pass) {
                    .on_pass = geometry_pass_on_pass,
                    .on_setup = geometry_pass_on_setup
            }
    }, sizeof(struct geometry_pass));

    m1->call->add_pass(m1, &(struct ct_render_graph_pass) {
            .on_pass = output_pass_on_pass,
            .on_setup = output_pass_on_setup
    }, sizeof(struct ct_render_graph_pass));

    return m1;
}

static struct ct_default_rg_a0 default_render_graph_api = {
        .create= create,
};

struct ct_default_rg_a0 *ct_default_rg_a0 = &default_render_graph_api;


static void _init(struct ct_api_a0 *api) {
    CT_UNUSED(api);
    _G = (struct _G) {
            .alloc = ct_memory_a0->system,
    };

    init_decl();

    api->register_api("ct_default_rg_a0", &default_render_graph_api);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        default_render_graph,
        {
            CT_INIT_API(api, ct_hashlib_a0);
            CT_INIT_API(api, ct_memory_a0);
            CT_INIT_API(api, ct_renderer_a0);
            CT_INIT_API(api, ct_render_graph_a0);
            CT_INIT_API(api, ct_debugui_a0);
            CT_INIT_API(api, ct_ecs_a0);
            CT_INIT_API(api, ct_camera_a0);
            CT_INIT_API(api, ct_mesh_renderer_a0);
            CT_INIT_API(api, ct_dd_a0);
            CT_INIT_API(api, ct_material_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            _shutdown();
        }
)