#include <cetech/kernel/hashlib/hashlib.h>
#include <cetech/kernel/config/config.h>
#include <cetech/kernel/memory/memory.h>
#include <cetech/kernel/api/api_system.h>
#include <cetech/kernel/yaml/ydb.h>
#include <cetech/kernel/containers/array.h>
#include <cetech/kernel/module/module.h>
#include "cetech/kernel/containers/map.inl"

#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/resource/resource.h>
#include <cetech/engine/ecs/ecs.h>

#include <cetech/playground/property_editor.h>
#include <cetech/playground/asset_browser.h>
#include <cetech/playground/entity_property.h>
#include <cetech/playground/explorer.h>
#include <cetech/kernel/ebus/ebus.h>


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_property_editor_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_explorer_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_ecs_a0);
CETECH_DECL_API(ct_ebus_a0);

using namespace celib;

struct comp {
    uint64_t name;
    ct_ep_on_component clb;
};

#define _G entity_property_global


static struct _G {
    uint64_t active_entity;
    struct ct_entity top_entity;
    struct ct_world active_world;

    comp *components;

    const char *filename;
    ct_alloc *allocator;
    ct_cdb_obj_t *obj;
} _G;


static void on_debugui() {
    if (!_G.filename) {
        return;
    }

    if (ct_debugui_a0.Button("Save", (float[2]) {0.0f})) {
        ct_ydb_a0.save(_G.filename);
    }
    ct_debugui_a0.SameLine(0.0f, -1.0f);

    ct_debugui_a0.InputText("asset",
                            (char *) _G.filename, strlen(_G.filename),
                            DebugInputTextFlags_ReadOnly, 0, NULL);

    if (ct_debugui_a0.CollapsingHeader("Entity",
                                       DebugUITreeNodeFlags_DefaultOpen)) {
        ct_debugui_a0.LabelText("Entity", "%llu", _G.active_entity);
    }

    struct ct_resource_id rid;
    ct_resource_a0.type_name_from_filename(_G.filename, &rid, NULL);

    uint64_t ent_type = ct_cdb_a0.read_uint64(_G.obj, CT_ID64_0("ent_type"), 0);
    for (int j = 0; j < ct_array_size(_G.components); ++j) {
        uint64_t cmask = ct_ecs_a0.component_mask(_G.components[j].name);
        if (ent_type & cmask) {
            _G.components[j].clb(_G.active_world, _G.obj);
        }
    }
}

void on_entity_click(uint32_t bus_name,
                     void *event) {

    ct_ent_selected_ev *ev = static_cast<ct_ent_selected_ev *>(event);

    ct_property_editor_a0.set_active(on_debugui);

    _G.active_world  = ev->world;
    _G.top_entity = ev->entity;
    _G.filename = ev->filename;
    _G.obj = ev->obj;
}


void register_on_component_(uint64_t type,
                            ct_ep_on_component on_component) {
    comp item = {.name= type, .clb = on_component};
    ct_array_push(_G.components, item, _G.allocator);
}

void unregister_on_component_(uint64_t type) {
}


static ct_entity_property_a0 entity_property_a0 = {
        .register_component = register_on_component_,
        .unregister_component = unregister_on_component_,
};


static void _init(ct_api_a0 *api) {
    _G = {
            .allocator = ct_memory_a0.main_allocator()
    };

    api->register_api("ct_entity_property_a0", &entity_property_a0);

    ct_ebus_a0.connect(EXPLORER_EBUS, EXPLORER_ENTITY_SELECT_EVENT, on_entity_click, 0);
}

static void _shutdown() {
    ct_ebus_a0.disconnect(EXPLORER_EBUS, EXPLORER_ENTITY_SELECT_EVENT, on_entity_click);

    _G = {};
}

CETECH_MODULE_DEF(
        entity_property,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_property_editor_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_explorer_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_cdb_a0);
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
