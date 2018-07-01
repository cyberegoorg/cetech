#include <corelib/ebus.h>
#include <corelib/macros.h>
#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/module.h"

#include <cetech/debugui/debugui.h>

#include <cetech/playground/property_editor.h>
#include <cetech/playground/playground.h>
#include <cetech/debugui/private/iconfontheaders/icons_font_awesome.h>


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

struct ct_property_editor_a0 *ct_property_editor_a0 = &property_inspector_api;

static void on_debugui(struct ct_dock_i *dock) {
    struct ct_api_entry it = ct_api_a0->first("ct_property_editor_i0");
    while (it.api) {
        struct ct_property_editor_i0 *i = (it.api);
        i->draw();

        it = ct_api_a0->next(it);
    }
}

static const char *dock_title() {
    return ICON_FA_TABLE " " WINDOW_NAME;
}

static struct ct_dock_i ct_dock_i = {
        .id = 0,
        .visible = true,
        .title = dock_title,
        .draw_ui = on_debugui,
};

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .visible = true
    };

    api->register_api("ct_property_editor_a0", &property_inspector_api);
    api->register_api("ct_dock_i", &ct_dock_i);

}

static void _shutdown() {
    _G = (struct _G) {};
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