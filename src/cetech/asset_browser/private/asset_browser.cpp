#include <cetech/debugui/debugui.h>
#include <cetech/asset_browser/asset_browser.h>
#include <cetech/debugui/private/ocornut-imgui/imgui.h>
#include <corelib/fs.h>
#include <corelib/os.h>
#include <cetech/resource/resource.h>
#include <cetech/editor/editor.h>
#include <corelib/ebus.h>
#include <cetech/selected_object/selected_object.h>
#include <cetech/debugui/private/iconfontheaders/icons_font_awesome.h>

#include "corelib/hashlib.h"
#include "corelib/config.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/module.h"
#include <cetech/dock/dock.h>
#include <cetech/kernel/kernel.h>
#include <cetech/texture/texture.h>
#include <cetech/asset_property/asset_property.h>
#include <cetech/asset_preview/asset_preview.h>

#define WINDOW_NAME "Asset browser"

static struct asset_browser_global {
    float left_column_width;
    float midle_column_width;
    char current_dir[512];

    uint64_t selected_dir_hash;
    uint64_t selected_file;
    uint32_t selected_file_idx;

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

    ct_alloc *allocator;
} _G;

static void set_current_dir(const char *dir,
                            uint64_t dir_hash) {
    strcpy(_G.current_dir, dir);
    _G.selected_dir_hash = dir_hash;
    _G.need_reaload = true;
}

static void ui_asset_filter() {
    _G.asset_filter.Draw(ICON_FA_SEARCH);
}

#define CURENT_DIR \
    CT_ID64_0(".", 0x223b2df3c7671369ULL)


static void ui_breadcrumb(const char *dir) {
    const size_t len = strlen(dir);

    char buffer[128] = {0};
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
                char tmp_dir[128] = {0};
                strncpy(tmp_dir, dir, sizeof(char) * (i + 1));
                uint64_t dir_hash = ct_hashlib_a0->id64(tmp_dir);
                set_current_dir(tmp_dir, dir_hash);
            };

            buffer_pos = 0;
        }
    }
}

static void ui_dir_list() {
    ImVec2 size = {_G.left_column_width, 0.0f};

    ImGui::BeginChild("left_col", size);
    ImGui::PushItemWidth(180);

    if (!_G.dirtree_list) {
        ct_fs_a0->listdir(SOURCE_ROOT, "", "*",
                          true, true, &_G.dirtree_list,
                          &_G.dirtree_list_count, _G.allocator);
    }


    if (ct_debugui_a0->TreeNode("Source")) {
        uint64_t dir_hash = CURENT_DIR;

        if (ImGui::Selectable(".", _G.selected_dir_hash == dir_hash)) {
            set_current_dir("", dir_hash);
        }

        for (uint32_t i = 0; i < _G.dirtree_list_count; ++i) {
            dir_hash = ct_hashlib_a0->id64(_G.dirtree_list[i]);

            char label[128];

            bool is_selected = _G.selected_dir_hash == dir_hash;

            if (is_selected) {
                snprintf(label, CT_ARRAY_LEN(label), ICON_FA_FOLDER_OPEN " %s",
                         _G.dirtree_list[i]);
            } else {
                snprintf(label, CT_ARRAY_LEN(label), ICON_FA_FOLDER " %s",
                         _G.dirtree_list[i]);
            }

            if (ImGui::Selectable(label, is_selected)) {
                set_current_dir(_G.dirtree_list[i], dir_hash);
            }
        }

        ct_debugui_a0->TreePop();
    }

    ImGui::PopItemWidth();
    ImGui::EndChild();
}

static void ui_asset_tooltip(ct_resource_id resourceid, const char* path) {
    ImGui::Text("%s", path);

    ct_resource_i0* ri = ct_resource_a0->get_interface(resourceid.type);

    if(!ri || !ri->get_interface) {
        return;
    }

    ct_asset_preview_i0* ai = \
                (ct_asset_preview_i0*)(ri->get_interface(ASSET_PREVIEW));

    if(!ai->tooltip) {
        return;
    }

    ai->tooltip(resourceid);
}

static void ui_asset_list() {
    ImVec2 size = {_G.midle_column_width, 0.0f};

    ImGui::BeginChild("middle_col", size);

    if (_G.need_reaload) {
        if (_G.asset_list) {
            ct_fs_a0->listdir_free(_G.asset_list, _G.asset_list_count,
                                   _G.allocator);
        }

        if (_G.dir_list) {
            ct_fs_a0->listdir_free(_G.dir_list, _G.dir_list_count,
                                   _G.allocator);
        }

        ct_fs_a0->listdir(SOURCE_ROOT,
                          _G.current_dir, "*",
                          false, false, &_G.asset_list,
                          &_G.asset_list_count, _G.allocator);

        ct_fs_a0->listdir(SOURCE_ROOT,
                          _G.current_dir, "*",
                          true, false, &_G.dir_list,
                          &_G.dir_list_count, _G.allocator);

        _G.need_reaload = false;
    }

    if (_G.dir_list) {
        char dirname[128] = {0};
        for (uint32_t i = 0; i < _G.dir_list_count; ++i) {
            const char *path = _G.dir_list[i];
            ct_os_a0->path->dirname(dirname, path);
            uint64_t filename_hash = ct_hashlib_a0->id64(dirname);

            if (!_G.asset_filter.PassFilter(dirname)) {
                continue;
            }

            char label[128];

            bool is_selected = _G.selected_file == filename_hash;

            snprintf(label, CT_ARRAY_LEN(label), ICON_FA_FOLDER" %s", dirname);

            if (ImGui::Selectable(label, is_selected,
                                  ImGuiSelectableFlags_AllowDoubleClick)) {
                _G.selected_file = filename_hash;

                if (ImGui::IsMouseDoubleClicked(0)) {
                    set_current_dir(path, ct_hashlib_a0->id64(path));
                }
            }
        }
    }

    if (_G.asset_list) {
        for (uint32_t i = 0; i < _G.asset_list_count; ++i) {
            const char *path = _G.asset_list[i];
            const char *filename = ct_os_a0->path->filename(path);
            uint64_t filename_hash = ct_hashlib_a0->id64(filename);

            if (!_G.asset_filter.PassFilter(filename)) {
                continue;
            }

            struct ct_resource_id resourceid = {.i128={0}};
            ct_resource_a0->type_name_from_filename(path, &resourceid, NULL);


            char label[128];

            snprintf(label, CT_ARRAY_LEN(label), ICON_FA_FILE " %s", filename);

            bool selected = ImGui::Selectable(label, _G.selected_file == filename_hash,
                                              ImGuiSelectableFlags_AllowDoubleClick);


            if(ImGui::IsItemHovered()) {
                ct_debugui_a0->BeginTooltip();
                ui_asset_tooltip(resourceid, path);
                ct_debugui_a0->EndTooltip();
            }

            if (selected) {
                _G.selected_file = filename_hash;
                _G.selected_file_idx = i;


                if (ImGui::IsMouseDoubleClicked(0)) {
                    uint64_t event;
                    event = ct_cdb_a0->create_object(ct_cdb_a0->db(),
                                                     ASSET_DCLICK_EVENT);

                    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(event);
                    ct_cdb_a0->set_uint64(w, ASSET_BROWSER_ASSET_NAME,
                                          resourceid.name);
                    ct_cdb_a0->set_uint64(w, ASSET_BROWSER_ASSET_TYPE2,
                                          resourceid.type);
                    ct_cdb_a0->set_str(w, ASSET_BROWSER_PATH, path);
                    ct_cdb_a0->set_uint64(w, ASSET_BROWSER_ROOT,
                                          ASSET_BROWSER_SOURCE);
                    ct_cdb_a0->write_commit(w);

                    ct_ebus_a0->broadcast(ASSET_BROWSER_EBUS, event);
                }

                uint64_t selected_asset = ct_cdb_a0->create_object(
                        ct_cdb_a0->db(), ASSET_BROWSER_ASSET_TYPE);

                ct_cdb_obj_o *w = ct_cdb_a0->write_begin(selected_asset);
                ct_cdb_a0->set_uint64(w, ASSET_BROWSER_ASSET_NAME,
                                      resourceid.name);
                ct_cdb_a0->set_uint64(w, ASSET_BROWSER_ASSET_TYPE2,
                                      resourceid.type);
                ct_cdb_a0->set_str(w, ASSET_BROWSER_PATH, path);
                ct_cdb_a0->write_commit(w);

                ct_selected_object_a0->set_selected_object(selected_asset);
            }

            if (ImGui::BeginDragDropSource(
                    ImGuiDragDropFlags_SourceAllowNullID)) {

                ui_asset_tooltip(resourceid, path);

                uint64_t selected_asset = ct_cdb_a0->create_object(ct_cdb_a0->db(),
                                                          ASSET_BROWSER_ASSET_TYPE);

                ct_cdb_obj_o *w = ct_cdb_a0->write_begin(selected_asset);
                ct_cdb_a0->set_uint64(w, ASSET_BROWSER_ASSET_NAME,
                                      resourceid.name);
                ct_cdb_a0->set_uint64(w, ASSET_BROWSER_ASSET_TYPE2,
                                      resourceid.type);
                ct_cdb_a0->set_str(w, ASSET_BROWSER_PATH, path);
                ct_cdb_a0->write_commit(w);

                ImGui::SetDragDropPayload("asset", &selected_asset,
                                          sizeof(uint64_t),
                                          ImGuiCond_Once);

                ImGui::EndDragDropSource();
            }
        }
    }

    ImGui::EndChild();
}


static void on_debugui(struct ct_dock_i0 *dock) {

    float content_w = ImGui::GetContentRegionAvailWidth();

    if (_G.midle_column_width < 0) {
        _G.midle_column_width = content_w - _G.left_column_width - 180;
    }

    ui_breadcrumb(_G.current_dir);
    ui_asset_filter();
    ui_dir_list();

    float left_size[] = {_G.left_column_width, 0.0f};
    ct_debugui_a0->SameLine(0.0f, -1.0f);
    ct_debugui_a0->VSplitter("vsplit1", left_size);
    _G.left_column_width = left_size[0];
    ct_debugui_a0->SameLine(0.0f, -1.0f);

    ui_asset_list();
}


static const char *dock_title(struct ct_dock_i0 *dock) {
    return ICON_FA_FOLDER_OPEN " Asset browser";
}

static const char *name(struct ct_dock_i0 *dock) {
    return "asset_browser";
}

static struct ct_dock_i0 ct_dock_i0 = {
        .id = 0,
        .visible = true,
        .display_title = dock_title,
        .name = name,
        .draw_ui = on_debugui,
};


static void _init(struct ct_api_a0 *api) {
    api->register_api(DOCK_INTERFACE_NAME, &ct_dock_i0);

    _G = {
            .allocator = ct_memory_a0->system,
    };

    ct_ebus_a0->create_ebus(ASSET_BROWSER_EBUS_NAME, ASSET_BROWSER_EBUS);

    _G.visible = true;
    _G.left_column_width = 180.0f;

}


static void _shutdown() {
    _G = {};
}

CETECH_MODULE_DEF(
        asset_browser,
        {
            CT_INIT_API(api, ct_memory_a0);
            CT_INIT_API(api, ct_hashlib_a0);
            CT_INIT_API(api, ct_debugui_a0);
            CT_INIT_API(api, ct_fs_a0);
            CT_INIT_API(api, ct_os_a0);
            CT_INIT_API(api, ct_resource_a0);
            CT_INIT_API(api, ct_ebus_a0);
            CT_INIT_API(api, ct_cdb_a0);
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