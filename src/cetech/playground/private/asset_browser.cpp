#include "cetech/core/containers/map.inl"

#include <cetech/engine/debugui/debugui.h>
#include <cetech/playground/asset_browser.h>
#include <cetech/engine/debugui/private/ocornut-imgui/imgui.h>
#include <cetech/core/fs/fs.h>
#include <cetech/core/os/path.h>
#include <cetech/engine/resource/resource.h>
#include <cetech/playground/playground.h>
#include <cetech/core/containers/array.h>
#include <cetech/core/ebus/ebus.h>

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/config/config.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_fs_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_playground_a0);
CETECH_DECL_API(ct_ebus_a0);


#define WINDOW_NAME "Asset browser"
#define PLAYGROUND_MODULE_NAME CT_ID64_0("asset_browser")

using namespace celib;

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


uint64_t get_selected_asset_type() {
    const char *path = _G.asset_list[_G.selected_file_idx];
    struct ct_resource_id resourceid;
    ct_resource_a0.type_name_from_filename(path, &resourceid, NULL);
    return resourceid.type;
}

void get_selected_asset_name(char *asset_name) {
    struct ct_resource_id resourceid;
    const char *path = _G.asset_list[_G.selected_file_idx];
    ct_resource_a0.type_name_from_filename(path, &resourceid, asset_name);
}

static ct_asset_browser_a0 asset_browser_api = {
        .get_selected_asset_name = get_selected_asset_name,
        .get_selected_asset_type = get_selected_asset_type

};


static void set_current_dir(const char *dir,
                            uint64_t dir_hash) {
    strcpy(_G.current_dir, dir);
    _G.selected_dir_hash = dir_hash;
    _G.need_reaload = true;
}

static void ui_asset_filter() {
    _G.asset_filter.Draw();
}

static void ui_breadcrumb(const char *dir) {
    const size_t len = strlen(dir);

    char buffer[128] = {0};
    uint32_t buffer_pos = 0;

    ct_debugui_a0.SameLine(0.0f, -1.0f);
    if (ct_debugui_a0.Button("Source", (float[2]) {0.0f})) {
        uint64_t dir_hash = CT_ID64_0(".");
        set_current_dir("", dir_hash);
    }

    for (int i = 0; i < len; ++i) {
        if (dir[i] != '/') {
            buffer[buffer_pos++] = dir[i];
        } else {
            buffer[buffer_pos] = '\0';
            ct_debugui_a0.SameLine(0.0f, -1.0f);
            ct_debugui_a0.Text(">");
            ct_debugui_a0.SameLine(0.0f, -1.0f);
            if (ct_debugui_a0.Button(buffer, (float[2]) {0.0f})) {
                char tmp_dir[128] = {0};
                strncpy(tmp_dir, dir, sizeof(char) * (i + 1));
                uint64_t dir_hash = CT_ID64_0(tmp_dir);
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
        ct_alloc *a = ct_memory_a0.main_allocator();
        ct_fs_a0.listdir(CT_ID64_0("source"), "", "*",
                                 true, true, &_G.dirtree_list,
                                 &_G.dirtree_list_count, a);
    }


    if (ImGui::TreeNode("Source")) {
        uint64_t dir_hash = CT_ID64_0(".");

        if (ImGui::Selectable(".", _G.selected_dir_hash == dir_hash)) {
            set_current_dir("", dir_hash);
        }

        for (uint32_t i = 0; i < _G.dirtree_list_count; ++i) {
            dir_hash = CT_ID64_0(_G.dirtree_list[i]);

            if (ImGui::Selectable(_G.dirtree_list[i],
                                  _G.selected_dir_hash == dir_hash)) {
                set_current_dir(_G.dirtree_list[i], dir_hash);
            }
        }

        ImGui::TreePop();
    }

    ImGui::PopItemWidth();
    ImGui::EndChild();
}

static void ui_asset_list() {
    ImVec2 size = {_G.midle_column_width, 0.0f};

    ImGui::BeginChild("middle_col", size);

    if (_G.need_reaload) {
        ct_alloc *a = ct_memory_a0.main_allocator();

        if (_G.asset_list) {
            ct_fs_a0.listdir_free(_G.asset_list, _G.asset_list_count,
                                          a);
        }

        if (_G.dir_list) {
            ct_fs_a0.listdir_free(_G.dir_list, _G.dir_list_count, a);
        }

        ct_fs_a0.listdir(CT_ID64_0("source"),
                                 _G.current_dir, "*",
                                 false, false, &_G.asset_list,
                                 &_G.asset_list_count, a);

        ct_fs_a0.listdir(CT_ID64_0("source"),
                                 _G.current_dir, "*",
                                 true, false, &_G.dir_list,
                                 &_G.dir_list_count, a);

        _G.need_reaload = false;
    }

    if (_G.dir_list) {
        char dirname[128] = {0};
        for (uint32_t i = 0; i < _G.dir_list_count; ++i) {
            const char *path = _G.dir_list[i];
            ct_path_a0.dirname(dirname, path);
            uint64_t filename_hash = CT_ID64_0(dirname);

            if (!_G.asset_filter.PassFilter(dirname)) {
                continue;
            }

            if (ImGui::Selectable(dirname, _G.selected_file == filename_hash,
                                  ImGuiSelectableFlags_AllowDoubleClick)) {
                _G.selected_file = filename_hash;

                if (ImGui::IsMouseDoubleClicked(0)) {
                    set_current_dir(path, CT_ID64_0(path));
                }
            }
        }
    }

    if (_G.asset_list) {
        for (uint32_t i = 0; i < _G.asset_list_count; ++i) {
            const char *path = _G.asset_list[i];
            const char *filename = ct_path_a0.filename(path);
            uint64_t filename_hash = CT_ID64_0(filename);

            if (!_G.asset_filter.PassFilter(filename)) {
                continue;
            }

            struct ct_resource_id resourceid;
            ct_resource_a0.type_name_from_filename(path, &resourceid, NULL);

            if (ImGui::Selectable(filename, _G.selected_file == filename_hash,
                                  ImGuiSelectableFlags_AllowDoubleClick)) {
                _G.selected_file = filename_hash;
                _G.selected_file_idx = i;

                ct_asset_browser_click_ev ev = {
                        .asset = resourceid.i64,
                        .path=path,
                        .root=CT_ID64_0("source")
                };

                if (ImGui::IsMouseDoubleClicked(0)) {
                    ct_ebus_a0.send(ASSET_BROWSER_EBUS, ASSET_DCLICK_EVENT, &ev,
                                    sizeof(ev));
                } else {
                    ct_ebus_a0.send(ASSET_BROWSER_EBUS, ASSET_CLICK_EVENT, &ev,
                                    sizeof(ev));
                }
            }
        }
    }

    ImGui::EndChild();
}


static void on_debugui(uint64_t bus_name,
                       void *event) {
    if (ct_debugui_a0.BeginDock(WINDOW_NAME,
                                &_G.visible,
                                DebugUIWindowFlags_(0))) {

        float content_w = ImGui::GetContentRegionAvailWidth();

        if (_G.midle_column_width < 0) {
            _G.midle_column_width = content_w - _G.left_column_width - 180;
        }

        ui_breadcrumb(_G.current_dir);
        ui_asset_filter();
        ui_dir_list();

        float left_size[] = {_G.left_column_width, 0.0f};
        ct_debugui_a0.SameLine(0.0f, -1.0f);
        ct_debugui_a0.VSplitter("vsplit1", left_size);
        _G.left_column_width = left_size[0];
        ct_debugui_a0.SameLine(0.0f, -1.0f);

        ui_asset_list();
    }

    ct_debugui_a0.EndDock();
}

static void on_menu_window(uint64_t bus_name,
                           void *event) {
    ct_debugui_a0.MenuItem2(WINDOW_NAME, NULL, &_G.visible, true);
}

static void _init(ct_api_a0 *api) {
    api->register_api("ct_asset_browser_a0", &asset_browser_api);


    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UI_EVENT, on_debugui);
    ct_ebus_a0.connect(PLAYGROUND_EBUS, PLAYGROUND_UI_MAINMENU_EVENT, on_menu_window);


    _G = {
            .allocator = ct_memory_a0.main_allocator(),
    };

    ct_ebus_a0.create_ebus(ASSET_BROWSER_EBUS_NAME);

    _G.visible = true;
    _G.left_column_width = 180.0f;

}

static void _shutdown() {

    _G = {};
}

CETECH_MODULE_DEF(
        asset_browser,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_fs_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_playground_a0);
            CETECH_GET_API(api, ct_ebus_a0);
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