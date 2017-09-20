#include "celib/map.inl"

#include <cetech/modules/debugui/debugui.h>
#include <cetech/kernel/resource.h>
#include <cetech/modules/level/level.h>
#include <cetech/modules/entity/entity.h>
#include <cetech/kernel/hashlib.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/module.h>

#include <cetech/modules/playground/property_editor.h>
#include <cetech/modules/playground/asset_browser.h>
#include <cetech/modules/playground/entity_property.h>
#include <cetech/modules/playground/explorer.h>
#include <cetech/kernel/ydb.h>


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_property_editor_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_explorer_a0);
CETECH_DECL_API(ct_level_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_entity_a0);

using namespace celib;

#define _G entity_property_global
static struct _G {
    uint64_t active_entity;
    uint64_t keys[64];
    uint64_t keys_count;
    struct ct_entity top_entity;
    struct ct_world active_world;

    //Array<ct_ep_on_component> on_component;
    Map<ct_ep_on_component> on_component;
    const char *filename;
} _G;

//#define _DEF_ON_CLB_FCE(type, name)                                            \
//    static void register_ ## name ## _(type name) {                            \
//        celib::array::push_back(_G.name, name);                                \
//    }                                                                          \
//    static void unregister_## name ## _(type name) {                           \
//        const auto size = celib::array::size(_G.name);                         \
//                                                                               \
//        for(uint32_t i = 0; i < size; ++i) {                                   \
//            if(_G.name[i] != name) {                                           \
//                continue;                                                      \
//            }                                                                  \
//                                                                               \
//            uint32_t last_idx = size - 1;                                      \
//            _G.name[i] = _G.name[last_idx];                                    \
//                                                                               \
//            celib::array::pop_back(_G.name);                                   \
//            break;                                                             \
//        }                                                                      \
//    }
//
//_DEF_ON_CLB_FCE(ct_ep_on_component, on_component);


static void on_debugui() {
    if (!_G.filename) {
        return;
    }

    if (ct_debugui_a0.CollapsingHeader("Entity", DebugUITreeNodeFlags_DefaultOpen)) {
        ct_debugui_a0.LabelText("Entity", "%lu", _G.active_entity);
    }

    struct ct_entity entity = ct_entity_a0.find_by_guid(_G.top_entity,
                                                       _G.active_entity);

    uint64_t tmp_keys[_G.keys_count + 3];
    memcpy(tmp_keys, _G.keys, sizeof(uint64_t) * _G.keys_count);

    tmp_keys[_G.keys_count] = ct_yng_a0.calc_key("components");


    uint64_t component_keys[32] = {};
    uint32_t component_keys_count = 0;

    ct_ydb_a0.get_map_keys(
            _G.filename,
            tmp_keys, _G.keys_count + 1,
            component_keys, CETECH_ARRAY_LEN(component_keys),
            &component_keys_count);

    for (uint32_t i = 0; i < component_keys_count; ++i) {
        tmp_keys[_G.keys_count + 1] = component_keys[i];
        tmp_keys[_G.keys_count + 2] = ct_yng_a0.calc_key("component_type");

        const char *component_type = ct_ydb_a0.get_string(_G.filename, tmp_keys,
                                                          _G.keys_count + 3,
                                                          "");
        uint64_t component_type_hash = ct_hash_a0.id64_from_str(component_type);


        ct_ep_on_component on_component = map::get<ct_ep_on_component>(_G.on_component,
                                                   component_type_hash, NULL);

        if (on_component) {
            on_component(_G.active_world, entity, _G.filename, tmp_keys,
                         _G.keys_count + 2);
        }

    }
}

void on_entity_click(struct ct_world world,
                     struct ct_entity level,
                     const char *filename,
                     uint64_t *keys,
                     uint32_t keys_count) {
    ct_property_editor_a0.set_active(on_debugui);

    _G.active_world = world;
    _G.top_entity = level;
    _G.filename = filename;

    memcpy(_G.keys, keys, sizeof(uint64_t) * keys_count);
    _G.keys_count = keys_count;

    _G.active_entity = keys_count ? keys[keys_count - 1] : 0;
}


void register_on_component_(uint64_t type,
                            ct_ep_on_component on_component) {
    map::set(_G.on_component, type, on_component);
}

void unregister_on_component_(uint64_t type) {
    map::remove(_G.on_component, type);
}

static ct_entity_property_a0 entity_property_a0 = {
        .register_component = register_on_component_,
        .unregister_component = unregister_on_component_,
};


static void _init(ct_api_a0 *api) {
    _G = {};

    api->register_api("ct_entity_property_a0", &entity_property_a0);

    _G.on_component.init(ct_memory_a0.main_allocator());

    ct_explorer_a0.register_on_entity_click(on_entity_click);
}

static void _shutdown() {
    _G.on_component.destroy();

    ct_explorer_a0.unregister_on_entity_click(on_entity_click);

    _G = {};
}

CETECH_MODULE_DEF(
        entity_property,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_property_editor_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_explorer_a0);
            CETECH_GET_API(api, ct_level_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_entity_a0);
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
