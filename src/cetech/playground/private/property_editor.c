#include <cetech/kernel/ebus/ebus.h>
#include <cetech/kernel/macros.h>
#include "cetech/kernel/hashlib/hashlib.h"
#include "cetech/kernel/memory/memory.h"
#include "cetech/kernel/api/api_system.h"
#include "cetech/kernel/module/module.h"

#include <cetech/engine/debugui/debugui.h>

#include <cetech/playground/property_editor.h>
#include <cetech/playground/playground.h>


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_playground_a0);
CETECH_DECL_API(ct_ebus_a0);

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

static struct ct_property_editor_a0 property_inspector_api = {
        .set_active = set_active
};


static void on_debugui(void *event) {
    if (ct_debugui_a0.BeginDock(WINDOW_NAME, &_G.visible, 0)) {
        if (_G.on_debugui) {
            _G.on_debugui();
        }
    }
    ct_debugui_a0.EndDock();
}

static void on_menu_window(void *event) {
    ct_debugui_a0.MenuItem2(WINDOW_NAME, NULL, &_G.visible, true);
}

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G){
            .visible = true
    };

    api->register_api("ct_property_editor_a0", &property_inspector_api);

    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UI_EVENT, on_debugui, 0);
    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UI_MAINMENU_EVENT,
                       on_menu_window, 0);
}

static void _shutdown() {
    ct_ebus_a0.disconnect(PLAYGROUND_EBUS, PLAYGROUND_UI_EVENT, on_debugui);
    ct_ebus_a0.disconnect(PLAYGROUND_EBUS, PLAYGROUND_UI_MAINMENU_EVENT,
                          on_menu_window);


    _G = (struct _G){};
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