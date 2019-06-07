//==============================================================================
// includes
//==============================================================================
#include <stdio.h>

#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/containers/array.h>

#include <celib/api.h>
#include <celib/config.h>
#include <celib/macros.h>
#include <celib/module.h>
#include <celib/memory/memory.h>
#include <celib/id.h>

#include <celib/private/api_private.inl>
#include <celib/cdb.h>

#include <celib/log.h>

#include <cetech/kernel/kernel.h>
#include <cetech/resource/resource.h>
#include <cetech/renderer/renderer.h>
#include <cetech/machine/machine.h>
#include <cetech/ecs/ecs.h>

#include <cetech/default_rg/default_rg.h>
#include <cetech/camera/camera.h>

#include <cetech/mesh/static_mesh.h>
#include <celib/task.h>

#include "bgfx/c99/bgfx.h"
#include "bgfx/c99/platform.h"

#include <cetech/debugui/debugui.h>

#include "cetech/render_graph/render_graph.h"
#include <cetech/game/game_system.h>
#include <cetech/renderer/gfx.h>
#include <celib/os/window.h>

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
} _G = {};


static uint32_t _get_reset_flags() {
    return (_G.capture ? BGFX_RESET_CAPTURE : 0) |
           (_G.vsync ? BGFX_RESET_VSYNC : 0);
}


//==============================================================================
// Interface
//==============================================================================

static void _render_task(void *data) {
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
    ce_api_entry_t0 it = ce_api_a0->first(CT_RENDERER_COMPONENT_I);
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
    ce_api_entry_t0 it = ce_api_a0->first(CT_RENDERER_COMPONENT_I);
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
    struct ct_rg_t0 *graph = ct_rg_a0->create_graph();
    struct ct_rg_module_t0 *module = ct_rg_a0->create_module();
    struct ct_rg_builder_t0 *builder = v->builder;

    builder->clear(builder);

    ct_default_rg_a0->feed_module(module);
    _feed_module(world, module);

    graph->set_module(graph, module);
    graph->setup(graph, v->builder);

    v->builder->execute(v->builder, &main_camera);

    _render_components(world, v->builder);

    ct_rg_a0->destroy_module(module);
    ct_rg_a0->destroy_graph(graph);
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

static struct ct_renderer_a0 rendderer_api = {
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

static uint64_t begin_task_name() {
    return CT_RENDER_BEGIN_TASK;
}

static uint64_t task_name() {
    return CT_RENDER_TASK;
}

static uint64_t *update_after(uint64_t *n) {
    static uint64_t a[] = {
            CT_RENDER_BEGIN_TASK,
    };

    *n = CE_ARRAY_LEN(a);
    return a;
}


static struct ct_kernel_task_i0 render_begin_task = {
        .name = begin_task_name,
        .update = render_begin,
};

static struct ct_kernel_task_i0 render_task = {
        .name = task_name,
        .update = render,
        .update_after = update_after,
};


#include "gfx.inl"
#include "c_viewport.inl"

void CE_MODULE_LOAD(renderer)(struct ce_api_a0 *api,
                              int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_config_a0);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ct_machine_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_ecs_a0);

    api->register_api(CT_RENDERER_API, &rendderer_api, sizeof(rendderer_api));
    api->register_api(CT_GFX_API, &gfx_api, sizeof(gfx_api));
    api->add_impl(CT_KERNEL_TASK_I, &render_task, sizeof(render_task));
    api->add_impl(CT_KERNEL_TASK_I, &render_begin_task, sizeof(render_begin_task));

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

    api->add_impl(CT_ECS_COMPONENT_I, &viewport_component_i, sizeof(viewport_component_i));
    ce_cdb_a0->reg_obj_type(VIEWPORT_COMPONENT, NULL, 0);

    renderer_create();

}

void CE_MODULE_UNLOAD(renderer)(struct ce_api_a0 *api,
                                int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    ce_array_free(_G.on_render, _G.allocator);
    bgfx_shutdown();

    _G = (struct _G) {};
}
