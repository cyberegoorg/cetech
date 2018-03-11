#include "cetech/core/containers/map.inl"

#include <cetech/engine/debugui/debugui.h>
#include <cetech/playground/property_editor.h>
#include <cetech/playground/playground.h>
#include <cetech/core/ebus/ebus.h>

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_playground_a0);
CETECH_DECL_API(ct_ebus_a0);

using namespace celib;

#define WINDOW_NAME "Property editor"
#define PLAYGROUND_MODULE_NAME CT_ID64_0("property_editor")

#define _G property_inspector_global
static struct _G {
    ct_pi_on_debugui on_debugui;
    bool visible;
} _G;


static void set_active(ct_pi_on_debugui on_debugui) {
    _G.on_debugui = on_debugui;
}

static ct_property_editor_a0 property_inspector_api = {
        .set_active = set_active
};


static void on_debugui(uint64_t bus_name,
                       void *event) {
    if (ct_debugui_a0.BeginDock(WINDOW_NAME,
                                &_G.visible,
                                DebugUIWindowFlags_(0))) {
        if (_G.on_debugui) {
            _G.on_debugui();
        }
    }
    ct_debugui_a0.EndDock();
}

static void on_menu_window(uint64_t bus_name,
                           void *event) {
    ct_debugui_a0.MenuItem2(WINDOW_NAME, NULL, &_G.visible, true);
}

static void _init(ct_api_a0 *api) {
    _G = {
            .visible = true
    };

    api->register_api("ct_property_editor_a0", &property_inspector_api);

    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UI_EVENT, on_debugui);
    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UI_MAINMENU_EVENT, on_menu_window);


}

static void _shutdown() {

    _G = {};
}

CETECH_MODULE_DEF(
        property_inspector,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_playground_a0);
            CETECH_GET_API(api, ct_ebus_a0);
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