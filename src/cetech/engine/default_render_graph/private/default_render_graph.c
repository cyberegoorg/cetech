//==============================================================================
// includes
//==============================================================================

#include <cetech/kernel/memory/allocator.h>
#include <cetech/kernel/api/api_system.h>
#include <cetech/kernel/memory/memory.h>
#include <cetech/kernel/module/module.h>
#include <cetech/kernel/hashlib/hashlib.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/ecs/ecs.h>
#include <cetech/engine/render_graph/render_graph.h>
#include <cetech/engine/camera/camera.h>
#include <cetech/engine/mesh_renderer/mesh_renderer.h>
#include <string.h>


#include "../default_render_graph.h"

CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_render_graph_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_ecs_a0);
CETECH_DECL_API(ct_camera_a0);
CETECH_DECL_API(ct_mesh_renderer_a0);

#define _G render_graph_global

//==============================================================================
// GLobals
//==============================================================================

static struct _G {
    struct ct_alloc *alloc;

} _G;

static void geometry_pass_on_setup(void *inst,
                                    struct ct_render_graph_builder *builder) {
    builder->call->write(builder, CT_ID64_0("color"),
                         CT_RENDER_BACKBUFFER_RATIO_EQUAL,
                         CT_RENDER_TEXTURE_FORMAT_RGBA8);

    builder->call->write(builder, CT_ID64_0("depth"),
                         CT_RENDER_BACKBUFFER_RATIO_EQUAL,
                         CT_RENDER_TEXTURE_FORMAT_D24);

    builder->call->add_pass(builder, inst, CT_ID64_0("default"));
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
    struct cameras *cameras = data;

    struct ct_camera_component *camera_data;
    camera_data = ct_ecs_a0.component_data(CAMERA_COMPONENT, item);

    for (int i = 1; i < n; ++i) {
        uint32_t idx = cameras->n++;

        cameras->ent[idx].h = ent[i].h;

        memcpy(cameras->camera_data + idx,
               camera_data + i,
               sizeof(struct ct_camera_component));
    }
}


//static void render_simu(struct ct_world world,
//                        float dt) {
//

//}

static void geometry_pass_on_pass(void *inst,
                                  uint8_t viewid,
                                  uint64_t layer,
                                  struct ct_render_graph_builder *builder) {
    struct ct_render_graph_pass* pass = inst;

    struct ct_world world = (struct ct_world) {.h = (uint64_t) pass->inst};

    ct_renderer_a0.set_view_clear(viewid,
                                  CT_RENDER_CLEAR_COLOR | CT_RENDER_CLEAR_DEPTH,
                                  0x66CCFFff, 1.0f, 0);

    uint16_t size[2] = {0};
    builder->call->get_size(builder, size);

    ct_renderer_a0.set_view_rect(viewid, 0, 0,
                                 size[0],
                                 size[1]);

    struct cameras cameras = {{{0}}};

    ct_ecs_a0.process(world,
                      ct_ecs_a0.component_mask(CAMERA_COMPONENT),
                      foreach_camera, &cameras);


    for (int i = 0; i < cameras.n; ++i) {
        float view_matrix[16];
        float proj_matrix[16];

        ct_camera_a0.get_project_view(world,
                                      cameras.ent[i],
                                      proj_matrix,
                                      view_matrix,
                                      size[0],
                                      size[1]);

        ct_renderer_a0.set_view_transform(viewid, view_matrix, proj_matrix);
        ct_mesh_renderer_a0.render_all(world, viewid,
                                       layer);
    }
}

static struct ct_render_graph_pass_fce pass1_fce = {
        .on_pass = geometry_pass_on_pass,
        .on_setup = geometry_pass_on_setup
};

static struct ct_render_graph_module *create(struct ct_world world) {
    struct ct_render_graph_pass *pass1 = CT_ALLOC(_G.alloc,
                                                  struct ct_render_graph_pass,
                                                  sizeof(struct ct_render_graph_pass));

    *pass1 = (struct ct_render_graph_pass) {
            .call = &pass1_fce,
            .inst = (void *) world.h
    };

    struct ct_render_graph_module *m1 = ct_render_graph_a0.create_module();

    m1->call->add_pass(m1, pass1);

    return m1;
}

static struct ct_default_render_graph_a0 default_render_graph_api = {
        .create= create,
};


static void _init(struct ct_api_a0 *api) {
    CT_UNUSED(api);
    _G = (struct _G) {
            .alloc = ct_memory_a0.main_allocator(),
    };

    api->register_api("ct_default_render_graph_a0", &default_render_graph_api);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        default_render_graph,
        {
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_render_graph_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_camera_a0);
            CETECH_GET_API(api, ct_mesh_renderer_a0);
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