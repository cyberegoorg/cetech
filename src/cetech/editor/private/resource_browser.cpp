#include <cetech/debugui/debugui.h>
#include <cetech/editor/resource_browser.h>
#include <cetech/debugui/private/ocornut-imgui/imgui.h>
#include <celib/fs.h>
#include <celib/os.h>
#include <cetech/resource/resource.h>
#include <cetech/editor/editor.h>
#include <celib/ebus.h>
#include <cetech/debugui/icons_font_awesome.h>

#include "celib/hashlib.h"
#include "celib/config.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/module.h"
#include <cetech/editor/dock.h>
#include <cetech/kernel/kernel.h>
#include <cetech/texture/texture.h>
#include <cetech/editor/resource_preview.h>
#include <cetech/resource/builddb.h>
#include <cetech/resource/resource_compiler.h>
#include <cetech/editor/selcted_object.h>
#include <cetech/editor/log_view.h>

#define WINDOW_NAME "Asset browser"

#define _G asset_browser_global

static struct _G {
    float left_column_width;
    float midle_column_width;
    char current_dir[512];

    uint64_t selected_dir_hash;
    uint64_t selected_file;
    uint32_t selected_file_idx;
    ct_resource_id selected_asset;

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

static void _broadcast_edit() {
    uint64_t event;
    event = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                     RESOURCE_DCLICK_EVENT);

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(event);
    ce_cdb_a0->set_uint64(w, RESOURCE_NAME, _G.selected_asset.uid);
    ce_cdb_a0->set_uint64(w, RESOURCE_BROWSER_ROOT, RESOURCE_BROWSER_SOURCE);
    ce_cdb_a0->write_commit(w);

    ce_ebus_a0->broadcast_obj(RESOURCE_BROWSER_EBUS, RESOURCE_DCLICK_EVENT,
                              event);
}

static void _broadcast_selected(uint64_t dock) {
    uint64_t obj = _G.selected_asset.uid;

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(dock);

    const uint64_t context = ce_cdb_a0->read_uint64(reader, PROP_DOCK_CONTEXT,
                                                    0);
    ct_selected_object_a0->set_selected_object(context, obj);
}

static void ui_asset_menu() {
    ct_debugui_a0->Text(ICON_FA_SEARCH);
    ct_debugui_a0->SameLine(0, 8);
    _G.asset_filter.Draw("");

    ct_debugui_a0->SameLine(0, -1);

    if (ct_debugui_a0->Button("Edit asset", (float[2]) {0, 0})) {
        _broadcast_edit();
    }
}

#define CURENT_DIR \
    CE_ID64_0(".", 0x223b2df3c7671369ULL)

static void ui_breadcrumb(const char *dir) {
    const size_t len = strlen(dir);

    char buffer[128] = {};
    uint32_t buffer_pos = 0;

    ct_debugui_a0->SameLine(0.0f, -1.0f);
    if (ct_debugui_a0->Button("Source", (float[2]) {0.0f})) {
        set_current_dir("", CURENT_DIR);
    }

    for (int i = 0; i < len; ++i) {
        if (dir[i] != '/') {
            buffer[buffer_pos++] = dir[i];
        } else {
            buffer[buffer_pos] = '\0';
            ct_debugui_a0->SameLine(0.0f, -1.0f);
            ct_debugui_a0->Text(">");
            ct_debugui_a0->SameLine(0.0f, -1.0f);

            if (ct_debugui_a0->Button(buffer, (float[2]) {0.0f})) {
                char tmp_dir[128] = {};
                strncpy(tmp_dir, dir, sizeof(char) * (i + 1));
                uint64_t dir_hash = ce_id_a0->id64(tmp_dir);
                set_current_dir(tmp_dir, dir_hash);
            };

            buffer_pos = 0;
        }
    }
}

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
        ct_builddb_a0->get_resource_dirs(&dirs, _G.allocator);
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

        ct_builddb_a0->get_resource_dirs_clean(dirs, _G.allocator);
        ce_array_free(dirs, _G.allocator);

        ct_debugui_a0->TreePop();
    }

    ImGui::PopItemWidth();
    ImGui::EndChild();
}

static void ui_asset_tooltip(ct_resource_id resourceid,
                             const char *path) {
    ct_debugui_a0->Text("%s", path);

    uint64_t type = ct_builddb_a0->get_resource_type(resourceid);

    ct_resource_i0 *ri = ct_resource_a0->get_interface(type);

    if (!ri || !ri->get_interface) {
        return;
    }

    ct_resource_preview_i0 *ai = \
                (ct_resource_preview_i0 *) (ri->get_interface(
            RESOURCE_PREVIEW_I));

    if (!ai->tooltip) {
        return;
    }

    uint64_t obj = resourceid.uid;
    ai->tooltip(obj);
}

static void ui_asset_list(uint64_t dock) {
    ImVec2 size(_G.midle_column_width, 0.0f);

    ImGui::BeginChild("middle_col", size);

    if (_G.need_reaload) {
        if (_G.asset_list) {
            ct_builddb_a0->get_resource_from_dirs_clean(_G.asset_list,
                                                        _G.allocator);
            ce_array_free(_G.asset_list, _G.allocator);
        }


        ct_builddb_a0->get_resource_from_dirs(_G.current_dir, &_G.asset_list,
                                              _G.allocator);

        _G.need_reaload = false;
    }

    if (_G.asset_list) {
        uint32_t dir_n = ce_array_size(_G.asset_list);
        for (int i = 0; i < dir_n; ++i) {
            const char *path = _G.asset_list[i];
            const char *filename = ce_os_a0->path->filename(path);
            uint64_t filename_hash = ce_id_a0->id64(filename);

            if (!_G.asset_filter.PassFilter(filename)) {
                continue;
            }

            struct ct_resource_id resourceid = {};
            ct_builddb_a0->get_resource_by_fullname(path, &resourceid);

            char label[128];

            snprintf(label, CE_ARRAY_LEN(label), ICON_FA_FILE " %s", filename);

            bool selected = ImGui::Selectable(label,
                                              _G.selected_file == filename_hash,
                                              ImGuiSelectableFlags_AllowDoubleClick);


            if (ImGui::IsItemHovered()) {
                ct_debugui_a0->BeginTooltip();
                ui_asset_tooltip(resourceid, path);
                ct_debugui_a0->EndTooltip();
            }

            if (selected) {
                _G.selected_asset = resourceid;
                _G.selected_file = filename_hash;
                _G.selected_file_idx = i;


                if (ImGui::IsMouseDoubleClicked(0)) {
                    _broadcast_edit();
                } else {
                    _broadcast_selected(dock);
                }
            }

            if (ct_debugui_a0->BeginDragDropSource(
                    DebugUIDragDropFlags_SourceAllowNullID)) {

                ui_asset_tooltip(resourceid, path);

                uint64_t selected_asset = ce_cdb_a0->create_object(
                        ce_cdb_a0->db(),
                        RESOURCE_BROWSER_ASSET_TYPE);

                ce_cdb_obj_o *w = ce_cdb_a0->write_begin(selected_asset);
                ce_cdb_a0->set_uint64(w, RESOURCE_NAME, resourceid.uid);
                ce_cdb_a0->write_commit(w);

                ct_debugui_a0->SetDragDropPayload("asset",
                                                  &selected_asset,
                                                  sizeof(uint64_t),
                                                  DebugUICond_Once);
                ct_debugui_a0->EndDragDropSource();
            }
        }

    }



//    if (_G.dir_list) {
//        char dirname[128] = {};
//        for (uint32_t i = 0; i < _G.dir_list_count; ++i) {
//            const char *path = _G.dir_list[i];
//            ce_os_a0->path->dirname(dirname, path);
//            uint64_t filename_hash = ce_id_a0->id64(dirname);
//
//            if (!_G.asset_filter.PassFilter(dirname)) {
//                continue;
//            }
//
//            char label[128];
//
//            bool is_selected = _G.selected_file == filename_hash;
//
//            snprintf(label, CE_ARRAY_LEN(label), ICON_FA_FOLDER" %s", dirname);
//
//            if (ImGui::Selectable(label, is_selected,
//                                  ImGuiSelectableFlags_AllowDoubleClick)) {
//                _G.selected_file = filename_hash;
//
//                if (ImGui::IsMouseDoubleClicked(0)) {
//                    set_current_dir(path, ce_id_a0->id64(path));
//                }
//            }
//        }
//    }


    ImGui::EndChild();
}


static void on_debugui(uint64_t dock) {

    float content_w = ImGui::GetContentRegionAvailWidth();

    if (_G.midle_column_width < 0) {
        _G.midle_column_width = content_w - _G.left_column_width - 180;
    }

    ui_breadcrumb(_G.current_dir);
    ui_asset_menu();
    ui_dir_list();

    float left_size[] = {_G.left_column_width, 0.0f};
    ct_debugui_a0->SameLine(0.0f, -1.0f);
    ct_debugui_a0->VSplitter("vsplit1", left_size);
    _G.left_column_width = left_size[0];
    ct_debugui_a0->SameLine(0.0f, -1.0f);

    ui_asset_list(dock);
}


static const char *dock_title(uint64_t dock) {
    return ICON_FA_FOLDER_OPEN " Asset browser";
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

static struct ct_dock_i0 ct_dock_i0 = {
        .cdb_type = cdb_type,
        .dock_flags = dock_flags,
        .display_title = dock_title,
        .name = name,
        .draw_ui = on_debugui,
};


static void _init(struct ce_api_a0 *api) {
    api->register_api(DOCK_INTERFACE_NAME, &ct_dock_i0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    ce_ebus_a0->create_ebus(RESOURCE_BROWSER_EBUS);

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
            CE_INIT_API(api, ce_ebus_a0);
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
