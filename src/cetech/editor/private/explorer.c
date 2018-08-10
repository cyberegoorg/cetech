#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <celib/hashlib.h>
#include <celib/config.h>
#include <celib/memory.h>
#include <celib/api_system.h>
#include <celib/module.h>
#include <celib/cdb.h>
#include <celib/yng.h>
#include <celib/ydb.h>
#include <celib/fs.h>
#include <celib/ebus.h>
#include <celib/macros.h>


#include <cetech/ecs/ecs.h>
#include <cetech/gfx/debugui.h>
#include <cetech/editor/dock.h>
#include <cetech/editor/asset_browser.h>
#include <cetech/editor/explorer.h>
#include <cetech/editor/editor.h>
#include <cetech/resource/resource.h>

#include <cetech/gfx/private/iconfontheaders/icons_font_awesome.h>
#include <cetech/editor/asset_editor.h>

#define WINDOW_NAME "Explorer"

#define _G explorer_globals
static struct _G {
    bool visible;

    uint64_t selected_object;

    struct ce_alloc *allocator;
    uint64_t top_level_obj;
} _G;

static void ui_entity_item_end() {
    ct_debugui_a0->TreePop();
}

static void ui_entity_item_begin(uint64_t obj,
                                 uint32_t id) {

    ImGuiTreeNodeFlags flags = DebugUITreeNodeFlags_OpenOnArrow |
                               DebugUITreeNodeFlags_OpenOnDoubleClick;

    bool selected =_G.selected_object == obj;
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
    uint64_t uid = ce_cdb_a0->read_uint64(obj, ENTITY_UID, 0);
    const char *ent_name = ce_cdb_a0->read_str(obj, ENTITY_NAME, NULL);
    if (ent_name) {
        strcpy(name, ent_name);
    } else {
        snprintf(name, CE_ARRAY_LEN(name), "%llu", uid);
    }

    char label[128] = {0};
    snprintf(label, CE_ARRAY_LEN(label),
             ICON_FA_CUBE " ""%s##%llu", name, uid);

    const bool open = ct_debugui_a0->TreeNodeEx(label, flags);
    if (ct_debugui_a0->IsItemClicked(0)) {
        _G.selected_object = obj;

        uint64_t event;
        event = ce_cdb_a0->create_object(ce_cdb_a0->db(), EXPLORER_OBJ_SELECTED);

        struct ct_cdb_obj_t*w = ce_cdb_a0->write_begin(event);
        ce_cdb_a0->set_ref(w, EXPLORER_OBJ_SELECTED, obj);
        ce_cdb_a0->write_commit(w);

        ce_ebus_a0->broadcast(EXPLORER_EBUS, event);
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
            if (!component_i->get_interface) {
                continue;
            }

            struct ct_editor_component_i0 *editor;
            editor = component_i->get_interface(EDITOR_COMPONENT);

            if (!editor) {
                continue;
            }

            ImGuiTreeNodeFlags c_flags = DebugUITreeNodeFlags_Leaf;

            bool c_selected;
            c_selected =_G.selected_object == component;

            if (c_selected) {
                c_flags |= DebugUITreeNodeFlags_Selected;
            }

            char c_label[128] = {0};
            snprintf(c_label, CE_ARRAY_LEN(c_label),
                     "%s##component_%d", editor->display_name(), ++id);

            ct_debugui_a0->TreeNodeEx(c_label, c_flags);
            if (ct_debugui_a0->IsItemClicked(0)) {
                _G.selected_object = component;
                uint64_t event;
                event = ce_cdb_a0->create_object(ce_cdb_a0->db(), EXPLORER_OBJ_SELECTED);

                struct ct_cdb_obj_t*w = ce_cdb_a0->write_begin(event);
                ce_cdb_a0->set_ref(w, EXPLORER_OBJ_SELECTED, component);
                ce_cdb_a0->write_commit(w);

                ce_ebus_a0->broadcast(EXPLORER_EBUS, event);
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
            ui_entity_item_begin(child, ++id);
        }
        ui_entity_item_end();
    }
}


static void on_debugui(struct ct_dock_i0 *dock) {
    if (!_G.top_level_obj) {
        return;
    }

    ct_debugui_a0->LabelText("Entity", "");

    uint64_t selected_object =_G.selected_object;
    if(selected_object) {
        uint64_t parent = ce_cdb_a0->parent(selected_object);

        if(ce_cdb_a0->type(parent) == ENTITY_COMPONENTS) {
            uint64_t comp_type = ce_cdb_a0->type(selected_object);

            if(ct_debugui_a0->Button("Remove", (float[2]){0.0f})) {
                ce_cdb_obj_o *w = ce_cdb_a0->write_begin(parent);
                ce_cdb_a0->remove_property(w, comp_type);
                ce_cdb_a0->write_commit(w);

                uint64_t ent = ce_cdb_a0->parent(parent);
                _G.selected_object = ent;
            }
        }
    }

    ui_entity_item_begin(_G.top_level_obj, rand());
}


static const char *dock_title() {
    return ICON_FA_TREE " " WINDOW_NAME;
}

static const char *name(struct ct_dock_i0* dock) {
    return "explorer";
}

static struct ct_dock_i0 ct_dock_i0 = {
        .id = 0,
        .visible = true,
        .name = name,
        .display_title = dock_title,
        .draw_ui = on_debugui,
};


static void _on_asset_selected(uint64_t event) {
    uint64_t type = ce_cdb_a0->read_uint64(event, ASSET_TYPE, 0);
    uint64_t name = ce_cdb_a0->read_uint64(event, ASSET_NAME, 0);

    struct ct_resource_id rid = {
            .name = name,
            .type = type,
    };

    if(type != ENTITY_RESOURCE_ID) {
        return;
    }

    _G.top_level_obj = ct_resource_a0->get(rid);
}

static void _on_editor_asset_selected(uint64_t event) {
    uint64_t type = ce_cdb_a0->read_uint64(event, ASSET_TYPE, 0);
    uint64_t name = ce_cdb_a0->read_uint64(event, ASSET_NAME, 0);

    struct ct_resource_id rid = {
            .name = name,
            .type = type,
    };

    _G.top_level_obj = ct_resource_a0->get(rid);
}

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .visible = true
    };

    api->register_api(DOCK_INTERFACE_NAME, &ct_dock_i0);

    ce_ebus_a0->create_ebus(EXPLORER_EBUS);

    ce_ebus_a0->connect(ASSET_BROWSER_EBUS, ASSET_BROWSER_ASSET_SELECTED, _on_asset_selected, 0);
    ce_ebus_a0->connect(ASSET_EDITOR_EBUS, ASSET_EDITOR_ASSET_SELECTED, _on_editor_asset_selected, 0);
}

static void _shutdown() {
    _G = (struct _G) {0};
}

CE_MODULE_DEF(
        level_inspector,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_debugui_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ce_ebus_a0);
            CE_INIT_API(api, ct_resource_a0);
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