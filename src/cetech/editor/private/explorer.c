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
#include <cetech/editor/selected_object.h>

#include <cetech/gfx/private/iconfontheaders/icons_font_awesome.h>

#define WINDOW_NAME "Explorer"

#define _G explorer_globals
static struct _G {
    bool visible;

    uint64_t ent_name;

    const char *path;
    struct ce_alloc *allocator;
} _G;


void set_level(uint64_t obj) {
    uint64_t asset_type = ce_cdb_a0->read_uint64(obj, ASSET_BROWSER_ASSET_TYPE2, 0);
    uint64_t asset_name = ce_cdb_a0->read_uint64(obj, ASSET_BROWSER_ASSET_NAME, 0);
    const char *path = ce_cdb_a0->read_str(obj, ASSET_BROWSER_PATH, 0);

    struct ct_resource_id rid = {.name = asset_name, .type = asset_type};

    if (_G.ent_name == rid.name) {
        return;
    }

    if(rid.type != ENTITY_RESOURCE_ID) {
        _G.ent_name = 0;
        _G.path = NULL;

        return;
    }

    _G.ent_name = rid.name;
    _G.path = path;
}

static void ui_entity_item_end() {
    ct_debugui_a0->TreePop();
}

static void ui_entity_item_begin(uint64_t obj,
                                 uint32_t id) {

    ImGuiTreeNodeFlags flags = DebugUITreeNodeFlags_OpenOnArrow |
                               DebugUITreeNodeFlags_OpenOnDoubleClick;

    bool selected = ct_selected_object_a0->selected_object() == obj;
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

    bool open = ct_debugui_a0->TreeNodeEx(label, flags);
    if (ct_debugui_a0->IsItemClicked(0)) {
        ct_selected_object_a0->set_selected_object(obj);
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
            c_selected = ct_selected_object_a0->selected_object() == component;

            if (c_selected) {
                c_flags |= DebugUITreeNodeFlags_Selected;
            }

            char c_label[128] = {0};
            snprintf(c_label, CE_ARRAY_LEN(c_label),
                     "%s##component_%d", editor->display_name(), ++id);

            ct_debugui_a0->TreeNodeEx(c_label, c_flags);
            if (ct_debugui_a0->IsItemClicked(0)) {
                ct_selected_object_a0->set_selected_object(component);
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
    uint64_t selected_object = ct_selected_object_a0->selected_object();
    if (selected_object &&
        (ce_cdb_a0->type(selected_object) == ASSET_BROWSER_ASSET_TYPE)) {
        set_level(selected_object);
    }

    ct_debugui_a0->LabelText("Entity", "%llu", _G.ent_name);

    if (_G.path) {
        struct ct_resource_id rid = (struct ct_resource_id) {
                .type = ENTITY_RESOURCE_ID,
                .name = _G.ent_name,
        };

        uint64_t obj = ct_resource_a0->get(rid);

        if(obj) {
            ui_entity_item_begin(obj, rand());
        }
    }
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

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .visible = true
    };

    api->register_api(DOCK_INTERFACE_NAME, &ct_dock_i0);
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