#include <string.h>

#include <celib/ebus.h>
#include <celib/macros.h>
#include "celib/hashlib.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/module.h"

#include <cetech/resource/resource.h>
#include <cetech/debugui/debugui.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/editor/property.h>
#include <cetech/editor/editor.h>
#include <cetech/editor/dock.h>
#include <cetech/asset_editor/asset_browser.h>
#include <cetech/editor/explorer.h>
#include <cetech/resource/sourcedb.h>
#include <celib/cdb.h>
#include <cetech/editor/selcted_object.h>

#define WINDOW_NAME "Property editor"

#define _G property_inspector_global
static struct _G {
    bool visible;
} _G;

static void draw(uint64_t obj) {
    if(!obj) {
        return;
    }

    struct ce_api_entry it = ce_api_a0->first(PROPERTY_EDITOR_INTERFACE);

    while (it.api) {
        struct ct_property_editor_i0 *i = (it.api);

        if (i && i->cdb_type
            && (i->cdb_type() == ce_cdb_a0->obj_type(obj))) {

            if (i->draw_ui) {
                i->draw_ui(obj);
            }
            return;
        }

        it = ce_api_a0->next(it);
    }
}

static void draw_menu(uint64_t obj) {
    if(!obj) {
        return;
    }

    struct ce_api_entry it = ce_api_a0->first(PROPERTY_EDITOR_INTERFACE);

    while (it.api) {
        struct ct_property_editor_i0 *i = (it.api);

        if (i && i->cdb_type
            && (i->cdb_type() == ce_cdb_a0->obj_type(obj))) {

            if (i->draw_menu) {
                i->draw_menu(obj);
            }
            return;
        }

        it = ce_api_a0->next(it);
    }
}

static void on_debugui(uint64_t dock) {
    const uint64_t context = ce_cdb_a0->read_uint64(dock, PROP_DOCK_CONTEXT, 0);
    uint64_t obj = ct_selected_object_a0->selected_object(context);

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

static void on_menu(uint64_t dock) {
    const uint64_t context = ce_cdb_a0->read_uint64(dock, PROP_DOCK_CONTEXT, 0);
    uint64_t obj = ct_selected_object_a0->selected_object(context);
    draw_menu(obj);
}


static const char *dock_title() {
    return ICON_FA_TABLE " " WINDOW_NAME;
}

static const char *name(uint64_t dock) {
    return "property_editor";
}

static uint64_t cdb_type() {
    return PROPERTY_EDITOR_INTERFACE;
};


static struct ct_dock_i0 ct_dock_i0 = {
        .cdb_type = cdb_type,
        .name = name,
        .display_title = dock_title,
        .draw_ui = on_debugui,
        .draw_menu = on_menu,
};


struct ct_property_editor_a0 property_editor_api = {
        .draw =draw,
};

struct ct_property_editor_a0 *ct_property_editor_a0 = &property_editor_api;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .visible = true
    };

    api->register_api(DOCK_INTERFACE_NAME, &ct_dock_i0);
    api->register_api("ct_property_editor_a0", ct_property_editor_a0);

    ct_dock_a0->create_dock(PROPERTY_EDITOR_INTERFACE, true);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CE_MODULE_DEF(
        property_inspector,
        {
            CE_INIT_API(api, ce_id_a0);
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