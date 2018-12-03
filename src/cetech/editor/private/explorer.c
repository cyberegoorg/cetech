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
#include <cetech/asset_editor/asset_browser.h>
#include <cetech/editor/explorer.h>
#include <cetech/editor/editor.h>
#include <cetech/resource/resource.h>

#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/asset_editor/asset_editor.h>
#include <cetech/resource/sourcedb.h>
#include <cetech/resource/builddb.h>
#include <cetech/editor/selcted_object.h>

#define WINDOW_NAME "Explorer"

#define _G explorer_globals
static struct _G {
    bool visible;
    struct ce_alloc *allocator;
} _G;

static uint64_t draw(uint64_t selected_obj) {
    struct ce_api_entry it = ce_api_a0->first(EXPLORER_INTERFACE);

    while (it.api) {
        struct ct_explorer_i0 *i = (it.api);

        if (i->draw_ui) {
            uint64_t new_selected_obj = i->draw_ui(selected_obj);
            return new_selected_obj;
        }

        it = ce_api_a0->next(it);
    }

    return 0;
}

static void draw_menu(uint64_t selected_obj) {
    uint64_t top_level_obj = ce_cdb_a0->find_root(selected_obj);

    if (ce_cdb_a0->prop_exist(top_level_obj, ASSET_NAME)) {
        struct ct_resource_id rid = {
                .type=ce_cdb_a0->obj_type(top_level_obj),
                .name = ce_cdb_a0->read_uint64(top_level_obj, ASSET_NAME, 0),
        };

        char fullname[256] = {};
        ct_builddb_a0->get_fullname(CE_ARR_ARG(fullname), rid.type, rid.name);
        ct_debugui_a0->Text("Asset: %s", fullname);

        bool save = ct_debugui_a0->Button("save", (float[2]) {0.0f});

        if (save) {
            ct_sourcedb_a0->save(rid);
        }

        ct_debugui_a0->SameLine(0, 10);
    }

    struct ce_api_entry it = ce_api_a0->first(EXPLORER_INTERFACE);

    while (it.api) {
        struct ct_explorer_i0 *i = (it.api);

        if (i->draw_menu) {
            i->draw_menu(selected_obj);
        }

        it = ce_api_a0->next(it);
    }
}

static void on_debugui(struct ct_dock_i0 *dock) {
    if (!ct_selected_object_a0->selected_object()) {
        return;
    }

    uint64_t selected_object = ct_selected_object_a0->selected_object();

    draw_menu(selected_object);

    ct_debugui_a0->Separator();

    uint64_t new_selected_object = draw(selected_object);
    if (new_selected_object) {
        ct_selected_object_a0->set_selected_object(new_selected_object);
    }
}


static const char *dock_title() {
    return ICON_FA_TREE " " WINDOW_NAME;
}

static const char *name(struct ct_dock_i0 *dock) {
    return "explorer";
}

static struct ct_dock_i0 ct_dock_i0 = {
        .id = 0,
        .visible = true,
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