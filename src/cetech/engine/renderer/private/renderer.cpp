//==============================================================================
// includes
//==============================================================================
#include <cstdio>

#include <cetech/core/memory/allocator.h>
#include <cetech/core/containers/array.h>
#include <cetech/core/containers/map.inl>

#include <cetech/core/api/api_system.h>
#include <cetech/core/config/config.h>
#include <cetech/macros.h>
#include <cetech/core/module/module.h>
#include <cetech/core/memory/memory.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/engine/kernel/kernel.h>

#include <cetech/engine/application/application.h>
#include <cetech/core/os/window.h>
#include <cetech/engine/resource/resource.h>

#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/debugui/private/bgfx_imgui/imgui.h>
#include <cetech/engine/machine/machine.h>
#include <cetech/core/api/private/api_private.h>

#include "bgfx/platform.h"

CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_window_a0);
CETECH_DECL_API(ct_api_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_machine_a0);
CETECH_DECL_API(ct_cdb_a0);

using namespace celib;

//==============================================================================
// GLobals
//==============================================================================

#define _G RendererGlobals
static struct _G {
    ct_render_on_render *on_render;
    ct_window *main_window;

    uint64_t type;

    uint32_t size_width;
    uint32_t size_height;

    bool capture;
    bool vsync;
    bool need_reset;
    ct_cdb_obj_t *config;
    ct_alloc *allocator;
} _G = {};


static uint32_t _get_reset_flags() {
    return (_G.capture ? BGFX_RESET_CAPTURE : 0) |
           (_G.vsync ? BGFX_RESET_VSYNC : 0);
}

#define CONFIG_SCREEN_X CT_ID64_0(CONFIG_SCREEN_X_ID)
#define CONFIG_SCREEN_Y CT_ID64_0(CONFIG_SCREEN_Y_ID)
#define CONFIG_SCREEN_VSYNC CT_ID64_0(CONFIG_SCREEN_VSYNC_ID)
#define CONFIG_SCREEN_FULLSCREEN CT_ID64_0(CONFIG_SCREEN_FULLSCREEN_ID)
#define CONFIG_DAEMON CT_ID64_0(CONFIG_DAEMON_ID)
#define CONFIG_WID CT_ID64_0(CONFIG_WID_ID)
#define CONFIG_RENDER_CONFIG CT_ID64_0(CONFIG_RENDER_CONFIG_ID)


//==============================================================================
// Interface
//==============================================================================

static void renderer_create() {

    if (!ct_cdb_a0.read_uint32(_G.config, CONFIG_DAEMON, 0)) {
        int w, h;
        w = ct_cdb_a0.read_uint32(_G.config, CONFIG_SCREEN_X, 0);
        h = ct_cdb_a0.read_uint32(_G.config, CONFIG_SCREEN_Y, 0);
        _G.size_width = w;
        _G.size_height = h;

        intptr_t wid = ct_cdb_a0.read_uint32(_G.config, CONFIG_WID, 0);

        char title[128] = {};
        snprintf(title, CETECH_ARRAY_LEN(title), "cetech");


        if (wid == 0) {
            uint32_t flags = WINDOW_NOFLAG;
            flags |= ct_cdb_a0.read_uint32(_G.config,
                                           CONFIG_SCREEN_FULLSCREEN, 0)
                     ? WINDOW_FULLSCREEN : WINDOW_NOFLAG;
            flags |= WINDOW_RESIZABLE;

            _G.main_window = ct_window_a0.create(
                    _G.allocator,
                    title,
                    WINDOWPOS_UNDEFINED,
                    WINDOWPOS_UNDEFINED,
                    w, h,
                    flags
            );
        } else {
            _G.main_window = ct_window_a0.create_from(_G.allocator,
                                                      (void *) wid);
        }
    }

    bgfx::PlatformData pd = {NULL};
    pd.nwh = _G.main_window->native_window_ptr(_G.main_window->inst);
    pd.ndt = _G.main_window->native_display_ptr(_G.main_window->inst);
    bgfx::setPlatformData(pd);

    // TODO: from config
    bgfx::init(bgfx::RendererType::OpenGL, 0, 0, NULL, NULL);

    _G.main_window->size(_G.main_window->inst, &_G.size_width, &_G.size_height);
    bgfx::reset(_G.size_width, _G.size_height, _get_reset_flags());
    //_G.main_window->update(_G.main_window);

    _G.need_reset = 1;
}


static void renderer_set_debug(int debug) {
    if (debug) {
        bgfx::setDebug(BGFX_DEBUG_STATS);
    } else {
        bgfx::setDebug(BGFX_DEBUG_NONE);
    }
}


static void renderer_get_size(uint32_t *width,
                              uint32_t *height) {
    *width = _G.size_width;
    *height = _G.size_height;
}


/////
#define _DEF_ON_CLB_FCE(type, name)                                            \
    static void register_ ## name ## _(type name) {                            \
        ct_array_push(_G.name, name, _G.allocator);                           \
    }                                                                          \
    static void unregister_## name ## _(type name) {                           \
        const auto size = ct_array_size(_G.name);                             \
                                                                               \
        for(uint32_t i = 0; i < size; ++i) {                                   \
            if(_G.name[i] != name) {                                           \
                continue;                                                      \
            }                                                                  \
                                                                               \
            uint32_t last_idx = size - 1;                                      \
            _G.name[i] = _G.name[last_idx];                                    \
                                                                               \
            ct_array_pop_back(_G.name);                                       \
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

    if (on_render) {
        on_render();
    }

    for (uint32_t i = 0; i < ct_array_size(_G.on_render); ++i) {
        _G.on_render[i]();
    }

    bgfx::frame();
}


static ct_renderer_a0 rendderer_api = {
        .render = on_render,
        .create = renderer_create,
        .set_debug = renderer_set_debug,
        .get_size = renderer_get_size,
        .register_on_render =register_on_render_,
        .unregister_on_render =unregister_on_render_,
};

static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_renderer_a0", &rendderer_api);
}

static void _init(struct ct_api_a0 *api) {
    _init_api(api);

    ct_api_a0 = *api;

    _G = {
            .allocator = ct_memory_a0.main_allocator(),
            .config = ct_config_a0.config_object(),
    };

    ct_cdb_writer_t *writer = ct_cdb_a0.write_begin(_G.config);

    if (!ct_cdb_a0.prop_exist(_G.config, CONFIG_SCREEN_X)) {
        ct_cdb_a0.set_uint32(writer, CONFIG_SCREEN_X, 1024);
    }

    if (!ct_cdb_a0.prop_exist(_G.config, CONFIG_SCREEN_Y)) {
        ct_cdb_a0.set_uint32(writer, CONFIG_SCREEN_Y, 768);
    }

    if (!ct_cdb_a0.prop_exist(_G.config, CONFIG_SCREEN_FULLSCREEN)) {
        ct_cdb_a0.set_uint32(writer, CONFIG_SCREEN_FULLSCREEN, 0);
    }

    if (!ct_cdb_a0.prop_exist(_G.config, CONFIG_DAEMON)) {
        ct_cdb_a0.set_uint32(writer, CONFIG_DAEMON, 0);
    }

    if (!ct_cdb_a0.prop_exist(_G.config, CONFIG_WID)) {
        ct_cdb_a0.set_uint32(writer, CONFIG_WID, 0);
    }

    if (!ct_cdb_a0.prop_exist(_G.config, CONFIG_RENDER_CONFIG)) {
        ct_cdb_a0.set_string(writer, CONFIG_RENDER_CONFIG, "default");
    }

    ct_cdb_a0.write_commit(writer);


    _G.vsync = ct_cdb_a0.read_uint32(_G.config, CONFIG_SCREEN_VSYNC, 1) > 0;

    CETECH_GET_API(api, ct_window_a0);

    renderer_create();

}

static void _shutdown() {
    if (!ct_cdb_a0.read_uint32(_G.config, CONFIG_DAEMON, 0)) {

        ct_array_free(_G.on_render, _G.allocator);

        bgfx::shutdown();
    }

    _G = {};
}

CETECH_MODULE_DEF(
        renderer,
        {
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_machine_a0);
            CETECH_GET_API(api, ct_cdb_a0);
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