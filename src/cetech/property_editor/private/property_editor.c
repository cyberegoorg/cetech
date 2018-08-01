#include <corelib/ebus.h>
#include <corelib/macros.h>
#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/module.h"

#include <cetech/debugui/debugui.h>

#include <cetech/property_editor/property_editor.h>
#include <cetech/editor/editor.h>
#include <cetech/debugui/private/iconfontheaders/icons_font_awesome.h>
#include <cetech/dock/dock.h>
#include <string.h>
#include <cetech/selected_object/selected_object.h>

#define WINDOW_NAME "Property editor"

#define _G property_inspector_global
static struct _G {
    bool visible;
} _G;

static void draw(uint64_t obj) {
    struct ct_api_entry it = ct_api_a0->first(PROPERTY_EDITOR_INTERFACE);

    while (it.api) {
        struct ct_property_editor_i0 *i = (it.api);
        i->draw_ui(obj);

        it = ct_api_a0->next(it);
    }
}

static void on_debugui(struct ct_dock_i0 *dock) {
    uint64_t obj = ct_selected_object_a0->selected_object();

    ct_debugui_a0->Columns(2, NULL, true);
    ct_debugui_a0->Separator();

    ct_debugui_a0->Text("Property");
    ct_debugui_a0->NextColumn();

    ct_debugui_a0->Text("Value");
    ct_debugui_a0->NextColumn();

    ct_debugui_a0->Separator();

    draw(obj);

    ct_debugui_a0->Columns(1, NULL, true);
}

static const char *dock_title() {
    return ICON_FA_TABLE " " WINDOW_NAME;
}

static const char *name(struct ct_dock_i0 *dock) {
    return "property_editor";
}

static struct ct_dock_i0 ct_dock_i0 = {
        .id = 0,
        .visible = true,
        .name = name,
        .display_title = dock_title,
        .draw_ui = on_debugui,
};


struct ct_property_editor_a0 property_editor_api = {
        .draw =draw,
};

struct ct_property_editor_a0 *ct_property_editor_a0 = &property_editor_api;

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .visible = true
    };

    api->register_api(DOCK_INTERFACE_NAME, &ct_dock_i0);
    api->register_api("ct_property_editor_a0", ct_property_editor_a0);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        property_inspector,
        {
            CT_INIT_API(api, ct_hashlib_a0);
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