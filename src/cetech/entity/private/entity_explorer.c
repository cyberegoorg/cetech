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
#include <celib/ydb.h>
#include <fcntl.h>
#include <cetech/gfx/private/iconfontheaders/icons_font_awesome.h>
#include <stdlib.h>
#include <celib/cdb.h>


static void ui_entity_item_end() {
    ct_debugui_a0->TreePop();
}

static uint64_t ui_entity_item_begin(uint64_t selected_obj,
                                     uint64_t obj,
                                     uint32_t id) {

    ImGuiTreeNodeFlags flags = DebugUITreeNodeFlags_OpenOnArrow |
                               DebugUITreeNodeFlags_OpenOnDoubleClick;

    uint64_t new_selected_object = 0;

    bool selected = selected_obj == obj;
    if (selected) {
        flags |= DebugUITreeNodeFlags_Selected;
    }

    uint64_t children = ce_cdb_a0->read_subobject(obj, ENTITY_CHILDREN, 0);

    const uint32_t children_n = ce_cdb_a0->prop_count(children);


    uint64_t components;
    components = ce_cdb_a0->read_subobject(obj, ENTITY_COMPONENTS, 0);

    const uint32_t component_n = ce_cdb_a0->prop_count(components);
    if (!children_n && !component_n) {
        flags |= DebugUITreeNodeFlags_Leaf;
    }

    char name[128] = {0};
    uint64_t uid = ce_cdb_a0->key(obj);
    const char *ent_name = ce_cdb_a0->read_str(obj, ENTITY_NAME, NULL);
    if (ent_name) {
        strcpy(name, ent_name);
    } else {
        snprintf(name, CE_ARRAY_LEN(name), "%llu", uid);
    }

    char label[128] = {0};
    snprintf(label, CE_ARRAY_LEN(label),
             ICON_FA_CUBE
                     " ""%s##%llu", name, uid);

    const bool open = ct_debugui_a0->TreeNodeEx(label, flags);
    if (ct_debugui_a0->IsItemClicked(0)) {
        new_selected_object = obj;
    }

    if (open) {
        const uint32_t component_n = ce_cdb_a0->prop_count(components);
        uint64_t keys[component_n];
        ce_cdb_a0->prop_keys(components, keys);

        for (uint32_t i = 0; i < component_n; ++i) {
            uint64_t key = keys[i];

            uint64_t component = ce_cdb_a0->read_subobject(components, key, 0);
            uint64_t type = ce_cdb_a0->type(component);

            struct ct_component_i0 *component_i;
            component_i = ct_ecs_a0->component->get_interface(type);
            if (!component_i || !component_i->get_interface) {
                continue;
            }

            struct ct_editor_component_i0 *editor;
            editor = component_i->get_interface(EDITOR_COMPONENT);

            if (!editor) {
                continue;
            }

            const char *component_display_name = editor->display_name();

            ImGuiTreeNodeFlags c_flags = DebugUITreeNodeFlags_Leaf;

            bool c_selected;
            c_selected = selected_obj == component;

            if (c_selected) {
                c_flags |= DebugUITreeNodeFlags_Selected;
            }

            char c_label[128] = {0};
            snprintf(c_label, CE_ARRAY_LEN(c_label),
                     "%s##component_%d", component_display_name, ++id);

            ct_debugui_a0->TreeNodeEx(c_label, c_flags);
            if (ct_debugui_a0->IsItemClicked(0)) {
                new_selected_object = component;
            }
            ct_debugui_a0->TreePop();
        }
    }

    if (open) {
        uint64_t keys[children_n];
        ce_cdb_a0->prop_keys(children, keys);

        for (uint32_t i = 0; i < children_n; ++i) {
            uint64_t key = keys[i];
            uint64_t child = ce_cdb_a0->read_subobject(children, key, 0);
            uint64_t new_selected_object2 = ui_entity_item_begin(selected_obj,
                                                                 child, ++id);
            if (new_selected_object2) {
                new_selected_object = new_selected_object2;
            }
        }
        ui_entity_item_end();
    }

    return new_selected_object;
}

static void draw_menu(uint64_t top_level_obj,
                      uint64_t selected_obj) {
    if (!selected_obj) {
        return;
    }

    ct_debugui_a0->SameLine(0, 10);

    uint64_t type = ce_cdb_a0->type(selected_obj);

    if (type == ENTITY_RESOURCE_ID) {
        uint64_t uid = ce_cdb_a0->key(selected_obj);

        bool add = ct_debugui_a0->Button(ICON_FA_PLUS, (float[2]) {0.0f});

        if (add) {
            uint64_t entity_obj;
            entity_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                                  ENTITY_RESOURCE_ID);

            uint64_t uid = ((uint64_t)rand() << 32) | rand(); // TODO: !!!!!

            uint64_t components_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                                               ENTITY_COMPONENTS);
            uint64_t children_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                                             ENTITY_CHILDREN);

            ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(entity_obj);
            ce_cdb_a0->set_subobject(writer, ENTITY_COMPONENTS, components_obj);
            ce_cdb_a0->set_subobject(writer, ENTITY_CHILDREN, children_obj);
            ce_cdb_a0->write_commit(writer);

            uint64_t add_children_obj;
            add_children_obj = ce_cdb_a0->read_subobject(selected_obj,
                                                         ENTITY_CHILDREN,
                                                         0);

            ce_cdb_obj_o *w = ce_cdb_a0->write_begin(add_children_obj);
            ce_cdb_a0->set_subobject(w, uid, entity_obj);
            ce_cdb_a0->write_commit(w);
        }
        ct_debugui_a0->SameLine(0, 10);

        if (ct_debugui_a0->Button(ICON_FA_MINUS, (float[2]) {0.0f})) {
            uint64_t parent = ce_cdb_a0->parent(selected_obj);
            ce_cdb_obj_o *w = ce_cdb_a0->write_begin(parent);
            ce_cdb_a0->remove_property(w, uid);
            ce_cdb_a0->write_commit(w);
        }
    }

}

static uint64_t draw_ui(uint64_t top_level_obj,
                        uint64_t selected_obj) {
    if (!top_level_obj) {
        return 0;
    }

    return ui_entity_item_begin(selected_obj, top_level_obj, rand());
}

static struct ct_explorer_i0 ct_explorer_i0 = {
        .draw_ui = draw_ui,
        .draw_menu = draw_menu,
};

static void _init(struct ce_api_a0 *api) {
    api->register_api(EXPLORER_INTERFACE_NAME, &ct_explorer_i0);
}

static void _shutdown() {
}

CE_MODULE_DEF(
        entity_explorer,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_debugui_a0);
            CE_INIT_API(api, ct_resource_a0);
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
