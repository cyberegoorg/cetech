#include "celib/map.inl"

#include <cetech/debugui/debugui.h>
#include <cetech/playground/asset_browser.h>
#include <cetech/debugui/private/ocornut-imgui/imgui.h>
#include <cetech/filesystem/filesystem.h>
#include <cetech/os/path.h>
#include <cetech/resource/resource.h>
#include <cetech/playground/playground.h>
#include <celib/array.h>

#include "cetech/hashlib/hashlib.h"
#include "cetech/config/config.h"
#include "cetech/os/memory.h"
#include "cetech/api/api_system.h"
#include "cetech/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_playground_a0);


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

    ct_ab_on_asset_click *on_asset_click;
    ct_ab_on_asset_double_click *on_asset_double_click;

    cel_alloc *allocator;
} _G;

#define _DEF_ON_CLB_FCE(type, name)                                            \
    static void register_ ## name ## _(type name) {                            \
        cel_array_push(_G.name, name, _G.allocator);                           \
    }                                                                          \
    static void unregister_## name ## _(type name) {                           \
        const auto size = cel_array_size(_G.name);                             \
                                                                               \
        for(uint32_t i = 0; i < size; ++i) {                                   \
            if(_G.name[i] != name) {                                           \
                continue;                                                      \
            }                                                                  \
                                                                               \
            uint32_t last_idx = size - 1;                                      \
            _G.name[i] = _G.name[last_idx];                                    \
                                                                               \
            cel_array_pop_back(_G.name);                                       \
            break;                                                             \
        }                                                                      \
    }

_DEF_ON_CLB_FCE(ct_ab_on_asset_click, on_asset_click);

_DEF_ON_CLB_FCE(ct_ab_on_asset_double_click, on_asset_double_click);


uint64_t get_selected_asset_type() {
    const char *path = _G.asset_list[_G.selected_file_idx];

    uint64_t type, name;
    ct_resource_a0.type_name_from_filename(path,
                                           &type, &name,
                                           NULL);

    return type;
}

void get_selected_asset_name(char *asset_name) {
    const char *path = _G.asset_list[_G.selected_file_idx];

    uint64_t type, name;
    ct_resource_a0.type_name_from_filename(path,
                                           &type, &name,
                                           asset_name);
}

static ct_asset_browser_a0 asset_browser_api = {
        .register_on_asset_click = register_on_asset_click_,
        .unregister_on_asset_click = unregister_on_asset_click_,
        .register_on_asset_double_click =  register_on_asset_double_click_,
        .unregister_on_asset_double_click = unregister_on_asset_double_click_,
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
        cel_alloc *a = ct_memory_a0.main_allocator();
        ct_filesystem_a0.listdir(CT_ID64_0("source"), "", "*",
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
        cel_alloc *a = ct_memory_a0.main_allocator();

        if (_G.asset_list) {
            ct_filesystem_a0.listdir_free(_G.asset_list, _G.asset_list_count,
                                          a);
        }

        if (_G.dir_list) {
            ct_filesystem_a0.listdir_free(_G.dir_list, _G.dir_list_count, a);
        }

        ct_filesystem_a0.listdir(CT_ID64_0("source"),
                                 _G.current_dir, "*",
                                 false, false, &_G.asset_list,
                                 &_G.asset_list_count, a);

        ct_filesystem_a0.listdir(CT_ID64_0("source"),
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

            uint64_t type, name;
            ct_resource_a0.type_name_from_filename(path,
                                                   &type, &name,
                                                   NULL);

            if (ImGui::Selectable(filename, _G.selected_file == filename_hash,
                                  ImGuiSelectableFlags_AllowDoubleClick)) {
                _G.selected_file = filename_hash;
                _G.selected_file_idx = i;

                if (ImGui::IsMouseDoubleClicked(0)) {
                    for (uint32_t j = 0;
                         j < cel_array_size(_G.on_asset_double_click); ++j) {
                        _G.on_asset_double_click[j](type, name,
                                                    CT_ID64_0("source"), path);
                    }
                } else {
                    for (uint32_t j = 0;
                         j < cel_array_size(_G.on_asset_click); ++j) {
                        _G.on_asset_click[j](type, name,
                                             CT_ID64_0("source"), path);
                    }
                }
            }
        }
    }

    ImGui::EndChild();
}


static void on_debugui() {
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

static void on_menu_window() {
    ct_debugui_a0.MenuItem2(WINDOW_NAME, NULL, &_G.visible, true);
}

static void _init(ct_api_a0 *api) {
    api->register_api("ct_asset_browser_a0", &asset_browser_api);

    ct_playground_a0.register_module(
            PLAYGROUND_MODULE_NAME,
            (ct_playground_module_fce) {
                    .on_ui = on_debugui,
                    .on_menu_window = on_menu_window,
            });

    _G = {
            .allocator = ct_memory_a0.main_allocator(),
    };
    _G.visible = true;
    _G.left_column_width = 180.0f;

}

static void _shutdown() {
    ct_playground_a0.unregister_module(
            PLAYGROUND_MODULE_NAME
    );

    cel_array_free(_G.on_asset_click, _G.allocator);
    cel_array_free(_G.on_asset_double_click, _G.allocator);

    _G = {};
}

CETECH_MODULE_DEF(
        asset_browser,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_playground_a0);
        },
        {
            CEL_UNUSED(reload);
            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
            _shutdown();
        }
)