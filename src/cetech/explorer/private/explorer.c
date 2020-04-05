#include <string.h>

#include <celib/id.h>
#include <celib/config.h>
#include <celib/memory/memory.h>
#include <celib/api.h>
#include <celib/module.h>
#include <celib/cdb.h>
#include <celib/yaml_cdb.h>

#include <celib/macros.h>


#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/editor/dock.h>
#include <cetech/explorer/explorer.h>
#include <cetech/asset/asset.h>

#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/editor/selcted_object.h>

#define WINDOW_NAME "Explorer"

#define _G explorer_globals
typedef const uint64_t i1;
static struct _G {
    bool visible;
    ce_alloc_t0 *allocator;
} _G;

static struct ct_explorer_i0 *_get_explorer_by_type(uint64_t type) {
    ce_api_entry_t0 it = ce_api_a0->first(CT_EXPLORER_I0);

    while (it.api) {
        struct ct_explorer_i0 *i = (it.api);

        if (i->cdb_type && (i->cdb_type() == type)) {
            return it.api;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
}

static uint64_t draw(uint64_t selected_obj,
                     uint64_t context) {
    uint64_t top_level = ce_cdb_a0->find_root(ce_cdb_a0->db(), selected_obj);
    uint64_t locked_object = 0;//ce_cdb_a0->read_ref(reader, CT_LOCKED_OBJ, 0);
    if (locked_object) {
        top_level = locked_object;
    }

    ct_explorer_i0 *i;
    i = _get_explorer_by_type(ce_cdb_a0->obj_type(ce_cdb_a0->db(), top_level));
    if (i && i->draw_ui) {
        return i->draw_ui(top_level, selected_obj, context);
    }
    return 0;
}

static void draw_menu(uint64_t content,
                      uint64_t context,
                      uint64_t selected_object) {
    uint64_t top_level_obj = ce_cdb_a0->find_root(ce_cdb_a0->db(),
                                                  selected_object);

    ct_explorer_i0 *i;
    i = _get_explorer_by_type(ce_cdb_a0->obj_type(ce_cdb_a0->db(),
                                                  top_level_obj));
    if (i && i->draw_menu) {
        i->draw_menu(selected_object, context);
    }
}

static void on_debugui(uint64_t content,
                       uint64_t context,
                       uint64_t selected_object) {
    if (!selected_object) {
        return;
    }

    ct_debugui_a0->Separator();

    uint64_t new_selected_object = draw(selected_object, context);
    if (new_selected_object) {
//        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), dock);
//        ce_cdb_a0->set_ref(w, PROP_DOCK_SELECTED_OBJ, new_selected_object);
//        ce_cdb_a0->write_commit(w);

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

static uint64_t dock_flags() {
    return 0;
}

static struct ct_dock_i0 dock_api = {
        .type = CT_EXPLORER_I0,
        .ui_flags = dock_flags,
        .name = name,
        .display_title = dock_title,
        .draw_ui = on_debugui,
        .draw_menu = draw_menu,
};

void CE_MODULE_LOAD(level_inspector)(struct ce_api_a0 *api,
                                     int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_debugui_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_asset_a0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .visible = true
    };

    api->add_impl(CT_DOCK_I0_STR, &dock_api, sizeof(dock_api));

    ct_dock_a0->create_dock(CT_EXPLORER_I0, true);
}

void CE_MODULE_UNLOAD(level_inspector)(struct ce_api_a0 *api,
                                       int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
    _G = (struct _G) {};
}
