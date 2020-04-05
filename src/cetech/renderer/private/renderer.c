//==============================================================================
// includes
//==============================================================================
#include <stdio.h>

#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/containers/array.h>
#include <celib/api.h>
#include <celib/config.h>
#include <celib/module.h>
#include <celib/memory/memory.h>
#include <celib/id.h>
#include <celib/cdb.h>
#include <celib/log.h>
#include <celib/task.h>
#include <celib/os/window.h>

#include <cetech/kernel/kernel.h>
#include <cetech/asset/asset.h>
#include <cetech/renderer/renderer.h>
#include <cetech/machine/machine.h>
#include <cetech/ecs/ecs.h>
#include <cetech/default_rg/default_rg.h>
#include <cetech/camera/camera.h>

#include "bgfx/c99/bgfx.h"

#include <cetech/debugui/debugui.h>
#include "cetech/render_graph/render_graph.h"
#include <cetech/renderer/gfx.h>
#include <cetech/transform/transform.h>
#include <cetech/metrics/metrics.h>

//==============================================================================
// GLobals
//==============================================================================

typedef struct viewport_t {
    ct_rg_builder_t0 *builder;
    ct_rg_module *module;
    ce_vec2_t size;
    bool free;
} viewport_t;

#define _G RendererGlobals
static struct _G {
    ct_renderender_on_render *on_render;
    ce_window_t0 *main_window;

    viewport_t *viewports;

    uint64_t type;

    uint32_t size_width;
    uint32_t size_height;

    uint64_t viewid;

    bool capture;
    bool vsync;
    bool need_reset;
    ce_alloc_t0 *allocator;
    ct_machine_ev_queue_o0 *ev_queue;

    uint32_t render_worker_id;
} _G = {};


static uint32_t _get_reset_flags() {
    return (_G.capture ? BGFX_RESET_CAPTURE : 0) |
           (_G.vsync ? BGFX_RESET_VSYNC : 0);
}


//==============================================================================
// Interface
//==============================================================================

static void _render_task(void *data) {
    _G.render_worker_id = ce_task_a0->worker_id();

    ce_log_a0->info("renderer", "This is render worker.");
    while (bgfx_render_frame(-1) != BGFX_RENDER_FRAME_EXITING) {
    }
}

static void _render_init_task(void *data) {
    bgfx_render_frame(-1);

    ce_task_a0->add(&(ce_task_item_t0) {
            .work = _render_task,
            .name = "Renderer worker",
    }, 1, NULL);
}

static void renderer_create() {
    if (true) {
        uint32_t w, h;
        w = ce_config_a0->read_uint(CONFIG_SCREEN_X, 0);
        h = ce_config_a0->read_uint(CONFIG_SCREEN_Y, 0);
        _G.size_width = w;
        _G.size_height = h;

        intptr_t wid = ce_config_a0->read_uint(CONFIG_WID, 0);

        char title[128] = {};
        snprintf(title, CE_ARRAY_LEN(title), "cetech");

        if (wid == 0) {
            bool fullscreen = ce_config_a0->read_uint(CONFIG_SCREEN_FULLSCREEN, 0) > 0;

            uint32_t flags = WINDOW_NOFLAG;

            flags |= fullscreen ? WINDOW_FULLSCREEN : WINDOW_NOFLAG;
            flags |= WINDOW_RESIZABLE;

            _G.main_window = ce_os_window_a0->create(
                    title,
                    WINDOWPOS_UNDEFINED,
                    WINDOWPOS_UNDEFINED,
                    w, h,
                    flags, _G.allocator);

        } else {
            _G.main_window = ce_os_window_a0->create_from(
                    (void *) wid, _G.allocator);
        }
    }

    bgfx_platform_data_t pd = {NULL};
    pd.nwh = _G.main_window->native_window_ptr(_G.main_window->inst);
    pd.ndt = _G.main_window->native_display_ptr(_G.main_window->inst);
    bgfx_set_platform_data(&pd);

    ce_task_counter_t0 *render_init_c = NULL;
    ce_task_a0->add(&(ce_task_item_t0) {
            .work = _render_init_task,
            .name = "Renderer init worker",
    }, 1, &render_init_c);

    ce_task_a0->wait_for_counter_no_work(render_init_c, 0);
    // TODO: from config

    bgfx_init_t init;
    bgfx_init_ctor(&init);

    const char *rtype = ce_config_a0->read_str(CONFIG_RENDERER_TYPE, "");

    bool invalid = true;

    static struct {
        const char *k;
        bgfx_renderer_type_t v;
    } _str_to_render_type[] = {
            {.k = "noop", .v = BGFX_RENDERER_TYPE_NOOP},
            {.k = "opengl", .v = BGFX_RENDERER_TYPE_OPENGL},
            {.k = "metal", .v = BGFX_RENDERER_TYPE_METAL},

            {.k = "",
#if CE_PLATFORM_LINUX
                    .v = BGFX_RENDERER_TYPE_OPENGL
#elif CE_PLATFORM_OSX
                    .v = BGFX_RENDERER_TYPE_METAL
#endif
            },
    };

    for (int i = 0; i < CE_ARRAY_LEN(_str_to_render_type); ++i) {
        if (strcmp(rtype, _str_to_render_type[i].k) == 0) {
            init.type = _str_to_render_type[i].v;
            invalid = false;
            break;
        }
    }

    if (invalid) {
        ce_log_a0->error("renderer", "Invalid render type '%s', force to noop",
                         rtype);
        init.type = BGFX_RENDERER_TYPE_NOOP;
    }


    bgfx_init(&init);

    _G.main_window->size(_G.main_window->inst, &_G.size_width, &_G.size_height);
    bgfx_reset(_G.size_width, _G.size_height, _get_reset_flags(), BGFX_TEXTURE_FORMAT_COUNT);
    //_G.main_window->update(_G.main_window);

    _G.need_reset = true;
}


static void renderer_set_debug(int debug) {
    if (debug) {
        bgfx_set_debug(BGFX_DEBUG_STATS);
    } else {
        bgfx_set_debug(BGFX_DEBUG_NONE);
    }
}


static void renderer_get_size(uint32_t *width,
                              uint32_t *height) {
    *width = _G.size_width;
    *height = _G.size_height;
}

static void _feed_module(ct_world_t0 world,
                         struct ct_rg_module_t0 *module) {
    ce_api_entry_t0 it = ce_api_a0->first(CT_RENDERER_COMPONENT_I0);
    while (it.api) {
        struct ct_renderer_component_i0 *i = (ct_renderer_component_i0 *) (it.api);

        if (i && i->feed_module) {
            i->feed_module(world, module);
        }

        it = ce_api_a0->next(it);
    }
}

void _render_components(ct_world_t0 world,
                        struct ct_rg_builder_t0 *builder) {
    ce_api_entry_t0 it = ce_api_a0->first(CT_RENDERER_COMPONENT_I0);
    while (it.api) {
        ct_renderer_component_i0 *ci = (ct_renderer_component_i0 *) (it.api);

        if (ci && ci->render) {
            ci->render(world, builder);
        }

        it = ce_api_a0->next(it);
    }
}

static void _render_viewport(viewport_t *v,
                             ct_world_t0 world,
                             ct_camera_data_t0 main_camera) {
    ct_rg_module_t0 *module = ct_rg_a0->create_module();
    ct_rg_builder_t0 *builder = v->builder;

    builder->clear(builder);

    ct_default_rg_a0->feed_module(module);
    _feed_module(world, module);

    module->on_setup(module->inst, builder);

    v->builder->execute(v->builder, &main_camera);

    _render_components(world, v->builder);

    ct_rg_a0->destroy_module(module);
}

static void render_begin(float dt) {
    _G.viewid = 0;

    ct_machine_ev_t0 ev = {};
    while (ct_machine_a0->pop_ev(_G.ev_queue, &ev)) {
        if (ev.ev_type != EVENT_WINDOW_RESIZED) {
            continue;
        }
        _G.need_reset = 1;
        _G.size_width = ev.window_resize.width;
        _G.size_height = ev.window_resize.height;
    }

    if (_G.need_reset) {
        _G.need_reset = 0;

        bgfx_reset(_G.size_width, _G.size_height, _get_reset_flags(), BGFX_TEXTURE_FORMAT_COUNT);
    }
}

static void render(float dt) {
    ct_debugui_a0->render();

    const bgfx_stats_t *s = bgfx_get_stats();

    double freq = s->cpuTimerFreq;

    ct_metrics_a0->set_float(ce_id_a0->id64("renderer.wait_submit"),
                             1000 * s->waitSubmit / freq);

    ct_metrics_a0->set_float(ce_id_a0->id64("renderer.wait_render"),
                             1000 * s->waitRender / freq);

    ct_metrics_a0->set_float(ce_id_a0->id64("renderer.num_draw"),
                             s->numDraw);

    bgfx_frame(false);
}

uint64_t new_viewid() {
    return _G.viewid++;
}

static uint32_t _new_viewport() {
    const uint32_t n = ce_array_size(_G.viewports);
    for (uint32_t i = 0; i < n; ++i) {
        viewport_t *v = &_G.viewports[i];

        if (!v->free) {
            continue;
        }

        return i;
    }

    uint32_t idx = n;
    ce_array_push(_G.viewports, (viewport_t) {}, _G.allocator);
    return idx;
}

struct ct_viewport_t0 create_viewport() {
    uint32_t idx = _new_viewport();

    viewport_t *v = &_G.viewports[idx];

    ct_rg_builder_t0 *builder = ct_rg_a0->create_builder();

    v->builder = builder;

    return (ct_viewport_t0) {.idx=idx};
}

ce_vec2_t viewport_size(ct_viewport_t0 viewport) {
    viewport_t *v = &_G.viewports[viewport.idx];
    return v->size;

}

void viewport_set_size(ct_viewport_t0 viewport,
                       ce_vec2_t size) {
    viewport_t *v = &_G.viewports[viewport.idx];
    v->size = size;
    v->builder->set_size(v->builder, size.x, size.y);
}

void destroy_viewport(ct_viewport_t0 viewport) {
    viewport_t *v = &_G.viewports[viewport.idx];
    ct_rg_a0->destroy_builder(v->builder);
    v->free = true;
}

struct ct_rg_builder_t0 *viewport_builder(ct_viewport_t0 viewport) {
    return _G.viewports[viewport.idx].builder;
}

void viewport_render(ct_viewport_t0 viewport,
                     ct_world_t0 world,
                     ct_camera_data_t0 main_camera) {
    _render_viewport(&_G.viewports[viewport.idx], world, main_camera);
}

struct ce_window_t0 *get_main_window() {
    return _G.main_window;
}

uint32_t render_worker_id() {
    return _G.render_worker_id;
}

static struct ct_renderer_a0 rendderer_api = {
        .render_worker_id = render_worker_id,
        .create = renderer_create,
        .set_debug = renderer_set_debug,
        .get_size = renderer_get_size,
        .new_viewid = new_viewid,
        .create_viewport = create_viewport,
        .viewport_size = viewport_size,
        .viewport_set_size = viewport_set_size,
        .destroy_viewport = destroy_viewport,
        .viewport_builder = viewport_builder,
        .viewport_render = viewport_render,
        .get_main_window = get_main_window,
};

struct ct_renderer_a0 *ct_renderer_a0 = &rendderer_api;

static struct ct_kernel_task_i0 render_begin_task = {
        .name = CT_RENDER_BEGIN_TASK,
        .update = render_begin,
};

static struct ct_kernel_task_i0 render_task = {
        .name = CT_RENDER_TASK,
        .update = render,
        .update_after = CT_KERNEL_AFTER(CT_RENDER_BEGIN_TASK)
};

void _render_all_viewport(ct_world_t0 w,
                          ct_entity_t0 *entities,
                          ct_ecs_ent_chunk_o0 *item,
                          uint32_t n,
                          void *_data) {

    viewport_component *viewport = ct_ecs_c_a0->get_all(w, VIEWPORT_COMPONENT, item);
    ct_local_to_world_c *ltw = ct_ecs_c_a0->get_all(w, LOCAL_TO_WORLD_COMPONENT, item);
    ct_camera_component *camera = ct_ecs_c_a0->get_all(w, CT_CAMERA_COMPONENT, item);

    for (int i = 0; i < n; ++i) {
        viewport_render(viewport[i].viewport,
                        w,
                        (ct_camera_data_t0) {
                                .world = ltw[i].world,
                                .camera = camera[i],
                        });
    }
}

static void render_world(ct_world_t0 world,
                         float dt,
                         uint32_t rq_version,
                         ct_ecs_cmd_buffer_t *cmd) {
    ct_ecs_q_a0->foreach(world,
                         (ct_ecs_query_t0) {
                                 .all = CT_ECS_ARCHETYPE(VIEWPORT_COMPONENT,
                                                         LOCAL_TO_WORLD_COMPONENT,
                                                         CT_CAMERA_COMPONENT)
                         }, rq_version,
                         _render_all_viewport, NULL);
}

static struct ct_system_i0 render_system = {
        .name = CT_RENDERER_SYSTEM,
        .group = CT_ECS_PRESENTATION_GROUP,
        .process = render_world,
};

#include "gfx.inl"
#include "c_viewport.inl"

void CE_MODULE_LOAD(renderer)(struct ce_api_a0 *api,
                              int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_config_a0);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_asset_a0);
    CE_INIT_API(api, ct_machine_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_ecs_a0);

    api->add_api(CT_RENDERER_A0_STR, &rendderer_api, sizeof(rendderer_api));
    api->add_api(CT_GFX_A0_STR, &gfx_api, sizeof(gfx_api));
    api->add_impl(CT_KERNEL_TASK_I0_STR, &render_task, sizeof(render_task));
    api->add_impl(CT_KERNEL_TASK_I0_STR, &render_begin_task, sizeof(render_begin_task));

    ce_api_a0 = api;

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .ev_queue = ct_machine_a0->new_ev_listener(),
    };


    if (!ce_config_a0->exist(CONFIG_SCREEN_X)) {
        ce_config_a0->set_uint(CONFIG_SCREEN_X, 1024);
    }

    if (!ce_config_a0->exist(CONFIG_SCREEN_Y)) {
        ce_config_a0->set_uint(CONFIG_SCREEN_Y, 768);
    }

    if (!ce_config_a0->exist(CONFIG_SCREEN_FULLSCREEN)) {
        ce_config_a0->set_uint(CONFIG_SCREEN_FULLSCREEN, 0);
    }

    if (!ce_config_a0->exist(CONFIG_WID)) {
        ce_config_a0->set_uint(CONFIG_WID, 0);
    }

    if (!ce_config_a0->exist(CONFIG_RENDERER_TYPE)) {
        ce_config_a0->set_str(CONFIG_RENDERER_TYPE, "");
    }

    _G.vsync = ce_config_a0->read_uint(CONFIG_SCREEN_VSYNC, 1) > 0;

    api->add_impl(CT_ECS_SYSTEM_I0_STR,
                  &render_system, sizeof(render_system));

    api->add_impl(CT_ECS_COMPONENT_I0_STR, &viewport_component_i, sizeof(viewport_component_i));

    ce_cdb_a0->reg_obj_type(VIEWPORT_COMPONENT, NULL, 0);

    renderer_create();

    ct_metrics_a0->reg_float_metric("renderer.wait_submit");
    ct_metrics_a0->reg_float_metric("renderer.wait_render");
    ct_metrics_a0->reg_float_metric("renderer.num_draw");
}

void CE_MODULE_UNLOAD(renderer)(struct ce_api_a0 *api,
                                int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    ce_array_free(_G.on_render, _G.allocator);
    bgfx_shutdown();

    _G = (struct _G) {};
}
