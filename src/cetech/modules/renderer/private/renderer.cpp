//==============================================================================
// includes
//==============================================================================
#include <cstdio>

#include <celib/allocator.h>
#include <celib/array.inl>
#include <celib/map.inl>
#include <celib/handler.inl>
#include <celib/fpumath.h>

#include <cetech/kernel/api_system.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/macros.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/hashlib.h>

#include <cetech/modules/application/application.h>
#include <cetech/modules/machine/window.h>
#include <cetech/modules/input/input.h>
#include <cetech/kernel/resource.h>

#include <cetech/modules/renderer/renderer.h>
#include <cetech/modules/debugui/private/bgfx_imgui/imgui.h>
#include <cetech/modules/machine/machine.h>

#include "bgfx/platform.h"

CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_window_a0);
CETECH_DECL_API(ct_api_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_machine_a0);

using namespace celib;

//==============================================================================
// GLobals
//==============================================================================

#define _G RendererGlobals
static struct G {
    celib::Array<ct_render_on_render> on_render;
    ct_window *main_window;

    uint64_t type;

    uint32_t size_width;
    uint32_t size_height;

    int capture;
    int vsync;
    int need_reset;
} _G = {};

static struct GConfig {
    ct_cvar screen_x;
    ct_cvar screen_y;
    ct_cvar screen_vsync;
    ct_cvar fullscreen;
    ct_cvar daemon;
    ct_cvar wid;
    ct_cvar render_config_name;
} GConfig;


static uint32_t _get_reset_flags() {
    return (_G.capture ? BGFX_RESET_CAPTURE : 0) |
           (_G.vsync ? BGFX_RESET_VSYNC : 0);
}

//==============================================================================
// Interface
//==============================================================================

void renderer_create() {

    if (!ct_config_a0.get_int(GConfig.daemon)) {
        int w, h;
        w = ct_config_a0.get_int(GConfig.screen_x);
        h = ct_config_a0.get_int(GConfig.screen_y);
        _G.size_width = w;
        _G.size_height = h;

        intptr_t wid = ct_config_a0.get_int(GConfig.wid);

        char title[128] = {};
        snprintf(title, CETECH_ARRAY_LEN(title), "cetech");


        if (wid == 0) {
            uint32_t flags = WINDOW_NOFLAG;
            flags |= ct_config_a0.get_int(GConfig.fullscreen)
                     ? WINDOW_FULLSCREEN : WINDOW_NOFLAG;
            flags |= WINDOW_RESIZABLE;

            _G.main_window = ct_window_a0.create(
                    ct_memory_a0.main_allocator(),
                    title,
                    WINDOWPOS_UNDEFINED,
                    WINDOWPOS_UNDEFINED,
                    w, h,
                    flags
            );
        } else {
            _G.main_window = ct_window_a0.create_from(
                    ct_memory_a0.main_allocator(), (void *) wid);
        }
    }

    bgfx::PlatformData pd = {};
    pd.nwh = _G.main_window->native_window_ptr(_G.main_window->inst);
    pd.ndt = _G.main_window->native_display_ptr(_G.main_window->inst);
    bgfx::setPlatformData(pd);

    // TODO: from config
    bgfx::init(bgfx::RendererType::OpenGL, 0, 0, NULL, NULL);

    _G.main_window->size(_G.main_window->inst, &_G.size_width, &_G.size_height);
    bgfx::reset(_G.size_width, _G.size_height, _get_reset_flags());
    _G.main_window->update(_G.main_window);

    _G.need_reset = 1;
}


void renderer_set_debug(int debug) {
    if (debug) {
        bgfx::setDebug(BGFX_DEBUG_STATS);
    } else {
        bgfx::setDebug(BGFX_DEBUG_NONE);
    }
}


void renderer_get_size(uint32_t *width,
                       uint32_t *height) {
    *width = _G.size_width;
    *height = _G.size_height;
}


/////
#define _DEF_ON_CLB_FCE(type, name)                                            \
    static void register_ ## name ## _(type name) {                            \
        array::push_back(_G.name, name);                                \
    }                                                                          \
    static void unregister_## name ## _(type name) {                           \
        const auto size = array::size(_G.name);                         \
                                                                               \
        for(uint32_t i = 0; i < size; ++i) {                                   \
            if(_G.name[i] != name) {                                           \
                continue;                                                      \
            }                                                                  \
                                                                               \
            uint32_t last_idx = size - 1;                                      \
            _G.name[i] = _G.name[last_idx];                                    \
                                                                               \
            array::pop_back(_G.name);                                   \
            break;                                                             \
        }                                                                      \
    }

_DEF_ON_CLB_FCE(ct_render_on_render, on_render)

#undef _DEF_ON_CLB_FCE

static void on_update() {
    ct_event_header *event = ct_machine_a0.event_begin();

    ct_window_resized_event *ev;
    while (event != ct_machine_a0.event_end()) {
        switch (event->type) {
            case EVENT_WINDOW_RESIZED:
                ev = (ct_window_resized_event *) event;
                _G.need_reset = 1;
                _G.size_width = ev->width;
                _G.size_height = ev->height;
                break;

            default:
                break;
        }

        event = ct_machine_a0.event_next(event);
    }
}

static void on_render(void (*on_render)()) {
    on_update();

    if (_G.need_reset) {
        _G.need_reset = 0;

        bgfx::reset(_G.size_width, _G.size_height, _get_reset_flags());
    }

    if(on_render) {
        on_render();
    }

    for (uint32_t i = 0; i < array::size(_G.on_render); ++i) {
        _G.on_render[i]();
    }

    bgfx::frame();
    _G.main_window->update(_G.main_window);
}


namespace renderer_module {
    static ct_renderer_a0 rendderer_api = {
            .render = on_render,
            .create = renderer_create,
            .set_debug = renderer_set_debug,
            .get_size = renderer_get_size,
            .register_on_render =register_on_render_,
            .unregister_on_render =unregister_on_render_,
    };

    void _init_api(struct ct_api_a0 *api) {
        api->register_api("ct_renderer_a0", &rendderer_api);
    }

    void _init(struct ct_api_a0 *api) {
        _init_api(api);

        ct_api_a0 = *api;

        _G = (struct G) {};


        GConfig = {
                .screen_x = ct_config_a0.new_int(
                        "screen.x", "Screen width", 1024),

                .screen_y = ct_config_a0.new_int(
                        "screen.y", "Screen height", 768),

                .screen_vsync = ct_config_a0.new_int(
                        "screen.vsync", "Screen vsync", 1),

                .fullscreen = ct_config_a0.new_int(
                        "screen.fullscreen", "Fullscreen", 0),

                .daemon = ct_config_a0.new_int("daemon", "Daemon mode", 0),
                .wid = ct_config_a0.new_int("wid", "Wid", 0),

                .render_config_name = ct_config_a0.new_str("renderer.config",
                                                           "Render condfig",
                                                           "default")
        };

        if (!ct_config_a0.get_int(GConfig.daemon)) {
            _G.vsync = ct_config_a0.get_int(GConfig.screen_vsync) > 0;

        }

        CETECH_GET_API(api, ct_window_a0);

        renderer_create();

        _G.on_render.init(ct_memory_a0.main_allocator());
    }

    void _shutdown() {
        ct_cvar daemon = ct_config_a0.find("daemon");
        if (!ct_config_a0.get_int(daemon)) {

            _G.on_render.destroy();

            bgfx::shutdown();
        }

        _G = (struct G) {};
    }

}

CETECH_MODULE_DEF(
        renderer,
        {
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_machine_a0);
        },
        {
            CEL_UNUSED(reload);
            renderer_module::_init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);

            renderer_module::_shutdown();
        }
)