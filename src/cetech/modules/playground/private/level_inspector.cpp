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
#include <cetech/modules/playground/level_inspector.h>
#include <cetech/kernel/yamlng.h>
#include <cetech/kernel/ydb.h>
#include <cstdio>

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);

using namespace celib;

#define _G property_inspector_global
static struct _G {
    bool visible;

    uint64_t level_name;
    struct ct_entity level;
    struct ct_world world;

    Array<ct_li_on_entity> on_entity_click;
    const char *path;
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
               const char* path) {

    CEL_UNUSED(root);

    if( _G.level_name == name) {
        return;
    }

    _G.level_name = name;
    _G.level = level;
    _G.world = world;
    _G.path = path;
}

static ct_level_inspector_a0 level_inspector_api = {
        .set_level = set_level,

        .register_on_entity_click  = register_on_entity_click_,
        .unregister_on_entity_click = unregister_on_entity_click_,
};


static void on_debugui() {
    if (ct_debugui_a0.BeginDock("Level inspector", &_G.visible,
                                DebugUIWindowFlags_(0))) {

        ct_debugui_a0.LabelText("Level", "%lu", _G.level_name);

        if(_G.path) {
            uint64_t  tmp_keys[32] = {};
            uint64_t  children_keys[32] = {};
            uint32_t  children_keys_count = 0;

            tmp_keys[0] = ct_yng_a0.calc_key("children");
            ct_ydb_a0.get_map_keys(
                    _G.path,
                    tmp_keys, 1,
                    children_keys, CETECH_ARRAY_LEN(children_keys),
                    &children_keys_count);

            for (uint32_t i = 0; i < children_keys_count; ++i) {
                char buffer[256];
                sprintf(buffer, "%lu", children_keys[i]);

                tmp_keys[1] = children_keys[i];
                tmp_keys[2] = ct_yng_a0.calc_key("name");

                const char* name = ct_ydb_a0.get_string(_G.path, tmp_keys, 3, buffer);

                if (ct_debugui_a0.Selectable(name, false, 0,
                                             (float[]) {0.0f, 0.0f})) {
                    for (uint32_t j = 0;j < array::size(_G.on_entity_click); ++j) {
                        _G.on_entity_click[j](_G.world, _G.level, _G.path, tmp_keys, 2);
                    }
                }
            }
        }
    }

    ct_debugui_a0.EndDock();
}

static void _init(ct_api_a0 *api) {
    _G = {
            .visible = true
    };

    api->register_api("ct_level_inspector_a0", &level_inspector_api);
    ct_debugui_a0.register_on_debugui(on_debugui);

    _G.on_entity_click.init(ct_memory_a0.main_allocator());
}

static void _shutdown() {
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