#include "celib/map.inl"

#include <cetech/modules/debugui/debugui.h>
#include <cetech/modules/playground/asset_browser.h>
#include <cetech/modules/debugui/private/ocornut-imgui/imgui.h>
#include <cetech/kernel/filesystem.h>
#include <cetech/kernel/path.h>
#include <cetech/kernel/resource.h>

#include "cetech/kernel/hashlib.h"
#include "cetech/kernel/config.h"
#include "cetech/kernel/memory.h"
#include "cetech/kernel/api_system.h"
#include "cetech/kernel/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_resource_a0);

using namespace celib;

#define _G property_inspector_global
static struct _G {
    float left_column_width;
    float midle_column_width;
    char current_dir[512];
    const char *root;
    bool visible;

    char **dirtree_list;
    uint32_t dirtree_list_count;

    bool reload_item_list;
    char **item_list;
    uint32_t item_list_count;

    Array<ct_ab_on_asset_click> on_asset_click;
    Array<ct_ab_on_asset_double_click> on_asset_double_click;
} _G;

#define _DEF_ON_CLB_FCE(type, name)                                            \
    static void register_ ## name ## _(type name) {                            \
        celib::array::push_back(_G.name, name);                                \
    }                                                                          \
    static void unregister_## name ## _(type name) {                           \
        const auto size = celib::array::size(_G.name);                         \
                                                                               \
        for(uint32_t i = 0; i < size; ++i) {                                   \
            if(_G.name[i] != name) {                                           \
                continue;                                                      \
            }                                                                  \
                                                                               \
            uint32_t last_idx = size - 1;                                      \
            _G.name[i] = _G.name[last_idx];                                    \
                                                                               \
            celib::array::pop_back(_G.name);                                   \
            break;                                                             \
        }                                                                      \
    }

_DEF_ON_CLB_FCE(ct_ab_on_asset_click, on_asset_click);

_DEF_ON_CLB_FCE(ct_ab_on_asset_double_click, on_asset_double_click);

static ct_asset_browser_a0 asset_browser_api = {
        .register_on_asset_click = register_on_asset_click_,
        .unregister_on_asset_click = unregister_on_asset_click_,
        .register_on_asset_double_click =  register_on_asset_double_click_,
        .unregister_on_asset_double_click = unregister_on_asset_double_click_,
};

static void dir_list_column() {
    ImVec2 size = {_G.left_column_width, 0.0f};

    ImGui::BeginChild("left_col", size);
    ImGui::PushItemWidth(120);

    if (!_G.dirtree_list) {
        cel_alloc *a = ct_memory_a0.main_allocator();
        ct_filesystem_a0.listdir(ct_hash_a0.id64_from_str("source"), "", "*",
                                 true, true, &_G.dirtree_list,
                                 &_G.dirtree_list_count, a);
    }

    if (ImGui::TreeNode("Source")) {
        if (ImGui::Selectable(".")) {
            strcpy(_G.current_dir, "");
            _G.reload_item_list = true;
        }

        for (uint32_t i = 0; i < _G.dirtree_list_count; ++i) {
            if (ImGui::Selectable(_G.dirtree_list[i])) {
                strcpy(_G.current_dir, _G.dirtree_list[i]);
                _G.reload_item_list = true;
            }
        }

        ImGui::TreePop();
    }

    ImGui::PopItemWidth();
    ImGui::EndChild();
}

static void item_list_column() {
    ImVec2 size = {_G.midle_column_width, 0.0f};

    ImGui::BeginChild("middle_col", size);

    if (_G.reload_item_list) {
        cel_alloc *a = ct_memory_a0.main_allocator();

        if(_G.item_list) {
            ct_filesystem_a0.listdir_free(_G.item_list, _G.item_list_count, a);
        }

        ct_filesystem_a0.listdir(ct_hash_a0.id64_from_str("source"),
                                 _G.current_dir, "*",
                                 false, false, &_G.item_list,
                                 &_G.item_list_count, a);

        _G.reload_item_list = false;
    }

    if (_G.item_list) {
        for (uint32_t i = 0; i < _G.item_list_count; ++i) {
            const char *path = _G.item_list[i];
            const char *filename = ct_path_a0.filename(path);

            uint64_t type, name;
            ct_resource_a0.type_name_from_filename(path,
                                                   &type, &name,
                                                   NULL);
            if (ImGui::Selectable(filename, false,
                                  ImGuiSelectableFlags_AllowDoubleClick)) {
                if (ImGui::IsMouseDoubleClicked(0)) {
                    for (uint32_t i = 0;
                         i < array::size(_G.on_asset_double_click); ++i) {
                        _G.on_asset_double_click[i](type, name, ct_hash_a0.id64_from_str("source"), path);
                    }
                } else {
                    for (uint32_t i = 0;
                         i < array::size(_G.on_asset_click); ++i) {
                        _G.on_asset_click[i](type, name, ct_hash_a0.id64_from_str("source"), path);
                    }
                }
            }
        }
    }

    ImGui::EndChild();
}


static void on_debugui() {
    if (ct_debugui_a0.BeginDock("Asset browser",
                                &_G.visible,
                                DebugUIWindowFlags_(0))) {

        float content_w = ImGui::GetContentRegionAvailWidth();
        if (_G.midle_column_width < 0)
            _G.midle_column_width = content_w -
                                    _G.left_column_width -
                                    120;

        dir_list_column();

        float left_size[] = {_G.left_column_width, 0.0f};
        ct_debugui_a0.SameLine(0.0f, -1.0f);
        ct_debugui_a0.VSplitter("vsplit1", left_size);
        _G.left_column_width = left_size[0];
        ct_debugui_a0.SameLine(0.0f, -1.0f);

        item_list_column();

    }

    ct_debugui_a0.EndDock();
}

static void _init(ct_api_a0 *api) {
    _G = {
            .visible = true,
            .left_column_width = 120.0f
    };

    api->register_api("ct_asset_browser_a0", &asset_browser_api);
    ct_debugui_a0.register_on_debugui(on_debugui);

    _G.on_asset_click.init(ct_memory_a0.main_allocator());
    _G.on_asset_double_click.init(ct_memory_a0.main_allocator());
}

static void _shutdown() {
    _G.on_asset_click.destroy();
    _G.on_asset_double_click.destroy();

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