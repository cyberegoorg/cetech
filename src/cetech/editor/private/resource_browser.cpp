#include <fnmatch.h>

#include <celib/fs.h>
#include <celib/buffer.inl>
#include <celib/os.h>
#include "celib/hashlib.h"
#include "celib/config.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/module.h"
#include <celib/hash.inl>
#include <celib/log.h>

#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/editor/resource_browser.h>
#include <cetech/debugui/private/ocornut-imgui/imgui.h>
#include <cetech/resource/resource.h>
#include <cetech/editor/editor.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/editor/dock.h>
#include <cetech/kernel/kernel.h>
#include <cetech/texture/texture.h>
#include <cetech/editor/resource_preview.h>
#include <cetech/resource/resourcedb.h>
#include <cetech/resource/resource_compiler.h>
#include <cetech/editor/selcted_object.h>
#include <cetech/editor/log_view.h>

#include <cetech/editor/editor_ui.h>
#include <cetech/controlers/controlers.h>
#include <cetech/controlers/keyboard.h>


#define WINDOW_NAME "Resource browser"

#define _G asset_browser_global

static struct _G {
    float left_column_width;
    float midle_column_width;
    char current_dir[512];

    uint64_t selected_dir_hash;
    uint64_t selected_file;
    uint32_t selected_file_idx;
    ct_resource_id selected_asset;
    ct_resource_id edit_asset;

    const char *root;
    bool visible;

    char **dirtree_list;
    uint32_t dirtree_list_count;

    bool need_reaload;

    ImGuiTextFilter asset_filter;

    char **asset_list;
    uint32_t asset_list_count;

    char **dir_list;
    uint32_t dir_list_count;

    ce_alloc *allocator;
} _G;

static void set_current_dir(const char *dir,
                            uint64_t dir_hash) {
    strcpy(_G.current_dir, dir);
    _G.selected_dir_hash = dir_hash;
    _G.need_reaload = true;
}

static void _broadcast_edit(uint64_t dock) {
    _G.edit_asset = _G.selected_asset;
}

static void _broadcast_selected(uint64_t dock) {
    uint64_t obj = _G.selected_asset.uid;

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), dock);

    const uint64_t context = ce_cdb_a0->read_uint64(reader, PROP_DOCK_CONTEXT,
                                                    0);
    ct_selected_object_a0->set_selected_object(context, obj);
}


static char modal_buffer[128] = {};
static char modal_buffer_name[128] = {};
static char modal_buffer_type[128] = {};
static char modal_buffer_from[128] = {};

static void _create_from_modal(const char *modal_id) {
    bool open = true;
    ct_debugui_a0->SetNextWindowSize((float[2]) {512, 512},
                                     static_cast<DebugUICond>(0));

    if (!modal_buffer_name[0]) {
        snprintf(modal_buffer_name, CE_ARRAY_LEN(modal_buffer_name),
                 "%s", _G.current_dir);
    }

    if (ct_debugui_a0->BeginPopupModal(modal_id, &open, 0)) {
        struct ct_controlers_i0 *kb = ct_controlers_a0->get(CONTROLER_KEYBOARD);

        if (kb->button_pressed(0, kb->button_index("escape"))) {
            ct_debugui_a0->CloseCurrentPopup();
            ct_debugui_a0->EndPopup();
            return;
        }

        char labelidi[128] = {'\0'};

        sprintf(labelidi, "##modal_create_from_name%llu", 1ULL);
        ct_debugui_a0->InputText(labelidi,
                                 modal_buffer_name,
                                 CE_ARRAY_LEN(modal_buffer_name),
                                 0, 0, NULL);

        uint64_t type = ce_id_a0->id64(modal_buffer_type);

        int cur_type_idx = 1;
        char *buffer = NULL;
        ce_hash_t type_hash = {};

        struct ce_api_entry it = ce_api_a0->first(RESOURCE_I);
        uint32_t idx = 1;
        while (it.api) {
            struct ct_resource_i0 *i = (struct ct_resource_i0 *) (it.api);

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

        sprintf(labelidi, "##modal_create_from_type_combo%llu", 1ULL);
        bool type_change = ct_debugui_a0->Combo2(labelidi, &cur_type_idx,
                                                 buffer, -1);

        if (type_change) {
            uint64_t type_id = ce_hash_lookup(&type_hash,
                                              cur_type_idx + 1,
                                              UINT64_MAX);
            const char *tn = ce_id_a0->str_from_id64(type_id);
            snprintf(modal_buffer_type, CE_ARRAY_LEN(modal_buffer_type),
                     "%s", tn);
        }

        ce_hash_free(&type_hash, _G.allocator);

        sprintf(labelidi, "Instance of##modal_create_from_name%llu", 1ULL);
        ct_debugui_a0->InputText(labelidi,
                                 modal_buffer_from,
                                 CE_ARRAY_LEN(modal_buffer_from),
                                 ImGuiInputTextFlags_ReadOnly, 0, NULL);

        ct_debugui_a0->Text(ICON_FA_SEARCH);
        ct_debugui_a0->SameLine(0, -1);
        sprintf(labelidi, "##modal_create_from_input%llu", 1ULL);
        ct_debugui_a0->InputText(labelidi,
                                 modal_buffer,
                                 CE_ARRAY_LEN(modal_buffer),
                                 0, 0, NULL);

        bool add = ct_debugui_a0->Button(ICON_FA_PLUS,
                                         (float[2]) {0.0f});

        if (add) {
            uint64_t new_res = 0;

            if (modal_buffer_from[0]) {
                uint64_t uid = ct_resourcedb_a0->get_uid(modal_buffer_from,
                                                         modal_buffer_type);
                if (uid) {
                    new_res = ce_cdb_a0->create_from(ce_cdb_a0->db(),
                                                     uid);
                }
            } else {
                new_res = ce_cdb_a0->create_object(ce_cdb_a0->db(), type);
            }

            if (new_res) {
                char filename[256] = {};
                snprintf(filename, CE_ARRAY_LEN(filename),
                         "%s.%s.yml", modal_buffer_name, modal_buffer_type);

                ct_resource_id rid = {.uid = new_res};

                ct_resourcedb_a0->put_resource(rid, modal_buffer_type,
                                               filename, modal_buffer_name);

                ct_resourcedb_a0->put_file(filename, 0);

                ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),
                                                         new_res);
                ce_cdb_a0->set_str(w, ASSET_NAME_PROP, modal_buffer_name);
                ce_cdb_a0->write_commit(w);

                ct_resource_i0 *ri = ct_resource_a0->get_interface(type);

                if (ri && ri->create_new) {
                    ri->create_new(new_res);
                }

                ct_resource_a0->save(new_res);

                _G.need_reaload = true;
                ct_debugui_a0->CloseCurrentPopup();
                ct_debugui_a0->EndPopup();
            }

            return;
        }


        char title[256] = {};
        snprintf(title, CE_ARRAY_LEN(title), "##child_new_asset_ab");

        ct_debugui_a0->BeginChild(title, (float[2]) {}, false,
                                  (DebugUIWindowFlags_) (0));

        char **asset_list = NULL;
        ct_resourcedb_a0->get_resource_from_dirs("", &asset_list,
                                                 _G.allocator);

        uint32_t dir_n = ce_array_size(asset_list);
        for (int i = 0; i < dir_n; ++i) {
            const char *path = asset_list[i];

            if (!path || !path[0]) {
                continue;
            }

            const char *filename = ce_os_a0->path->filename(path);
            uint64_t filename_hash = ce_id_a0->id64(filename);

            char filter[256] = {};
            snprintf(filter, CE_ARRAY_LEN(filter),
                     "*%s*", modal_buffer);

            if (0 != fnmatch(filter, path, FNM_CASEFOLD)) {
                continue;
            }

            const char *type = ce_os_a0->path->extension(path);
            uint64_t  resource_type = ce_id_a0->id64(type);

            char name[256] = {};
            uint32_t path_len = snprintf(name, CE_ARRAY_LEN(name),
                                         "%s", path);
            uint32_t type_len = strlen(type);

            name[path_len - type_len - 1] = '\0';

            char label[128];

            ct_resource_i0 *ri = ct_resource_a0->get_interface(resource_type);
            const char *icon = ri->display_icon ? ri->display_icon() : NULL;

            if (icon) {
                sprintf(label, "%s %s##modal_type_item_%s", icon, path, path);
            } else {
                sprintf(label, "%s##modal_type_item_%s", path, path);
            }

            bool selected = ImGui::Selectable(label,
                                              _G.selected_file == filename_hash,
                                              ImGuiSelectableFlags_DontClosePopups);

            if (ImGui::IsItemHovered()) {
                ct_resource_id rid = {
                        .uid = ct_resourcedb_a0->get_uid(name, type)
                };

                ct_debugui_a0->BeginTooltip();
                ct_editor_ui_a0->resource_tooltip(rid, path,
                                                  (float[2]) {256, 256});
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

        ct_resourcedb_a0->get_resource_from_dirs_clean(asset_list,
                                                       _G.allocator);

        ct_debugui_a0->EndChild();
        ct_debugui_a0->EndPopup();
    }
}


static char _selected_type[256] = {};

static void _select_type_modal(const char *modal_id) {
    bool open = true;
    ct_debugui_a0->SetNextWindowSize((float[2]) {512, 512},
                                     static_cast<DebugUICond>(0));


    if (ct_debugui_a0->BeginPopupModal(modal_id, &open, 0)) {
        struct ct_controlers_i0 *kb = ct_controlers_a0->get(CONTROLER_KEYBOARD);

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


        if (ct_debugui_a0->Selectable("All types", false, 0,
                                      (float[2]) {0.0f})) {
            _selected_type[0] = '\0';
        };

        struct ce_api_entry it = ce_api_a0->first(RESOURCE_I);
        while (it.api) {
            struct ct_resource_i0 *i = (struct ct_resource_i0 *) (it.api);


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
                                          (float[2]) {0.0f})) {
                snprintf(_selected_type,
                         CE_ARRAY_LEN(_selected_type), "%s", type);
            };

            it = ce_api_a0->next(it);
        }


        ct_debugui_a0->EndPopup();
    }
}

static void ui_asset_menu(uint64_t dock) {
    //    ct_debugui_a0->SameLine(0, -1);

    if (ct_debugui_a0->Button(ICON_FA_PENCIL, (float[2]) {0, 0})) {
        _broadcast_edit(dock);
    }

    ct_debugui_a0->SameLine(0, -1);
    if (ct_debugui_a0->Button(ICON_FA_FLOPPY_O, (float[2]) {0, 0})) {
        const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), dock);

        const uint64_t context = ce_cdb_a0->read_uint64(reader,
                                                        PROP_DOCK_CONTEXT,
                                                        0);

        uint64_t selected = ct_selected_object_a0->selected_object(context);
        ct_resource_a0->save(selected);
    }


    ct_debugui_a0->SameLine(0, -1);
    bool create_from = ct_debugui_a0->Button(
            ICON_FA_PLUS" " ICON_FA_FOLDER_OPEN,
            (float[2]) {0, 0});

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
                 "T: %s##select_type_%llx", ri->display_icon(), dock);
    } else {
        if (_selected_type[0]) {
            snprintf(title, CE_ARRAY_LEN(title),
                     "T: %s##select_type_%llx", _selected_type, dock);
        } else {
            snprintf(title, CE_ARRAY_LEN(title),
                     "T##select_type_%llx", dock);
        }
    }


    sprintf(modal_id, "select...##select_type_%llx", dock);
    if (ct_debugui_a0->Button(title, (float[2]) {})) {
        ct_debugui_a0->OpenPopup(modal_id);
    }
    _select_type_modal(modal_id);

    ct_debugui_a0->SameLine(0, -1);

    ct_debugui_a0->Text(ICON_FA_SEARCH);
    ct_debugui_a0->SameLine(0, -1);
    _G.asset_filter.Draw("");

}

#define CURENT_DIR \
    CE_ID64_0(".", 0x223b2df3c7671369ULL)

//static void ui_breadcrumb(const char *dir) {
//    const size_t len = strlen(dir);
//
//    char buffer[128] = {};
//    uint32_t buffer_pos = 0;
//
//    ct_debugui_a0->SameLine(0.0f, -1.0f);
//    if (ct_debugui_a0->Button("Source", (float[2]) {0.0f})) {
//        set_current_dir("", CURENT_DIR);
//    }
//
//    for (int i = 0; i < len; ++i) {
//        if (dir[i] != '/') {
//            buffer[buffer_pos++] = dir[i];
//        } else {
//            buffer[buffer_pos] = '\0';
//            ct_debugui_a0->SameLine(0.0f, -1.0f);
//            ct_debugui_a0->Text(">");
//            ct_debugui_a0->SameLine(0.0f, -1.0f);
//
//            if (ct_debugui_a0->Button(buffer, (float[2]) {0.0f})) {
//                char tmp_dir[128] = {};
//                strncpy(tmp_dir, dir, sizeof(char) * (i + 1));
//                uint64_t dir_hash = ce_id_a0->id64(tmp_dir);
//                set_current_dir(tmp_dir, dir_hash);
//            };
//
//            buffer_pos = 0;
//        }
//    }
//}

static void ui_dir_list() {
    ImVec2 size(_G.left_column_width, 0.0f);

    ImGui::BeginChild("left_col", size);
    ImGui::PushItemWidth(180);

    if (!_G.dirtree_list) {
        ce_fs_a0->listdir(SOURCE_ROOT, "", "*",
                          true, true, &_G.dirtree_list,
                          &_G.dirtree_list_count, _G.allocator);
    }


    if (ct_debugui_a0->TreeNode("Source")) {
        uint64_t dir_hash = CURENT_DIR;

        if (ImGui::Selectable(".", _G.selected_dir_hash == dir_hash)) {
            set_current_dir("", dir_hash);
        }

        char **dirs = NULL;
        ct_resourcedb_a0->get_resource_dirs(&dirs, _G.allocator);
        uint32_t dir_n = ce_array_size(dirs);
        for (int j = 0; j < dir_n; ++j) {
            const char *dirname = dirs[j];
            dir_hash = ce_id_a0->id64(dirs[j]);

            char label[128];

            bool is_selected = _G.selected_dir_hash == dir_hash;

            if (is_selected) {
                snprintf(label, CE_ARRAY_LEN(label), ICON_FA_FOLDER_OPEN " %s",
                         dirname);
            } else {
                snprintf(label, CE_ARRAY_LEN(label), ICON_FA_FOLDER " %s",
                         dirname);
            }

            if (ImGui::Selectable(label, is_selected)) {
                set_current_dir(dirname, dir_hash);
            }
        }

        ct_resourcedb_a0->get_resource_dirs_clean(dirs, _G.allocator);
        ce_array_free(dirs, _G.allocator);

        ct_debugui_a0->TreePop();
    }

    ImGui::PopItemWidth();
    ImGui::EndChild();
}


static void ui_resource_list(uint64_t dock) {
    float size[2] = {_G.midle_column_width, 0.0f};

    ct_debugui_a0->BeginChild("middle_col", size, false,
                              (DebugUIWindowFlags_) (0));


    if (_G.need_reaload) {
        if (_G.asset_list) {
            ct_resourcedb_a0->get_resource_from_dirs_clean(_G.asset_list,
                                                           _G.allocator);
            ce_array_free(_G.asset_list, _G.allocator);
        }

        ct_resourcedb_a0->get_resource_from_dirs(_G.current_dir,
                                                 &_G.asset_list,
                                                 _G.allocator);

        _G.need_reaload = false;
    }

    char buffer[256] = {};
    snprintf(buffer, CE_ARRAY_LEN(buffer), "##asset_list_child%llx", dock);

    ct_debugui_a0->BeginChild("#asset_", (float[2]) {},
                              false, (DebugUIWindowFlags_) (0));
    if (_G.asset_list) {
        uint32_t dir_n = ce_array_size(_G.asset_list);
        for (int i = 0; i < dir_n; ++i) {
            const char *path = _G.asset_list[i];
            const char *filename = ce_os_a0->path->filename(path);
            uint64_t filename_hash = ce_id_a0->id64(filename);

            if (!_G.asset_filter.PassFilter(filename)) {
                continue;
            }


            char filter[256] = {};
            snprintf(filter, CE_ARRAY_LEN(filter),
                     "*%s*", _selected_type);

            if (0 != fnmatch(filter, filename, FNM_CASEFOLD)) {
                continue;
            }

            struct ct_resource_id resourceid = {};
            ct_resourcedb_a0->get_resource_by_fullname(path, &resourceid);

            char label[128];


            uint64_t rtype = ct_resourcedb_a0->get_resource_type(resourceid);
            ct_resource_i0 *ri = ct_resource_a0->get_interface(rtype);

            if (ri && ri->display_icon) {
                snprintf(label, CE_ARRAY_LEN(label),
                         "%s %s", ri->display_icon(), filename);
            } else {
                snprintf(label, CE_ARRAY_LEN(label), ICON_FA_FILE " %s",
                         filename);
            }

            bool selected = ImGui::Selectable(label,
                                              _G.selected_file == filename_hash,
                                              ImGuiSelectableFlags_AllowDoubleClick);

            if (ImGui::IsItemHovered()) {
                ct_debugui_a0->BeginTooltip();
                ct_editor_ui_a0->resource_tooltip(resourceid, path,
                                                  (float[2]) {128, 128});
                ct_debugui_a0->EndTooltip();
            }

            if (selected) {
                _G.selected_asset = resourceid;
                _G.selected_file = filename_hash;
                _G.selected_file_idx = i;


                if (ImGui::IsMouseDoubleClicked(0)) {
                    _broadcast_edit(dock);
                } else {
                    _broadcast_selected(dock);
                }
            }

            if (ct_debugui_a0->BeginDragDropSource(
                    DebugUIDragDropFlags_SourceAllowNullID)) {

                ct_editor_ui_a0->resource_tooltip(resourceid, path,
                                                  (float[2]) {128, 128});

                ct_debugui_a0->SetDragDropPayload("asset",
                                                  &resourceid.uid,
                                                  sizeof(uint64_t),
                                                  DebugUICond_Once);
                ct_debugui_a0->EndDragDropSource();
            }
        }

    }
    ct_debugui_a0->EndChild();

    ct_debugui_a0->EndChild();
}


static void on_debugui(uint64_t dock) {
    _G.edit_asset.uid = 0;

    float content_w = ImGui::GetContentRegionAvailWidth();

    if (_G.midle_column_width < 0) {
        _G.midle_column_width = content_w - _G.left_column_width - 180;
    }

//    ui_breadcrumb(_G.current_dir);
    ui_dir_list();

    float left_size[] = {_G.left_column_width, 0.0f};
    ct_debugui_a0->SameLine(0.0f, -1.0f);
    ct_debugui_a0->VSplitter("vsplit1", left_size);
    _G.left_column_width = left_size[0];
    ct_debugui_a0->SameLine(0.0f, -1.0f);

    ui_resource_list(dock);
}


static const char *dock_title(uint64_t dock) {
    return ICON_FA_FOLDER_OPEN " Resource browser";
}

static const char *name(uint64_t dock) {
    return "asset_browser";
}

static uint64_t cdb_type() {
    return RESOURCE_BROWSER;
};

static uint64_t dock_flags() {
    return 0;
}

static void on_draw_menu(uint64_t dock) {
    ct_dock_a0->context_btn(dock);
    ct_debugui_a0->SameLine(0, -1);
    ui_asset_menu(dock);
}

static struct ct_dock_i0 ct_dock_i0 = {
        .cdb_type = cdb_type,
        .dock_flags = dock_flags,
        .display_title = dock_title,
        .name = name,
        .draw_ui = on_debugui,
        .draw_menu = on_draw_menu,
};


static uint64_t edited() {
    return _G.edit_asset.uid;
}

static struct ct_resource_browser_a0 resource_browser_api = {
        .edited = edited,
};

struct ct_resource_browser_a0 *ct_resource_browser_a0 = &resource_browser_api;


static void _init(struct ce_api_a0 *api) {
    api->register_api(DOCK_INTERFACE, &ct_dock_i0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    ct_dock_a0->create_dock(RESOURCE_BROWSER, true);

    _G.visible = true;
    _G.left_column_width = 180.0f;
}


static void _shutdown() {
    _G = (struct _G) {};
}

CE_MODULE_DEF(
        asset_browser,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_debugui_a0);
            CE_INIT_API(api, ce_fs_a0);
            CE_INIT_API(api, ce_os_a0);
            CE_INIT_API(api, ct_resource_a0);

            CE_INIT_API(api, ce_cdb_a0);
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
