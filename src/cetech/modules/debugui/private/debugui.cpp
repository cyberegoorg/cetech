#include <cetech/engine/entity/entity.h>
#include <cetech/modules/renderer/renderer.h>
#include <cetech/modules/transform/transform.h>
#include <cetech/core/blob/blob.h>
#include <cetech/modules/camera/camera.h>
#include <cetech/modules/debugui/debugui.h>
#include <cetech/modules/debugui/private/bgfx_imgui/imgui.h>
#include <cetech/engine/input/input.h>
#include "celib/map.inl"

#include "cetech/core/yaml/yaml.h"
#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/config/config.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "celib/fpumath.h"
#include "cetech/core/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_mouse_a0);

using namespace celib;

static struct DebugUIGlobal {
    Array<void (*)()> on_gui;
} _G;

namespace debugui {
    void render() {
        float mp[3] = {};
        ct_mouse_a0.axis(0, ct_mouse_a0.axis_index("absolute"), mp);

        static bool show_app_about = true;
        static bool show_app_about2 = true;

        uint8_t btn = 0;

        if (ct_mouse_a0.button_state(0, ct_mouse_a0.button_index("left"))) {
            btn |= IMGUI_MBUT_LEFT;
        }

        if (ct_mouse_a0.button_state(0, ct_mouse_a0.button_index("right"))) {
            btn |= IMGUI_MBUT_RIGHT;
        }

        if (ct_mouse_a0.button_state(0, ct_mouse_a0.button_index("midle"))) {
            btn |= IMGUI_MBUT_MIDDLE;
        }

        uint32_t w, h;
        ct_renderer_a0.get_size(&w, &h);

        imguiBeginFrame(mp[0], h - mp[1], btn, 0, w, h);
        for(uint32_t i = 0; i < array::size(_G.on_gui); ++i) {
            _G.on_gui[i]();
        }
        imguiEndFrame();
    }

    void register_on_gui(void (*on_gui)()) {
        array::push_back(_G.on_gui, on_gui);
    }
}

namespace debugui_module {

    static ct_debugui_a0 debugui_api = {
            .render = debugui::render,
            .register_on_gui = debugui::register_on_gui
    };

    static void _init(ct_api_a0 *api) {
        api->register_api("ct_debugui_a0", &debugui_api);
        imguiCreate(16);

        _G = {};
        _G.on_gui.init(ct_memory_a0.main_allocator());
    }

    static void _shutdown() {
        imguiDestroy();

        _G.on_gui.destroy();
        _G = {};
    }
}

CETECH_MODULE_DEF(
        debugui,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_mouse_a0);
            CETECH_GET_API(api, ct_renderer_a0);
        },
        {
            debugui_module::_init(api);
        },
        {
            CEL_UNUSED(api);
            debugui_module::_shutdown();
        }
)