#include <fnmatch.h>

#include <celib/memory/allocator.h>
#include <celib/macros.h>
#include <celib/fs.h>
#include <celib/containers/buffer.h>

#include "celib/id.h"
#include "celib/config.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include "celib/module.h"
#include <celib/containers/hash.h>
#include <celib/log.h>

#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/asset_browser/asset_browser.h>
#include <cetech/debugui/private/ocornut-imgui/imgui.h>
#include <cetech/resource/resource.h>
#include <cetech/editor/editor.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/editor/dock.h>
#include <cetech/kernel/kernel.h>
#include <cetech/texture/texture.h>
#include <cetech/asset_preview/asset_preview.h>
#include <cetech/resource/resourcedb.h>
#include <cetech/asset_io/asset_io.h>
#include <cetech/editor/selcted_object.h>

#include <cetech/editor/editor_ui.h>
#include <cetech/controlers/controlers.h>
#include <cetech/controlers/keyboard.h>
#include <celib/os/path.h>


#define WINDOW_NAME "Resource browser"

#define _G asset_browser_global

CE_MODULE(ct_resourcedb_a0);

static struct _G {
    char current_dir[512];

    uint64_t selected_file;
    ce_cdb_uuid_t0 selected_asset;
    ce_cdb_uuid_t0 edit_asset;

    const char *root;
    bool visible;

    bool need_reaload;

    ImGuiTextFilter asset_filter;

    char **asset_list;
    uint32_t asset_list_count;

    ce_alloc_t0 *allocator;
} _G;

static void set_current_dir(const char *dir) {
    strcpy(_G.current_dir, dir);
    _G.need_reaload = true;
}

static void _broadcast_edit() {
    _G.edit_asset = _G.selected_asset;
}

static void _broadcast_selected(uint64_t context) {
    ce_cdb_uuid_t0 obj_uid = _G.selected_asset;
    uint64_t obj = ce_cdb_a0->obj_from_uid(ce_cdb_a0->db(), obj_uid);
    ct_selected_object_a0->set_selected_object(context, obj);
}


static char modal_buffer[128] = {};
static char modal_buffer_name[128] = {};
static char modal_buffer_type[128] = {};
static char modal_buffer_from[128] = {};

static void _create_from_modal(const char *modal_id) {
    bool open = true;
    ce_vec2_t size = {512, 512};
    ct_debugui_a0->SetNextWindowSize(&size,
                                     static_cast<DebugUICond>(0));

    if (!modal_buffer_name[0]) {
        snprintf(modal_buffer_name, CE_ARRAY_LEN(modal_buffer_name),
                 "%s", _G.current_dir);
    }

    if (ct_debugui_a0->BeginPopupModal(modal_id, &open, 0)) {
        struct ct_controler_i0 *kb = ct_controlers_a0->get(CONTROLER_KEYBOARD);

        if (kb->button_pressed(0, kb->button_index("escape"))) {
            ct_debugui_a0->CloseCurrentPopup();
            ct_debugui_a0->EndPopup();
            return;
        }


        ct_debugui_a0->Columns(2, NULL, true);

        ct_debugui_a0->Text("Name");
        ct_debugui_a0->NextColumn();

        char labelidi[128] = {'\0'};

        sprintf(labelidi, "##modal_create_from_name%llu", 1ULL);
        ct_debugui_a0->InputText(labelidi,
                                 modal_buffer_name,
                                 CE_ARRAY_LEN(modal_buffer_name),
                                 0, 0, NULL);
        ct_debugui_a0->NextColumn();

        uint64_t type = ce_id_a0->id64(modal_buffer_type);

        int cur_type_idx = 1;
        char *buffer = NULL;
        ce_hash_t type_hash = {};

        struct ce_api_entry_t0 it = ce_api_a0->first(CT_RESOURCE_I);
        uint32_t idx = 1;
        while (it.api) {
            struct ct_resource_i0 *i = (ct_resource_i0 *) (it.api);

            if (i->cdb_type() == type) {
                cur_type_idx = idx - 1;
            }

            const char *type = ce_id_a0->str_from_id64(i->cdb_type());
            ce_hash_add(&type_hash, idx, i->cdb_type(), _G.allocator);

            ce_array_push_n(buffer, type, strlen(type) + 1, _G.allocator);

            ++idx;

            it = ce_api_a0->next(it);
        }
        ce_array_push(buffer, '\0', _G.allocator);

        ct_debugui_a0->Text("Type");
        ct_debugui_a0->NextColumn();

        sprintf(labelidi, "##modal_create_from_type_combo%llu", 1ULL);
        bool type_change = ct_debugui_a0->Combo2(labelidi, &cur_type_idx,
                                                 buffer, -1);
        ct_debugui_a0->NextColumn();

        if (type_change && !strlen(modal_buffer_from)) {
            uint64_t type_id = ce_hash_lookup(&type_hash,
                                              cur_type_idx + 1,
                                              UINT64_MAX);
            const char *tn = ce_id_a0->str_from_id64(type_id);
            snprintf(modal_buffer_type, CE_ARRAY_LEN(modal_buffer_type),
                     "%s", tn);
        }

        ce_buffer_free(buffer, _G.allocator);
        ce_hash_free(&type_hash, _G.allocator);

        ct_debugui_a0->Text("Instance of");
        ct_debugui_a0->NextColumn();
        sprintf(labelidi, "##modal_create_from_instance_name%llu", 1ULL);
        ct_debugui_a0->InputText(labelidi,
                                 modal_buffer_from,
                                 CE_ARRAY_LEN(modal_buffer_from),
                                 0, 0, NULL);
        ct_debugui_a0->NextColumn();

        ct_debugui_a0->Columns(1, NULL, true);

        ct_debugui_a0->Text(ICON_FA_SEARCH);
        ct_debugui_a0->SameLine(0, -1);
        sprintf(labelidi, "##modal_create_from_input%llu", 1ULL);
        ct_debugui_a0->InputText(labelidi,
                                 modal_buffer,
                                 CE_ARRAY_LEN(modal_buffer),
                                 0, 0, NULL);

        ct_debugui_a0->SameLine(0, -1);
        ce_vec2_t zero_v = {};
        bool add = ct_debugui_a0->Button(ICON_FA_PLUS" Create", &zero_v);
        if (add) {
            uint64_t new_res = 0;

            if (modal_buffer_from[0]) {
                ce_cdb_uuid_t0 uuid = ct_resourcedb_a0->get_file_resource(modal_buffer_from);
                if (uuid.id) {
                    new_res = ce_cdb_a0->create_from(ce_cdb_a0->db(), uuid.id);
                }
            } else {
                uint64_t type = ce_id_a0->id64(modal_buffer_type);
                new_res = ce_cdb_a0->create_object(ce_cdb_a0->db(), type);

                ct_resource_i0 *ri = ct_resource_a0->get_interface(type);

                if (ri && ri->create_new) {
                    ri->create_new(new_res);
                }
            }

            if (new_res) {
                char filename[256] = {};
                snprintf(filename, CE_ARRAY_LEN(filename),
                         "%s.%s", modal_buffer_name, modal_buffer_type);

                ce_cdb_uuid_t0 rid = ce_cdb_a0->obj_uid(ce_cdb_a0->db(), new_res);

                ct_resourcedb_a0->put_file(filename, 0);
                ct_resourcedb_a0->put_resource(rid, modal_buffer_type, filename);
                ct_resourcedb_a0->put_obj(ce_cdb_a0->db(), new_res, _G.allocator);
                ct_resource_a0->save(new_res);

                _G.need_reaload = true;
                ct_debugui_a0->CloseCurrentPopup();
                ct_debugui_a0->EndPopup();
            }

            return;
        }


        char title[256] = {};
        snprintf(title, CE_ARRAY_LEN(title), "##child_new_asset_ab");

        ct_debugui_a0->BeginChild(title, (ce_vec2_t) {}, false,
                                  (DebugUIWindowFlags_) (0));

        char **asset_list = NULL;
        ct_resourcedb_a0->list_resource_from_dirs("", &asset_list,
                                                  _G.allocator);

        uint32_t dir_n = ce_array_size(asset_list);
        for (int i = 0; i < dir_n; ++i) {
            const char *path = asset_list[i];

            if (!path || !path[0]) {
                continue;
            }

            const char *filename = ce_os_path_a0->filename(path);
            uint64_t filename_hash = ce_id_a0->id64(filename);

            char filter[256] = {};
            snprintf(filter, CE_ARRAY_LEN(filter), "*%s*", modal_buffer);

            if (0 != fnmatch(filter, path, FNM_CASEFOLD)) {
                continue;
            }


            ce_cdb_uuid_t0 rid = ct_resourcedb_a0->get_file_resource(path);

            uint64_t resource_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), rid.id);

            ct_resource_i0 *ri = ct_resource_a0->get_interface(resource_type);

            if (!ri) {
                continue;
            }

            const char *type = ri->name();

            char name[256] = {};
            snprintf(name, CE_ARRAY_LEN(name), "%s", path);

            char label[128];


            const char *icon =
                    ri && ri->display_icon ? ri->display_icon() : NULL;

            if (icon) {
                sprintf(label, "%s %s##modal_type_item_%s", icon, path, path);
            } else {
                sprintf(label, "%s##modal_type_item_%s", path, path);
            }

            bool selected = ImGui::Selectable(label,
                                              _G.selected_file == filename_hash,
                                              ImGuiSelectableFlags_DontClosePopups);

            struct ce_cdb_uuid_t0 r = ct_resourcedb_a0->get_file_resource(name);
            if (ct_debugui_a0->IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {

                ct_debugui_a0->BeginTooltip();
                ct_resource_preview_a0->resource_tooltip(r, path, (ce_vec2_t) {256, 256});
                ct_debugui_a0->EndTooltip();
            }

            if (selected) {
                snprintf(modal_buffer_name, CE_ARRAY_LEN(modal_buffer_name),
                         "%s", name);

                snprintf(modal_buffer_type, CE_ARRAY_LEN(modal_buffer_type),
                         "%s", type);

                snprintf(modal_buffer_from, CE_ARRAY_LEN(modal_buffer_from),
                         "%s", name);
            }
        }

        ct_resourcedb_a0->clean_resource_list(asset_list, _G.allocator);

        ct_debugui_a0->EndChild();

        ct_debugui_a0->EndPopup();
    }
}


static char _selected_type[256] = {};

static void _select_type_modal(const char *modal_id) {
    if (ct_debugui_a0->BeginPopup(modal_id, 0)) {
        struct ct_controler_i0 *kb = ct_controlers_a0->get(CONTROLER_KEYBOARD);

        if (kb->button_pressed(0, kb->button_index("escape"))) {
            ct_debugui_a0->CloseCurrentPopup();
            ct_debugui_a0->EndPopup();
            return;
        }


        char labelidi[128] = {'\0'};

        ct_debugui_a0->Text(ICON_FA_SEARCH);
        ct_debugui_a0->SameLine(0, -1);
        sprintf(labelidi, "##modal_create_from_input%llu", 1ULL);
        ct_debugui_a0->InputText(labelidi,
                                 modal_buffer,
                                 CE_ARRAY_LEN(modal_buffer),
                                 0, 0, NULL);

        char filter[256] = {};
        snprintf(filter, CE_ARRAY_LEN(filter),
                 "*%s*", modal_buffer);


        if (ct_debugui_a0->Selectable("All", false, 0, &CE_VEC2_ZERO)) {
            _selected_type[0] = '\0';
        };

        struct ce_api_entry_t0 it = ce_api_a0->first(CT_RESOURCE_I);
        while (it.api) {
            struct ct_resource_i0 *i = (ct_resource_i0 *) (it.api);


            const char *type = ce_id_a0->str_from_id64(i->cdb_type());

            const char *icon = i->display_icon ? i->display_icon() : NULL;

            if (0 != fnmatch(filter, type, FNM_CASEFOLD)) {
                it = ce_api_a0->next(it);
                continue;
            }

            if (icon) {
                sprintf(labelidi, "%s %s##modal_type_%s", icon, type, type);
            } else {
                sprintf(labelidi, "%s##modal_type_%s", type, type);
            }

            if (ct_debugui_a0->Selectable(labelidi, false, 0,
                                          &CE_VEC2_ZERO)) {
                snprintf(_selected_type,
                         CE_ARRAY_LEN(_selected_type), "%s", type);
            };

            it = ce_api_a0->next(it);
        }


        ct_debugui_a0->EndPopup();
    }
}

static void ui_asset_menu(uint64_t context) {
    //    ct_debugui_a0->SameLine(0, -1);

    if (ct_debugui_a0->Button(ICON_FA_PENCIL, &CE_VEC2_ZERO)) {
        _broadcast_edit();
    }

    ct_debugui_a0->SameLine(0, -1);
    if (ct_debugui_a0->Button(ICON_FA_FLOPPY_O, &CE_VEC2_ZERO)) {
        uint64_t selected = ct_selected_object_a0->selected_object(context);
        ct_resource_a0->save(selected);
    }


    ct_debugui_a0->SameLine(0, -1);
    bool create_from = ct_debugui_a0->Button(
            ICON_FA_PLUS" " ICON_FA_FOLDER_OPEN,
            &CE_VEC2_ZERO);

    char modal_id[128] = {'\0'};
    sprintf(modal_id, "select...##select_comp_%llu", 1ULL);

    _create_from_modal(modal_id);

    if (create_from) {
        ct_debugui_a0->OpenPopup(modal_id);
    }

    ct_debugui_a0->SameLine(0, -1);

    uint64_t resource_type = ce_id_a0->id64(_selected_type);
    ct_resource_i0 *ri = ct_resource_a0->get_interface(resource_type);

    char title[128] = {'\0'};

    if (ri && ri->display_icon) {
        snprintf(title, CE_ARRAY_LEN(title),
                 "T: %s##select_type_%llx", ri->display_icon(), 1ULL);
    } else {
        if (_selected_type[0]) {
            snprintf(title, CE_ARRAY_LEN(title),
                     "T: %s##select_type_%llx", _selected_type, 1ULL);
        } else {
            snprintf(title, CE_ARRAY_LEN(title),
                     "T: All##select_type_%llx", 1ULL);
        }
    }


    sprintf(modal_id, "select...##select_type_%llx", 1ULL);
    if (ct_debugui_a0->Button(title, &CE_VEC2_ZERO)) {
        ct_debugui_a0->OpenPopup(modal_id);
    }
    _select_type_modal(modal_id);

    ct_debugui_a0->SameLine(0, -1);

    ct_debugui_a0->Text(ICON_FA_SEARCH);
    ct_debugui_a0->SameLine(0, -1);
    _G.asset_filter.Draw("");

}


static void ui_resource_list(uint64_t context) {
    if (_G.need_reaload) {
        if (_G.asset_list) {
            ct_resourcedb_a0->clean_resource_list(_G.asset_list, _G.allocator);
            ce_array_free(_G.asset_list, _G.allocator);
        }

        ct_resourcedb_a0->list_resource_from_dirs(_G.current_dir, &_G.asset_list, _G.allocator);

        _G.need_reaload = false;
    }

    char buffer[256] = {};
    snprintf(buffer, CE_ARRAY_LEN(buffer), "##asset_list_child%llx", 1ULL);

    if (_G.asset_list) {
        uint32_t dir_n = ce_array_size(_G.asset_list);
        for (int i = 0; i < dir_n; ++i) {
            const char *path = _G.asset_list[i];
            const char *filename = ce_os_path_a0->filename(path);
            uint64_t filename_hash = ce_id_a0->id64(filename);

            if (!_G.asset_filter.PassFilter(path)) {
                continue;
            }


            char filter[256] = {};
            snprintf(filter, CE_ARRAY_LEN(filter),
                     "*%s*", _selected_type);

            if (0 != fnmatch(filter, filename, FNM_CASEFOLD)) {
                continue;
            }

            ce_cdb_uuid_t0 resourceid = ct_resourcedb_a0->get_file_resource(path);

            char label[128];

            uint64_t rtype = ct_resourcedb_a0->get_resource_type(resourceid);
            ct_resource_i0 *ri = ct_resource_a0->get_interface(rtype);

            if (ri && ri->display_icon) {
                snprintf(label, CE_ARRAY_LEN(label),
                         "%s %s", ri->display_icon(), path);
            } else {
                snprintf(label, CE_ARRAY_LEN(label), ICON_FA_FILE " %s",
                         path);
            }

            bool selected = ImGui::Selectable(label,
                                              _G.selected_file == filename_hash,
                                              ImGuiSelectableFlags_AllowDoubleClick);

//            if (ct_debugui_a0->IsItemHovered(0)) {
//                ct_debugui_a0->BeginTooltip();
//                ct_resource_preview_a0->resource_tooltip(resourceid, path,
//                                                         (ce_vec2_t) {128, 128});
//                ct_debugui_a0->EndTooltip();
//            }

            if (selected) {
                _G.selected_asset = resourceid;
                _G.selected_file = filename_hash;

                if (ct_debugui_a0->IsMouseDoubleClicked(0)) {
                    _broadcast_edit();
                } else {
                    _broadcast_selected(context);
                }
            }

            if (ct_debugui_a0->BeginDragDropSource(DebugUIDragDropFlags_SourceAllowNullID)) {
                ct_resource_preview_a0->resource_tooltip(resourceid, path, (ce_vec2_t) {128, 128});

                uint64_t obj = ce_cdb_a0->obj_from_uid(ce_cdb_a0->db(), resourceid);

                ct_debugui_a0->SetDragDropPayload("asset",
                                                  &obj,
                                                  sizeof(uint64_t),
                                                  DebugUICond_Once);
                ct_debugui_a0->EndDragDropSource();
            }
        }

    }
}


static void on_debugui(uint64_t content,
                       uint64_t context,
                       uint64_t selected_object) {
    _G.edit_asset.id = 0;
    ui_resource_list(context);
}

static const char *dock_title(uint64_t content,
                              uint64_t selected_object) {
    return ICON_FA_FOLDER_OPEN " Resource browser";
}

static const char *name() {
    return "asset_browser";
}

static uint64_t dock_flags() {
    return 0;
}

static void on_draw_menu(uint64_t content,
                         uint64_t context,
                         uint64_t selected_object) {
    ui_asset_menu(context);
}

static struct ct_dock_i0 dock_api = {
        .type = ASSET_BROWSER,
        .ui_flags = dock_flags,
        .display_title = dock_title,
        .name = name,
        .draw_ui = on_debugui,
        .draw_menu = on_draw_menu,
};


static uint64_t edited() {
    return _G.edit_asset.id;
}

static struct ct_resource_browser_a0 resource_browser_api = {
        .edited = edited,
};

struct ct_resource_browser_a0 *ct_resource_browser_a0 = &resource_browser_api;


static void _shutdown() {
    _G = (struct _G) {};
}


extern "C" {

void CE_MODULE_LOAD(asset_browser)(struct ce_api_a0 *api,
                                   int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_debugui_a0);
    CE_INIT_API(api, ce_fs_a0);
    CE_INIT_API(api, ct_resource_a0);

    CE_INIT_API(api, ce_cdb_a0);

    api->add_impl(CT_DOCK_I, &dock_api, sizeof(dock_api));

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    set_current_dir("");
    ct_dock_a0->create_dock(ASSET_BROWSER, true);

    _G.visible = true;
}

void CE_MODULE_UNLOAD(asset_browser)(struct ce_api_a0 *api,
                                     int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
    _shutdown();
}

}