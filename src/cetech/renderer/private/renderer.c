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

#include <cetech/mesh/mesh_renderer.h>
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
    ct_world_t0 world;
    ct_entity_t0 entity;
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
    uint64_t config;
    ce_alloc_t0 *allocator;
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
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), _G.config);
    if (!ce_cdb_a0->read_uint64(reader, CONFIG_DAEMON, 0)) {
        uint32_t w, h;
        w = ce_cdb_a0->read_uint64(reader, CONFIG_SCREEN_X, 0);
        h = ce_cdb_a0->read_uint64(reader, CONFIG_SCREEN_Y, 0);
        _G.size_width = w;
        _G.size_height = h;

        intptr_t wid = ce_cdb_a0->read_uint64(reader, CONFIG_WID, 0);

        char title[128] = {};
        snprintf(title, CE_ARRAY_LEN(title), "cetech");

        if (wid == 0) {
            bool fullscreen = ce_cdb_a0->read_uint64(reader,
                                                     CONFIG_SCREEN_FULLSCREEN,
                                                     0) > 0;

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

    const char *rtype = ce_cdb_a0->read_str(reader,
                                            CONFIG_RENDERER_TYPE, "");


    bool invalid = true;

    static struct {
        const char *k;
        bgfx_renderer_type_t v;
    } _str_to_render_type[] = {
            {.k = "noop", .v = BGFX_RENDERER_TYPE_NOOP},
            {.k = "opengl", .v = BGFX_RENDERER_TYPE_OPENGL},
            {.k = "metal", .v = BGFX_RENDERER_TYPE_METAL},

            {.k = "",
                    .v =
#if CE_PLATFORM_LINUX
                    BGFX_RENDERER_TYPE_OPENGL
#elif CE_PLATFORM_OSX
                    BGFX_RENDERER_TYPE_METAL
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
    bgfx_reset(_G.size_width, _G.size_height, _get_reset_flags(),
               BGFX_TEXTURE_FORMAT_COUNT);
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
    ce_api_entry_t0 it = ce_api_a0->first(COMPONENT_INTERFACE);
    while (it.api) {
        struct ct_component_i0 *i = (ct_component_i0 *) (it.api);

        struct ct_renderer_component_i0 *ci;
        ci = i->get_interface(CT_RENDERER_COMPONENT_I);

        if (ci && ci->feed_module) {
            ci->feed_module(world, module);
        }

        it = ce_api_a0->next(it);
    }
}

void _render_components(ct_world_t0 world,
                        struct ct_rg_builder_t0 *builder) {
    ce_api_entry_t0 it = ce_api_a0->first(COMPONENT_INTERFACE);
    while (it.api) {
        struct ct_component_i0 *i = (ct_component_i0 *) (it.api);

        struct ct_renderer_component_i0 *ci;
        ci = i->get_interface(CT_RENDERER_COMPONENT_I);

        if (ci && ci->render) {
            ci->render(world, builder);
        }

        it = ce_api_a0->next(it);
    }
}

static void render(float dt) {
    _G.viewid = 0;


    uint64_t events_n = 0;
    const uint64_t *events = ct_machine_a0->events(&events_n);
    for (int i = 0; i < events_n; ++i) {
        uint64_t event = events[i];
        const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), event);
        uint64_t event_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), event);

        if (event_type != EVENT_WINDOW_RESIZED) {
            continue;
        }

        _G.need_reset = 1;
        _G.size_width = ce_cdb_a0->read_uint64(reader, CT_MACHINE_WINDOW_WIDTH, 0);
        _G.size_height = ce_cdb_a0->read_uint64(reader, CT_MACHINE_WINDOW_HEIGHT, 0);
    }

    if (_G.need_reset) {
        _G.need_reset = 0;

        bgfx_reset(_G.size_width, _G.size_height, _get_reset_flags(), BGFX_TEXTURE_FORMAT_COUNT);
    }

    const uint32_t v_n = ce_array_size(_G.viewports);
    for (int i = 0; i < v_n; ++i) {
        viewport_t *v = &_G.viewports[i];

        if (v->free) {
            continue;
        }

        struct ct_rg_t0 *graph = ct_rg_a0->create_graph();
        struct ct_rg_module_t0 *module = ct_rg_a0->create_module();
        struct ct_rg_builder_t0 *builder = v->builder;

        builder->clear(builder);

        ct_default_rg_a0->feed_module(module, v->world, v->entity);
        _feed_module(v->world, module);

        graph->set_module(graph, module);
        graph->setup(graph, v->builder);

        v->builder->execute(v->builder);

        _render_components(v->world, v->builder);

        ct_rg_a0->destroy_module(module);
        ct_rg_a0->destroy_graph(graph);
    }

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

struct ct_viewport_t0 create_viewport(ct_world_t0 world,
                                      struct ct_entity_t0 main_camera) {
    uint32_t idx = _new_viewport();

    viewport_t *v = &_G.viewports[idx];

    ct_rg_builder_t0 *builder = ct_rg_a0->create_builder();

    v->world = world;
    v->entity = main_camera;
    v->builder = builder;

    return (ct_viewport_t0) {.idx=idx};
}


void destroy_viewport(ct_viewport_t0 viewport) {
    viewport_t *v = &_G.viewports[viewport.idx];
    v->free = true;
}

struct ct_rg_builder_t0 *viewport_builder(ct_viewport_t0 viewport) {
    return _G.viewports[viewport.idx].builder;
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
        .destroy_viewport = destroy_viewport,
        .viewport_builder = viewport_builder,
        .get_main_window = get_main_window,
};


struct ct_renderer_a0 *ct_renderer_a0 = &rendderer_api;

static uint64_t task_name() {
    return CT_RENDER_TASK;
}

static uint64_t *update_after(uint64_t *n) {
    static uint64_t a[] = {
            CT_GAME_TASK,
    };

    *n = CE_ARRAY_LEN(a);
    return a;
}


static struct ct_kernel_task_i0 render_task = {
        .name = task_name,
        .update = render,
        .update_after = update_after,
};

#include "gfx.inl"

static void _init_api(struct ce_api_a0 *api) {
    api->register_api(CT_RENDERER_API, &rendderer_api, sizeof(rendderer_api));
    api->register_api(CT_GFX_API, &gfx_api, sizeof(gfx_api));
    api->register_api(KERNEL_TASK_INTERFACE, &render_task, sizeof(render_task));
}

static void _init(struct ce_api_a0 *api) {
    _init_api(api);

    ce_api_a0 = api;

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .config = ce_config_a0->obj(),
    };

    ce_cdb_obj_o0 *writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(), _G.config);

    if (!ce_cdb_a0->prop_exist(writer, CONFIG_SCREEN_X)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_SCREEN_X, 1024);
    }

    if (!ce_cdb_a0->prop_exist(writer, CONFIG_SCREEN_Y)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_SCREEN_Y, 768);
    }

    if (!ce_cdb_a0->prop_exist(writer, CONFIG_SCREEN_FULLSCREEN)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_SCREEN_FULLSCREEN, 0);
    }

    if (!ce_cdb_a0->prop_exist(writer, CONFIG_DAEMON)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_DAEMON, 0);
    }

    if (!ce_cdb_a0->prop_exist(writer, CONFIG_WID)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_WID, 0);
    }

    if (!ce_cdb_a0->prop_exist(writer, CONFIG_RENDERER_TYPE)) {
        ce_cdb_a0->set_str(writer, CONFIG_RENDERER_TYPE, "");
    }

    ce_cdb_a0->write_commit(writer);

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), _G.config);

    _G.vsync = ce_cdb_a0->read_uint64(reader, CONFIG_SCREEN_VSYNC, 1) > 0;

    renderer_create();

}

static void _shutdown() {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), _G.config);
    if (!ce_cdb_a0->read_uint64(reader, CONFIG_DAEMON, 0)) {

        ce_array_free(_G.on_render, _G.allocator);

        bgfx_shutdown();
    }

    _G = (struct _G) {};
}

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
    _init(api);
}

void CE_MODULE_UNLOAD(renderer)(struct ce_api_a0 *api,
                                int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _shutdown();
}
