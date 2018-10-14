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
#include <cetech/gfx/debugui.h>
#include <cetech/editor/dock.h>
#include <cetech/editor/asset_browser.h>
#include <cetech/editor/explorer.h>
#include <cetech/editor/editor.h>
#include <cetech/resource/resource.h>

#include <cetech/gfx/private/iconfontheaders/icons_font_awesome.h>
#include <cetech/editor/asset_editor.h>

#define WINDOW_NAME "Explorer"

#define _G explorer_globals
static struct _G {
    bool visible;

    uint64_t selected_object;

    struct ce_alloc *allocator;
    uint64_t top_level_obj;
} _G;

static uint64_t draw(uint64_t top_level_obj, uint64_t selected_obj) {
    struct ce_api_entry it = ce_api_a0->first(EXPLORER_INTERFACE);

    while (it.api) {
        struct ct_explorer_i0 *i = (it.api);

        if (i->draw_ui) {
            uint64_t new_selected_obj = i->draw_ui(top_level_obj, selected_obj);
            return new_selected_obj;
        }

        it = ce_api_a0->next(it);
    }

    return 0;
}

static void draw_menu(uint64_t top_level_obj, uint64_t selected_obj) {
    struct ce_api_entry it = ce_api_a0->first(EXPLORER_INTERFACE);

    while (it.api) {
        struct ct_explorer_i0 *i = (it.api);

        if (i->draw_menu) {
            i->draw_menu(top_level_obj, selected_obj);
        }

        it = ce_api_a0->next(it);
    }
}

static void on_debugui(struct ct_dock_i0 *dock) {
    if (!_G.top_level_obj) {
        return;
    }

    draw_menu(_G.top_level_obj, _G.selected_object);

    ct_debugui_a0->Separator();

    uint64_t selected_object = draw(_G.top_level_obj, _G.selected_object);
    if(selected_object) {
        _G.selected_object = selected_object;

        uint64_t event;
        event = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                         EXPLORER_OBJ_SELECTED);

        struct ct_cdb_obj_t *w = ce_cdb_a0->write_begin(event);
        ce_cdb_a0->set_ref(w, EXPLORER_OBJ_SELECTED, selected_object);
        ce_cdb_a0->write_commit(w);

        ce_ebus_a0->broadcast(EXPLORER_EBUS, event);
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


static void _on_asset_selected(uint64_t event) {
    uint64_t type = ce_cdb_a0->read_uint64(event, ASSET_TYPE, 0);
    uint64_t name = ce_cdb_a0->read_uint64(event, ASSET_NAME, 0);

    struct ct_resource_id rid = {
            .name = name,
            .type = type,
    };

    if (type != ENTITY_RESOURCE_ID) {
        return;
    }

    _G.top_level_obj = ct_resource_a0->get(rid);
}

static void _on_editor_asset_selected(uint64_t event) {
    uint64_t type = ce_cdb_a0->read_uint64(event, ASSET_TYPE, 0);
    uint64_t name = ce_cdb_a0->read_uint64(event, ASSET_NAME, 0);

    struct ct_resource_id rid = {
            .name = name,
            .type = type,
    };

    _G.top_level_obj = ct_resource_a0->get(rid);
}

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .visible = true
    };

    api->register_api(DOCK_INTERFACE_NAME, &ct_dock_i0);

    ce_ebus_a0->create_ebus(EXPLORER_EBUS);

    ce_ebus_a0->connect(ASSET_BROWSER_EBUS, ASSET_BROWSER_ASSET_SELECTED,
                        _on_asset_selected, 0);
    ce_ebus_a0->connect(ASSET_EDITOR_EBUS, ASSET_EDITOR_ASSET_SELECTED,
                        _on_editor_asset_selected, 0);
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