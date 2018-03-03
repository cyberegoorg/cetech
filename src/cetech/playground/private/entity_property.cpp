#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/config/config.h>
#include <cetech/core/memory/memory.h>
#include <cetech/core/api/api_system.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/core/containers/array.h>
#include <cetech/core/module/module.h>
#include "cetech/core/containers/map.inl"

#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/resource/resource.h>
#include <cetech/engine/world/world.h>

#include <cetech/playground/property_editor.h>
#include <cetech/playground/asset_browser.h>
#include <cetech/playground/entity_property.h>
#include <cetech/playground/explorer.h>


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
CETECH_DECL_API(ct_world_a0);

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
        uint64_t cmask = ct_world_a0.component_mask(_G.components[j].name);
        if (ent_type & cmask) {
            _G.components[j].clb(_G.active_world, _G.obj);
        }
    }
}

void on_entity_click(struct ct_world world,
                     struct ct_entity entity,
                     const char *filename,
                     ct_cdb_obj_t* obj) {
    ct_property_editor_a0.set_active(on_debugui);

    _G.active_world = world;
    _G.top_entity = entity;
    _G.filename = filename;
    _G.obj = obj;
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
    ct_explorer_a0.register_on_entity_click(on_entity_click);
}

static void _shutdown() {

    ct_explorer_a0.unregister_on_entity_click(on_entity_click);

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
            CETECH_GET_API(api, ct_world_a0);
            CETECH_GET_API(api, ct_cdb_a0);
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
