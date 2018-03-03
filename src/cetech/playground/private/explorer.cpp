#include "cetech/core/containers/map.inl"

#include <cetech/engine/debugui/debugui.h>
#include <cetech/core/fs/fs.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/config/config.h>
#include <cetech/core/memory/memory.h>
#include <cetech/core/api/api_system.h>
#include <cetech/core/module/module.h>
#include <cetech/engine/world/world.h>

#include <cetech/playground/asset_browser.h>
#include <cetech/playground/explorer.h>
#include <cetech/core/yaml/yng.h>
#include <cetech/core/yaml/ydb.h>
#include <cstdio>
#include <cetech/playground/playground.h>
#include <cetech/engine/debugui/private/ocornut-imgui/imgui.h>
#include <cetech/engine/resource/resource.h>

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_fs_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_playground_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_resource_a0);

using namespace celib;

#define WINDOW_NAME "Explorer"
#define PLAYGROUND_MODULE_NAME CT_ID64_0("explorer")

#define _G explorer_globals
static struct _G {
    bool visible;

    struct ct_cdb_obj_t* selected_obj;
    uint32_t ent_name;
    struct ct_entity entity;
    struct ct_world world;

    ct_li_on_entity *on_entity_click;
    const char *path;
    ct_alloc *allocator;
} _G;

#define _DEF_ON_CLB_FCE(type, name)                                            \
    static void register_ ## name ## _(type name) {                            \
        ct_array_push(_G.name, name, _G.allocator);                           \
    }                                                                          \
    static void unregister_## name ## _(type name) {                           \
        const auto size = ct_array_size(_G.name);                             \
                                                                               \
        for(uint32_t i = 0; i < size; ++i) {                                   \
            if(_G.name[i] != name) {                                           \
                continue;                                                      \
            }                                                                  \
                                                                               \
            uint32_t last_idx = size - 1;                                      \
            _G.name[i] = _G.name[last_idx];                                    \
                                                                               \
            ct_array_pop_back(_G.name);                                       \
            break;                                                             \
        }                                                                      \
    }

_DEF_ON_CLB_FCE(ct_li_on_entity, on_entity_click);


void set_level(struct ct_world world,
               struct ct_entity level,
               uint64_t name,
               uint64_t root,
               const char *path) {

    CT_UNUSED(root);

    if (_G.ent_name == name) {
        return;
    }

    _G.ent_name = name;
    _G.entity = level;
    _G.world = world;
    _G.path = path;
}

static ct_explorer_a0 level_inspector_api = {
        .set_level = set_level,

        .register_on_entity_click  = register_on_entity_click_,
        .unregister_on_entity_click = unregister_on_entity_click_,
};


static void ui_entity_item_end() {
    ct_debugui_a0.TreePop();
}

static void ui_entity_item_begin(ct_cdb_obj_t *obj) {

    ImGuiTreeNodeFlags flags = DebugUITreeNodeFlags_OpenOnArrow |
                               DebugUITreeNodeFlags_OpenOnDoubleClick;

    bool selected = _G.selected_obj == obj;
    if (selected) {
        flags |= DebugUITreeNodeFlags_Selected;
    }

    ct_cdb_obj_t** children = ct_cdb_a0.children(obj);
    if (ct_array_size(children) == 0) {
        flags |= DebugUITreeNodeFlags_Leaf;
    }


    char name[128] = {0};
    const char* ent_name = ct_cdb_a0.read_str(obj, CT_ID64_0("name"), NULL);
    if(ent_name) {
        strcpy(name, ent_name);
    } else {
        uint64_t uid = ct_cdb_a0.read_uint64(obj, CT_ID64_0("uid"), 0);
        snprintf(name, CT_ARRAY_LEN(name), "%llu", uid);
    }

    bool open = ct_debugui_a0.TreeNodeEx(name, flags);
    if (ImGui::IsItemClicked(0)) {
        if (_G.selected_obj != obj) {
            for (uint32_t j = 0; j < ct_array_size(_G.on_entity_click); ++j) {
                _G.on_entity_click[j](_G.world, _G.entity, _G.path, obj);
            }
        }

        _G.selected_obj = obj;
    }

    if (open) {
        ct_cdb_obj_t** children = ct_cdb_a0.children(obj);
        for (uint32_t i = 0; i < ct_array_size(children); ++i) {
            ui_entity_item_begin(children[i]);
        }
        ui_entity_item_end();
    }
}


static void on_debugui() {
    if (ct_debugui_a0.BeginDock(WINDOW_NAME, &_G.visible,
                                DebugUIWindowFlags_(0))) {

        ct_debugui_a0.LabelText("Entity", "%u", _G.ent_name);

        if (_G.path) {
            struct ct_resource_id rid = (struct ct_resource_id){
                    .type = CT_ID32_0("entity"),
                    .name = _G.ent_name,
            };

            ct_cdb_obj_t* obj = ct_resource_a0.get_obj(rid);

#define PROP_ENT_OBJ (CT_ID64_0("ent_obj") << 32)
            obj = ct_cdb_a0.read_ref(obj, PROP_ENT_OBJ, NULL);
            ui_entity_item_begin(obj);

        }
    }

    ct_debugui_a0.EndDock();
}

static void on_menu_window() {
    ct_debugui_a0.MenuItem2(WINDOW_NAME, NULL, &_G.visible, true);
}

static void _init(ct_api_a0 *api) {
    _G = {
            .allocator = ct_memory_a0.main_allocator(),
            .visible = true
    };

    api->register_api("ct_explorer_a0", &level_inspector_api);

    ct_playground_a0.register_module(
            PLAYGROUND_MODULE_NAME,
            (ct_playground_module_fce) {
                    .on_ui = on_debugui,
                    .on_menu_window = on_menu_window,
            });
}

static void _shutdown() {
    ct_playground_a0.unregister_module(PLAYGROUND_MODULE_NAME);

    ct_array_free(_G.on_entity_click, _G.allocator);

    _G = {};
}

CETECH_MODULE_DEF(
        level_inspector,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_fs_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_playground_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_resource_a0);
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