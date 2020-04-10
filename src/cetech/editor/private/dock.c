#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/containers/array.h>
#include <celib/id.h>
#include <celib/memory/memory.h>
#include <celib/api.h>
#include <celib/module.h>
#include <celib/cdb.h>

#include <cetech/property_editor/property_editor.h>
#include <stdio.h>
#include <celib/fs.h>
#include <cetech/asset_browser/asset_browser.h>
#include <cetech/ui/icons_font_awesome.h>

#include <cetech/renderer/gfx.h>

#include <cetech/editor/selcted_object.h>
#include <cetech/editor/editor_ui.h>
#include <celib/containers/hash.h>
#include <cetech/ui/ui.h>

#include "../dock.h"

#define _G explorer_globals

typedef struct dock_t {
    uint64_t type;
    uint64_t content;
    uint64_t context;
    uint64_t id;
    uint64_t locked_obj;
} dock_t;

static struct _G {
    uint64_t *contexts;

    ce_hash_t dock_count;
    dock_t *docks;

    ce_alloc_t0 *allocator;
} _G;

#define MAX_CONTEXT 8

struct ct_dock_i0 *_find_dock_i(uint64_t type) {
    ce_api_entry_t0 it = ce_api_a0->first(CT_DOCK_I0);

    while (it.api) {
        struct ct_dock_i0 *i = (it.api);

        if (i && i->type == type) {
            return i;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
}

void create_dock(uint64_t type,
                 bool visible) {

    ct_dock_i0 *i = _find_dock_i(type);

    if (!i) {
        return;
    }

    uint32_t dock_id = ce_hash_lookup(&_G.dock_count, type, 0);

    dock_t dock = {
            .type = type,
            .id = dock_id,
    };

    ce_hash_add(&_G.dock_count, type, dock_id + 1, _G.allocator);

    if (i->open) {
        dock.content = i->open();
    }

    ce_array_push(_G.docks, dock, _G.allocator);
}

void close_dock(dock_t *dock) {
    ct_dock_i0 *i = _find_dock_i(dock->type);

    if (i && i->close) {
        i->close(dock->content, 0);
    }

    uint64_t idx = dock - _G.docks;

    uint32_t dock_n = ce_array_size(_G.docks);

    uint32_t last_idx = dock_n - 1;
    _G.docks[idx] = _G.docks[last_idx];
    ce_array_pop_back(_G.docks);
}

void lock_selected_obj(dock_t *dock,
                       uint64_t selected_obj) {
    uint64_t locked_object = dock->locked_obj;
    bool checked = locked_object != 0;
    if (ct_ui_a0->checkbox(&(ct_ui_checkbox_t0) {.text=ICON_FA_LOCK}, &checked)) {
        if (checked) {
            dock->locked_obj = selected_obj;
        } else {
            dock->locked_obj = 0;
        }
    }
}

static uint64_t _selected_obj(dock_t *dock) {
    if (dock->locked_obj) {
        return dock->locked_obj;
    }

    return ct_selected_object_a0->selected_object(dock->context);
}

bool context_btn(dock_t *dock) {
    char title[256] = {};
    snprintf(title, CE_ARRAY_LEN(title),
             ICON_FA_MAP_SIGNS
                     " %llu##dock_context_btn_%llx", dock->context, (uint64_t) dock);

    bool change = ct_ui_a0->button(&(ct_ui_button_t0) {.text=title});
    if (change) {
        ct_ui_a0->popup_open(ce_id_a0->id64("select_dock_context"));
    };

    ct_ui_a0->same_line(0, 4);

    bool has_prev = ct_selected_object_a0->has_previous(dock->context);
    bool has_next = ct_selected_object_a0->has_next(dock->context);

    snprintf(title, CE_ARRAY_LEN(title),
             ICON_FA_ARROW_LEFT
                     "##dock_context_btn_prev_selected%llx", (uint64_t) dock);

    if (!has_prev) {
        ct_editor_ui_a0->begin_disabled();
    }

    if (ct_ui_a0->button(&(ct_ui_button_t0) {.text=title})) {
        ct_selected_object_a0->set_previous(dock->context);
    };

    if (!has_prev) {
        ct_editor_ui_a0->end_disabled();
    }

    ct_ui_a0->same_line(0, 4);

    snprintf(title, CE_ARRAY_LEN(title),
             ICON_FA_ARROW_RIGHT
                     "##dock_context_btn_next_selected%llx", (uint64_t) dock);

    if (!has_next) {
        ct_editor_ui_a0->begin_disabled();
    }

    if (ct_ui_a0->button(&(ct_ui_button_t0) {.text=title})) {
        ct_selected_object_a0->set_next(dock->context);
    };

    if (!has_next) {
        ct_editor_ui_a0->end_disabled();
    }

    return change;
}

void draw_all() {
    const uint64_t n = ce_array_size(_G.docks);

    for (int i = 0; i < n; ++i) {
        dock_t *dock = &_G.docks[i];

        struct ct_dock_i0 *di = _find_dock_i(dock->type);

        if (di) {
            uint32_t flags = di->ui_flags ? di->ui_flags() : 0;

            uint64_t selected_obj = _selected_obj(dock);

            char title[128] = {};
            snprintf(title, CE_ARRAY_LEN(title), "%s##%s_dock%llu",
                     di->display_title(dock->content, selected_obj), di->name(0), dock->id);

            bool visible = true;

            ct_ui_popup_t0 popup = {.id=ce_id_a0->id64("select_dock_context")};

            if (ct_ui_a0->dock_begin(&(ct_ui_dock_t0) {.label=title, .flags=flags}, &visible)) {
                if (ct_ui_a0->popup_begin(&popup)) {
                    for (int j = 0; j < MAX_CONTEXT; ++j) {
                        snprintf(title, CE_ARRAY_LEN(title), "%d", j);

                        bool selected = dock->context == j;

                        if (ct_ui_a0->menu_item(&(ct_ui_menu_item_t0) {
                                .text=title,
                                .selected=selected})) {
                            dock->context = j;
                        }
                    }
                    ct_ui_a0->popup_end();
                }


                context_btn(dock);
                ct_ui_a0->same_line(0, -1);
                lock_selected_obj(dock, ct_selected_object_a0->selected_object(dock->context));
                if (di->draw_menu) {
                    di->draw_menu(dock->content, dock->context, selected_obj);
                }

                snprintf(title, CE_ARRAY_LEN(title), "%s##%s_child_dock%llu",
                         di->display_title(dock->content, selected_obj), di->name(0), dock->id);


                ct_ui_a0->child_begin(&(ct_ui_child_t0) {.id=dock->id});

                if (di->draw_ui) {
                    di->draw_ui(dock->content, dock->context, selected_obj);
                }

                ct_ui_a0->child_end();
            }

            ct_ui_a0->dock_end();

            if (!visible) {
                close_dock(dock);
            }
        }
    }
}

static const char *display_name(ct_dock_i0 *i) {
    const char *name = NULL;

    if (i->display_title) {
        name = i->display_title(0, 0);
    }

    if (!name) {
        name = "(no name)";
    }
    return name;
}

static void draw_menu() {
    const uint64_t n = ce_array_size(_G.docks);

    if (ct_ui_a0->menu_begin(&(ct_ui_menu_t0) {.text="Docks"})) {

        if (ct_ui_a0->menu_begin(&(ct_ui_menu_t0) {.text=ICON_FA_PLUS" ""Add new dock"})) {
            struct ce_api_entry_t0 it = ce_api_a0->first(CT_DOCK_I0);

            while (it.api) {
                struct ct_dock_i0 *i = (it.api);

                const char *name = display_name(i);


                if (ct_ui_a0->menu_item(&(ct_ui_menu_item_t0) {.text=name})) {
                    create_dock(i->type, true);
                }

                it = ce_api_a0->next(it);
            }

            ct_ui_a0->menu_end();
        }

        if (ct_ui_a0->menu_begin(&(ct_ui_menu_t0) {.text="Layout"})) {
            if (ct_ui_a0->menu_item(&(ct_ui_menu_item_t0) {.text="save"})) {
                struct ce_vio_t0 *f = ce_fs_a0->open(ASSET_BROWSER_SOURCE,
                                                     "core/default.dock_layout",
                                                     FS_OPEN_WRITE);
                ct_ui_a0->dock_save(f);
                ce_fs_a0->close(f);
            }

            if (ct_ui_a0->menu_item(&(ct_ui_menu_item_t0) {.text="Load"})) {
                ct_ui_a0->dock_load("core/default.dock_layout");
            }
            ct_ui_a0->menu_end();
        }

        ct_ui_a0->separator();

        for (int i = 0; i < n; ++i) {
            dock_t *dock = &_G.docks[i];

            struct ct_dock_i0 *di = _find_dock_i(dock->type);

            const char *name = display_name(di);

            char label[256] = {};
            snprintf(label, CE_ARRAY_LEN(label), "%s##menu_%llx", name, (uint64_t) dock);

            if (ct_ui_a0->menu_begin(&(ct_ui_menu_t0) {.text=label})) {
                if (ct_ui_a0->menu_item(&(ct_ui_menu_item_t0) {
                        .text=ICON_FA_WINDOW_CLOSE" ""Close"})) {
                    close_dock(dock);
                }
                ct_ui_a0->menu_end();
            }
        }

        ct_ui_a0->menu_end();
    }

    for (int i = 0; i < n; ++i) {
        dock_t *dock = &_G.docks[i];

        struct ct_dock_i0 *di = _find_dock_i(dock->type);

        if (di && di->draw_main_menu) {
            di->draw_main_menu(dock->content);
        }

    }
}

static uint64_t create_context(const char *name) {
    const uint64_t context_id = ce_id_a0->id64(name);
    const uint64_t context_idx = ce_array_size(_G.contexts);
    ce_array_push(_G.contexts, context_id, _G.allocator);
    return context_idx;
}


struct ct_dock_a0 dock_a0 = {
        .draw_all = draw_all,
        .draw_menu = draw_menu,
        .create_dock = create_dock,
};

struct ct_dock_a0 *ct_dock_a0 = &dock_a0;

static const ce_cdb_prop_def_t0 docks_layout_cdb_type_def[] = {
        {
                .name = "docks",
                .type = CE_CDB_TYPE_SET_SUBOBJECT,
        },
};

static const ce_cdb_prop_def_t0 dock_layout_cdb_type_def[] = {
        {
                .name = "index",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "label",
                .type = CE_CDB_TYPE_STR,
        },
        {
                .name = "location",
                .type = CE_CDB_TYPE_STR,
        },
        {
                .name = "x",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "y",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "size_x",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "size_y",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "status",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "active",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "opened",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "prev",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "next",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "child0",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "child1",
                .type = CE_CDB_TYPE_UINT64,
        },
        {
                .name = "parent",
                .type = CE_CDB_TYPE_UINT64,
        },
};

void CE_MODULE_LOAD(dock)(struct ce_api_a0 *api,
                          int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_module_a0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->add_api(CT_DOCK_A0_STR, ct_dock_a0, sizeof(dock_a0));

    ce_cdb_a0->reg_obj_type(DOCKS_LAYOUT_TYPE, CE_ARR_ARG(docks_layout_cdb_type_def));
    ce_cdb_a0->reg_obj_type(DOCK_LAYOUT_TYPE, CE_ARR_ARG(dock_layout_cdb_type_def));

    create_context("");
}

void CE_MODULE_UNLOAD(dock)(struct ce_api_a0 *api,
                            int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _G = (struct _G) {};
}