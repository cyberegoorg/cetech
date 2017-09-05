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

#include <cetech/modules/playground/property_inspector.h>
#include <cetech/modules/playground/asset_browser.h>
#include <cetech/modules/playground/entity_property.h>
#include <cetech/modules/playground/level_inspector.h>


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_property_inspector_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_level_inspector_a0);
CETECH_DECL_API(ct_level_a0);

using namespace celib;

#define _G entity_property_global
static struct _G {
    uint64_t active_entity;
    struct ct_entity active_level;
    struct ct_world active_world;

    Array<ct_ep_on_component> on_component;
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

_DEF_ON_CLB_FCE(ct_ep_on_component, on_component);


static void on_debugui() {
    if (!_G.active_entity) {
        return;
    }

    ct_debugui_a0.LabelText("Entity", "%lu", _G.active_entity);

    struct ct_entity entity = ct_level_a0.entity_by_id(_G.active_level, _G.active_entity);

    for (uint32_t i = 0;i < array::size(_G.on_component); ++i) {
        _G.on_component[i](_G.active_world, entity);
    }

}

void on_entity_click(struct ct_world world, struct ct_entity level, uint64_t entity_id) {
    ct_property_inspector_a0.set_active(on_debugui);

    _G.active_world = world;
    _G.active_level = level;
    _G.active_entity = entity_id;
}

static ct_entity_property_a0 entity_property_a0 = {
    .register_component = register_on_component_,
    .unregister_component = unregister_on_component_,
};


static void _init(ct_api_a0 *api) {
    _G = {};

    api->register_api("ct_entity_property_a0", &entity_property_a0);

    _G.on_component.init(ct_memory_a0.main_allocator());

    ct_level_inspector_a0.register_on_entity_click(on_entity_click);
}

static void _shutdown() {
    _G.on_component.destroy();

    ct_level_inspector_a0.unregister_on_entity_click(on_entity_click);

    _G = {};
}

CETECH_MODULE_DEF(
        entity_property,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_property_inspector_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_level_inspector_a0);
            CETECH_GET_API(api, ct_level_a0);
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
