#include "celib/map.inl"

#include <cetech/modules/debugui/debugui.h>
#include <cetech/kernel/filesystem.h>
#include <cetech/kernel/hashlib.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/module.h>
#include <cetech/modules/level/level.h>
#include <cetech/modules/entity/entity.h>

#include <cetech/modules/playground/asset_browser.h>
#include <cetech/modules/playground/explorer.h>
#include <cetech/kernel/yamlng.h>
#include <cetech/kernel/ydb.h>
#include <cstdio>
#include <cetech/modules/playground/playground.h>
#include <cetech/modules/debugui/private/ocornut-imgui/imgui.h>

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_playground_a0);

using namespace celib;

#define WINDOW_NAME "Explorer"
#define PLAYGROUND_MODULE_NAME CT_ID64_0("explorer")

#define _G explorer_globals
static struct _G {
    bool visible;

    uint64_t selected_name;
    uint64_t level_name;
    struct ct_entity level;
    struct ct_world world;

    Array<ct_li_on_entity> on_entity_click;
    const char *path;
    bool is_level;
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

_DEF_ON_CLB_FCE(ct_li_on_entity, on_entity_click);


void set_level(struct ct_world world,
               struct ct_entity level,
               uint64_t name,
               uint64_t root,
               const char* path,
               bool is_level) {

    CEL_UNUSED(root);

    if( _G.level_name == name) {
        return;
    }

    _G.level_name = name;
    _G.level = level;
    _G.world = world;
    _G.path = path;
    _G.is_level = is_level;
}

static ct_explorer_a0 level_inspector_api = {
        .set_level = set_level,

        .register_on_entity_click  = register_on_entity_click_,
        .unregister_on_entity_click = unregister_on_entity_click_,
};


static void ui_entity_item_end() {
    ct_debugui_a0.TreePop();
}

static void ui_entity_item_begin(const char *name,
                                 uint64_t *keys,
                                 uint32_t keys_count) {

    uint64_t name_hash = ct_yng_a0.calc_key(name);
    bool selected = _G.selected_name == name_hash;

    uint64_t  children_keys[32] = {};
    uint32_t  children_keys_count = 0;

    uint64_t tmp_keys[keys_count+3];
    memcpy(tmp_keys, keys, sizeof(uint64_t) * keys_count);

    tmp_keys[keys_count] = ct_yng_a0.calc_key("children");
    ct_ydb_a0.get_map_keys(
            _G.path,
            tmp_keys, keys_count+1,
            children_keys, CETECH_ARRAY_LEN(children_keys),
            &children_keys_count);

    ImGuiTreeNodeFlags flags = DebugUITreeNodeFlags_OpenOnArrow | DebugUITreeNodeFlags_OpenOnDoubleClick;

    if(selected) {
        flags |= DebugUITreeNodeFlags_Selected;
    }

    if(children_keys_count == 0) {
        flags |= DebugUITreeNodeFlags_Leaf;
    }

    bool open = ct_debugui_a0.TreeNodeEx(name, flags);
    if(ImGui::IsItemClicked(0)) {
        if(_G.selected_name != name_hash) {
            for (uint32_t j = 0; j < array::size(_G.on_entity_click); ++j) {
                _G.on_entity_click[j](_G.world, _G.level, _G.path, keys,
                                      keys_count);
            }
        }

        _G.selected_name = name_hash;
    }

    if (open) {
        for (uint32_t i = 0; i < children_keys_count; ++i) {
            char buffer[256];
            sprintf(buffer, "%llu", children_keys[i]);

            tmp_keys[keys_count+1] = children_keys[i];
            tmp_keys[keys_count+2] = ct_yng_a0.calc_key("name");

            const char* name = ct_ydb_a0.get_string(_G.path, tmp_keys, keys_count+3, buffer);
            ui_entity_item_begin(name, tmp_keys, keys_count+2);
        }

        ui_entity_item_end();
    }
}



static void on_debugui() {
    if (ct_debugui_a0.BeginDock(WINDOW_NAME, &_G.visible,
                                DebugUIWindowFlags_(0))) {

        if(_G.is_level) {
            ct_debugui_a0.LabelText("Level", "%llu", _G.level_name);
        } else {
            ct_debugui_a0.LabelText("Entity", "%llu", _G.level_name);
        }

        if(_G.path) {
            uint64_t  tmp_keys[32] = {};
            tmp_keys[0] = ct_yng_a0.calc_key("name");
            const char* name = ct_ydb_a0.get_string(_G.path, tmp_keys, 1, "ROOT");
            ui_entity_item_begin(name, tmp_keys, 0);
        }
    }

    ct_debugui_a0.EndDock();
}

static void on_menu_window() {
    ct_debugui_a0.MenuItem2(WINDOW_NAME, NULL, &_G.visible, true);
}

static void _init(ct_api_a0 *api) {
    _G = {
            .visible = true
    };

    api->register_api("ct_explorer_a0", &level_inspector_api);

    ct_playground_a0.register_module(
            PLAYGROUND_MODULE_NAME,
            (ct_playground_module_fce){
                    .on_ui = on_debugui,
                    .on_menu_window = on_menu_window,
            });

    _G.on_entity_click.init(ct_memory_a0.main_allocator());
}

static void _shutdown() {
    ct_playground_a0.unregister_module(PLAYGROUND_MODULE_NAME);

    _G.on_entity_click.destroy();

    _G = {};
}

CETECH_MODULE_DEF(
        level_inspector,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
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