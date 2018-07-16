#include <float.h>
#include <stdio.h>

#include <corelib/hashlib.h>
#include <corelib/config.h>
#include <corelib/memory.h>
#include <corelib/api_system.h>
#include <corelib/ydb.h>
#include <corelib/array.inl>
#include <corelib/module.h>

#include <cetech/debugui/debugui.h>
#include <cetech/resource/resource.h>
#include <cetech/ecs/ecs.h>

#include <cetech/property_editor/property_editor.h>
#include <cetech/asset_browser/asset_browser.h>
#include <cetech/ecs/entity_property.h>
#include <cetech/explorer/explorer.h>
#include <corelib/ebus.h>
#include <corelib/fmath.inl>
#include <cetech/command_system/command_system.h>
#include <corelib/hash.inl>
#include <cetech/selected_object/selected_object.h>
#include <corelib/yng.h>


#define _G entity_property_global

static struct _G {
    uint64_t active_entity;
    struct ct_entity top_entity;
    struct ct_world active_world;

    struct ct_hash_t components;

    struct ct_alloc *allocator;
    uint64_t obj;
} _G;

static struct ct_component_i0 *get_component_interface(uint64_t cdb_type) {
    struct ct_api_entry it = ct_api_a0->first(COMPONENT_INTERFACE);
    while (it.api) {
        struct ct_component_i0 *i = (it.api);

        if (cdb_type == i->cdb_type()) {
            return i;
        }

        it = ct_api_a0->next(it);
    }

    return NULL;
};

static void draw_component(uint64_t obj) {
    uint64_t type = ct_cdb_a0->type(obj);

    struct ct_component_i0 *c = get_component_interface(type);
    if (!c->get_interface) {
        return;
    }

    struct ct_editor_component_i0 *editor = c->get_interface(EDITOR_COMPONENT);

    if (!ct_debugui_a0->CollapsingHeader(editor->display_name(),
                                         DebugUITreeNodeFlags_DefaultOpen)) {
        return;
    }

    if (!editor->property_editor) {
        return;
    }

    editor->property_editor(obj);
}

static void draw_ui() {
    uint64_t obj = ct_selected_object_a0->selected_object();
    if (!obj) {
        return;
    }

    uint64_t obj_type = ct_cdb_a0->type(obj);

    if (ENTITY_RESOURCE == obj_type) {
        if (ct_debugui_a0->CollapsingHeader("Entity",
                                            DebugUITreeNodeFlags_DefaultOpen)) {
            ct_debugui_a0->LabelText("Entity", "%llu", _G.active_entity);
        }

        uint64_t components_obj;
        components_obj = ct_cdb_a0->read_subobject(obj, ENTITY_COMPONENTS, 0);

        uint64_t n = ct_cdb_a0->prop_count(components_obj);
        uint64_t components_name[n];
        ct_cdb_a0->prop_keys(components_obj, components_name);

        for (uint64_t j = 0; j < n; ++j) {
            uint64_t name = components_name[j];

            uint64_t c_obj;
            c_obj = ct_cdb_a0->read_subobject(components_obj, name, 0);

            draw_component(c_obj);
        }

    } else if (get_component_interface(obj_type)) {
        draw_component(obj);
    }
}

static struct ct_property_editor_i0 ct_property_editor_i0 = {
        .draw_ui = draw_ui,
};

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0->system
    };

    api->register_api(PROPERTY_EDITOR_INTERFACE_NAME, &ct_property_editor_i0);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        entity_property,
        {
            CT_INIT_API(api, ct_memory_a0);
            CT_INIT_API(api, ct_hashlib_a0);
            CT_INIT_API(api, ct_debugui_a0);
            CT_INIT_API(api, ct_resource_a0);
            CT_INIT_API(api, ct_yng_a0);
            CT_INIT_API(api, ct_ydb_a0);
            CT_INIT_API(api, ct_ecs_a0);
            CT_INIT_API(api, ct_cdb_a0);
            CT_INIT_API(api, ct_ebus_a0);
            CT_INIT_API(api, ct_cmd_system_a0);
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
