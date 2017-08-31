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

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_yamlng_a0);
CETECH_DECL_API(ct_ydb_a0);

using namespace celib;

#define _G property_inspector_global
static struct _G {
    bool visible;

    ct_yamlng_document *document;

    uint64_t level_name;
    struct ct_level level;
    struct ct_world world;

    Array<ct_li_on_entity> on_entity_click;
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


void set_level(struct ct_world world, struct ct_level level, uint64_t name, uint64_t root, const char* path) {
    if( _G.level_name == name) {
        return;
    }

    _G.level_name = name;
    _G.level = level;
    _G.world = world;

    _G.document = ct_ydb_a0.get(path);
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

        if(_G.document) {
            ct_yamlng_node node = _G.document->get(_G.document->inst, 0);

            _G.document->foreach_dict_node(
                    _G.document->inst,
                    node,
                    [](struct ct_yamlng_node key,
                       struct ct_yamlng_node value,
                       void *_data) {

                        const char* key_str = _G.document->as_string(_G.document->inst, key, "INVALID");

                        if (ct_debugui_a0.TreeNodeEx(key_str, DebugUITreeNodeFlags_DefaultOpen)) {
                            _G.document->foreach_dict_node(
                                    _G.document->inst,
                                    value,
                                    [](struct ct_yamlng_node key,
                                       struct ct_yamlng_node value,
                                       void *_data) {

                                        const char* key_str = _G.document->as_string(_G.document->inst, key, "INVALID");
                                        uint64_t name_id = ct_hash_a0.id64_from_str(key_str);
                                        if (ct_debugui_a0.Selectable(key_str, false, 0, (float[]){0.0f, 0.0f})) {
                                            for (uint32_t i = 0;
                                                 i < array::size(_G.on_entity_click); ++i) {
                                                _G.on_entity_click[i](_G.world, _G.level, name_id);
                                            }
                                        }

                                    }, NULL);

                            ct_debugui_a0.TreePop();
                        }

                    }, NULL);
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
            CETECH_GET_API(api, ct_yamlng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
        },
        {
            _init(api);
        },
        {
            CEL_UNUSED(api);
            _shutdown();
        }
)