#include <float.h>
#include <stdio.h>

#include <celib/hashlib.h>
#include <celib/config.h>
#include <celib/memory.h>
#include <celib/api_system.h>
#include <celib/ydb.h>
#include <celib/array.inl>
#include <celib/module.h>

#include <cetech/gfx/debugui.h>
#include <cetech/resource/resource.h>
#include <cetech/ecs/ecs.h>

#include <cetech/editor/property_editor.h>
#include <cetech/editor/asset_browser.h>
#include <cetech/editor/explorer.h>
#include <celib/ebus.h>
#include <celib/fmath.inl>
#include <cetech/editor/command_system.h>
#include <celib/hash.inl>
#include <celib/yng.h>


#define _G entity_property_global

static struct _G {
    uint64_t active_entity;
    struct ct_entity top_entity;
    struct ct_world active_world;

    struct ce_hash_t components;

    struct ce_alloc *allocator;
    uint64_t obj;
} _G;

static struct ct_component_i0 *get_component_interface(uint64_t cdb_type) {
    struct ce_api_entry it = ce_api_a0->first(COMPONENT_INTERFACE);
    while (it.api) {
        struct ct_component_i0 *i = (it.api);

        if (cdb_type == i->cdb_type()) {
            return i;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
};

static void draw_component(uint64_t obj) {
    uint64_t type = ce_cdb_a0->type(obj);

    struct ct_component_i0 *c = get_component_interface(type);
    if (!c->get_interface) {
        return;
    }

    struct ct_editor_component_i0 *editor = c->get_interface(EDITOR_COMPONENT);

    ct_debugui_a0->Separator();
    if (!ct_debugui_a0->TreeNodeEx(editor->display_name(),
                                         DebugUITreeNodeFlags_DefaultOpen)) {
        ct_debugui_a0->Separator();
        ct_debugui_a0->NextColumn();
        ct_debugui_a0->NextColumn();


        return;
    }

    ct_debugui_a0->Separator();
    ct_debugui_a0->NextColumn();
    ct_debugui_a0->NextColumn();

    if (!editor->property_editor) {
        return;
    }

    editor->property_editor(obj);

    ct_debugui_a0->TreePop();

}

static void draw_ui(uint64_t obj) {
    if (!obj) {
        return;
    }

    uint64_t obj_type = ce_cdb_a0->type(obj);

    if (ENTITY_RESOURCE == obj_type) {
//        if (ct_debugui_a0->CollapsingHeader("Entity",
//                                            DebugUITreeNodeFlags_DefaultOpen)) {
//            ct_debugui_a0->NextColumn();
//
//            ct_debugui_a0->LabelText("Entity", "%llu", _G.active_entity);
//        }

        uint64_t components_obj;
        components_obj = ce_cdb_a0->read_subobject(obj, ENTITY_COMPONENTS, 0);

        uint64_t n = ce_cdb_a0->prop_count(components_obj);
        uint64_t components_name[n];
        ce_cdb_a0->prop_keys(components_obj, components_name);

        for (uint64_t j = 0; j < n; ++j) {
            uint64_t name = components_name[j];

            uint64_t c_obj;
            c_obj = ce_cdb_a0->read_subobject(components_obj, name, 0);

            draw_component(c_obj);
        }

    } else if (get_component_interface(obj_type)) {
        draw_component(obj);
    }
}

static struct ct_property_editor_i0 ct_property_editor_i0 = {
        .draw_ui = draw_ui,
};

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system
    };

    api->register_api(PROPERTY_EDITOR_INTERFACE_NAME, &ct_property_editor_i0);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CE_MODULE_DEF(
        entity_property,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_debugui_a0);
            CE_INIT_API(api, ct_resource_a0);
            CE_INIT_API(api, ce_yng_a0);
            CE_INIT_API(api, ce_ydb_a0);
            CE_INIT_API(api, ct_ecs_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ce_ebus_a0);
            CE_INIT_API(api, ct_cmd_system_a0);
        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)
