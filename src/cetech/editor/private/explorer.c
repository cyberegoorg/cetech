#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <celib/hashlib.h>
#include <celib/config.h>
#include <celib/memory.h>
#include <celib/api_system.h>
#include <celib/module.h>
#include <celib/cdb.h>
#include <celib/ydb.h>
#include <celib/ydb.h>
#include <celib/fs.h>
#include <celib/ebus.h>
#include <celib/macros.h>


#include <cetech/ecs/ecs.h>
#include <cetech/debugui/debugui.h>
#include <cetech/editor/dock.h>
#include <cetech/editor/resource_browser.h>
#include <cetech/editor/explorer.h>
#include <cetech/editor/editor.h>
#include <cetech/resource/resource.h>

#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/editor/resource_editor.h>
#include <cetech/asset/sourcedb.h>
#include <cetech/resource/builddb.h>
#include <cetech/editor/selcted_object.h>

#define WINDOW_NAME "Explorer"

#define _G explorer_globals
static struct _G {
    bool visible;
    struct ce_alloc *allocator;
} _G;

static struct ct_explorer_i0 *_get_explorer_by_type(uint64_t type) {
    struct ce_api_entry it = ce_api_a0->first(EXPLORER_INTERFACE);

    while (it.api) {
        struct ct_explorer_i0 *i = (it.api);

        if (i->cdb_type && (i->cdb_type() == type)) {
            return it.api;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
}

static uint64_t draw(uint64_t selected_obj) {
    uint64_t top_level = ce_cdb_a0->find_root(selected_obj);

    struct ct_explorer_i0 *i;
    i = _get_explorer_by_type(ce_cdb_a0->obj_type(top_level));
    if (i && i->draw_ui) {
        return i->draw_ui(top_level, selected_obj);
    }
    return 0;
}

static void draw_menu(uint64_t selected_obj) {
    uint64_t top_level_obj = ce_cdb_a0->find_root(selected_obj);

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(top_level_obj);

    if (ce_cdb_a0->prop_exist(top_level_obj, ASSET_NAME_PROP)) {
        const char *name = ce_cdb_a0->read_str(reader, ASSET_NAME_PROP, "");
        ct_debugui_a0->Text("Asset: %s", name);
        ct_debugui_a0->SameLine(0, 10);
    }

    struct ct_explorer_i0 *i;
    i = _get_explorer_by_type(ce_cdb_a0->obj_type(selected_obj));
    if (i && i->draw_menu) {
        i->draw_menu(selected_obj);
    }
}

static void on_debugui(uint64_t dock) {
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(dock);
    const uint64_t context = ce_cdb_a0->read_uint64(reader, PROP_DOCK_CONTEXT, 0);

    if (!ct_selected_object_a0->selected_object(context)) {
        return;
    }

    uint64_t selected_object = ct_selected_object_a0->selected_object(context);

    draw_menu(selected_object);

    ct_debugui_a0->Separator();

    uint64_t new_selected_object = draw(selected_object);
    if (new_selected_object) {
        ct_selected_object_a0->set_selected_object(context,
                                                   new_selected_object);
    }
}


static const char *dock_title() {
    return ICON_FA_TREE " " WINDOW_NAME;
}

static const char *name(uint64_t dock) {
    return "explorer";
}


static uint64_t cdb_type() {
    return EXPLORER_INTERFACE;
};

static uint64_t dock_flags() {
    return 0;
}


static struct ct_dock_i0 ct_dock_i0 = {
        .cdb_type = cdb_type,
        .dock_flags = dock_flags,
        .name = name,
        .display_title = dock_title,
        .draw_ui = on_debugui,
};


static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .visible = true
    };

    api->register_api(DOCK_INTERFACE_NAME, &ct_dock_i0);

    ct_dock_a0->create_dock(EXPLORER_INTERFACE, true);

    ce_ebus_a0->create_ebus(EXPLORER_EBUS);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CE_MODULE_DEF(
        level_inspector,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_debugui_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ce_ebus_a0);
            CE_INIT_API(api, ct_resource_a0);
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