//==============================================================================
// includes
//==============================================================================
#include <cstdio>


extern "C" {
#include <celib/allocator.h>
#include <celib/array.inl>

#include <celib/api_system.h>
#include <celib/config.h>
#include <celib/macros.h>
#include <celib/module.h>
#include <celib/memory.h>
#include <celib/hashlib.h>
#include <celib/os.h>
#include <celib/private/api_private.h>
#include <celib/cdb.h>
#include <celib/ebus.h>
#include <celib/log.h>

#include <cetech/kernel/kernel.h>
#include <cetech/resource/resource.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/machine/machine.h>
#include <cetech/ecs/ecs.h>
#include <cetech/debugui/debugui.h>
#include <cetech/game/game_system.h>
}

#include "bgfx/c99/bgfx.h"
#include "bgfx/c99/platform.h"

//==============================================================================
// GLobals
//==============================================================================

#define _G RendererGlobals
static struct _G {
    ct_renderender_on_render *on_render;
    ce_window *main_window;

    uint64_t type;

    uint32_t size_width;
    uint32_t size_height;

    uint64_t viewid;

    bool capture;
    bool vsync;
    bool need_reset;
    uint64_t config;
    ce_alloc *allocator;
} _G = {};


static uint32_t _get_reset_flags() {
    return (_G.capture ? BGFX_RESET_CAPTURE : 0) |
           (_G.vsync ? BGFX_RESET_VSYNC : 0);
}




//==============================================================================
// Interface
//==============================================================================

static void renderer_create() {
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(_G.config);
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

            _G.main_window = ce_os_a0->window->create(
                    _G.allocator,
                    title,
                    WINDOWPOS_UNDEFINED,
                    WINDOWPOS_UNDEFINED,
                    w, h,
                    flags);

        } else {
            _G.main_window = ce_os_a0->window->create_from(_G.allocator,
                                                           (void *) wid);
        }
    }

    bgfx_platform_data_t pd = {NULL};
    pd.nwh = _G.main_window->native_window_ptr(_G.main_window->inst);
    pd.ndt = _G.main_window->native_display_ptr(_G.main_window->inst);
    bgfx_set_platform_data(&pd);

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
                    BGFX_RENDERER_TYPE_METAL// metal in future
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


static void on_resize(uint64_t type,
                      void *event) {
    _G.need_reset = 1;

    struct ebus_cdb_event *ev = (struct ebus_cdb_event *) event;

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ev->obj);

    _G.size_width = ce_cdb_a0->read_uint64(reader, CT_MACHINE_WINDOW_WIDTH, 0);
    _G.size_height = ce_cdb_a0->read_uint64(reader, CT_MACHINE_WINDOW_HEIGHT,
                                            0);
}

static void render(float dt) {
    _G.viewid = 0;
    if (_G.need_reset) {
        _G.need_reset = 0;

        bgfx_reset(_G.size_width, _G.size_height, _get_reset_flags(),
                   BGFX_TEXTURE_FORMAT_COUNT);
    }

    ct_debugui_a0->begin();
    ct_debugui_a0->end();
    bgfx_frame(false);
}

uint64_t new_viewid() {
    return _G.viewid++;
}

static struct ct_renderer_a0 rendderer_api = {
        .create = renderer_create,
        .set_debug = renderer_set_debug,
        .get_size = renderer_get_size,
        .new_viewid = new_viewid,
};


struct ct_renderer_a0 *ct_renderer_a0 = &rendderer_api;

#include "gfx.inl"

static uint64_t task_name() {
    return CT_RENDER_TASK;
}

static uint64_t * update_after(uint64_t* n) {
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

static void _init_api(struct ce_api_a0 *api) {
    api->register_api("ct_renderer_a0", &rendderer_api);
    api->register_api("ct_gfx_a0", &rendderapi_api);
    api->register_api("ct_kernel_task_i0", &render_task);
}

static void _init(struct ce_api_a0 *api) {
    _init_api(api);

    ce_api_a0 = api;

    _G = {
            .allocator = ce_memory_a0->system,
            .config = ce_config_a0->obj(),
    };

    ce_ebus_a0->connect(WINDOW_EBUS, EVENT_WINDOW_RESIZED, on_resize, 0);


    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(_G.config);

    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_SCREEN_X)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_SCREEN_X, 1024);
    }

    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_SCREEN_Y)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_SCREEN_Y, 768);
    }

    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_SCREEN_FULLSCREEN)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_SCREEN_FULLSCREEN, 0);
    }

    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_DAEMON)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_DAEMON, 0);
    }

    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_WID)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_WID, 0);
    }

    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_RENDERER_TYPE)) {
        ce_cdb_a0->set_str(writer, CONFIG_RENDERER_TYPE, "");
    }

    ce_cdb_a0->write_commit(writer);

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(_G.config);

    _G.vsync = ce_cdb_a0->read_uint64(reader, CONFIG_SCREEN_VSYNC, 1) > 0;

    CE_INIT_API(api, ce_os_a0);

    renderer_create();

}

static void _shutdown() {
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(_G.config);
    if (!ce_cdb_a0->read_uint64(reader, CONFIG_DAEMON, 0)) {

        ce_array_free(_G.on_render, _G.allocator);

        bgfx_shutdown();
    }

    ce_ebus_a0->disconnect(WINDOW_EBUS, EVENT_WINDOW_RESIZED, on_resize);

    _G = {};
}

CE_MODULE_DEF(
        renderer,
        {
            CE_INIT_API(api, ce_config_a0);
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_resource_a0);
            CE_INIT_API(api, ct_machine_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ct_ecs_a0);
            CE_INIT_API(api, ce_ebus_a0);
        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);

            _shutdown();
        }
)