#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <corelib/hashlib.h>
#include <corelib/config.h>
#include <corelib/memory.h>
#include <corelib/api_system.h>
#include <corelib/module.h>
#include <corelib/cdb.h>
#include <corelib/yng.h>
#include <corelib/ydb.h>
#include <corelib/fs.h>
#include <corelib/ebus.h>
#include <corelib/macros.h>


#include <cetech/ecs/ecs.h>
#include <cetech/debugui/debugui.h>
#include <cetech/dock/dock.h>
#include <cetech/asset_browser/asset_browser.h>
#include <cetech/explorer/explorer.h>
#include <cetech/playground/playground.h>
#include <cetech/resource/resource.h>
#include <cetech/selected_object/selected_object.h>

#include <cetech/debugui/private/iconfontheaders/icons_font_awesome.h>

#define WINDOW_NAME "Explorer"
#define PLAYGROUND_MODULE_NAME CT_ID64_0("explorer")

#define _G explorer_globals
static struct _G {
    bool visible;

    uint32_t ent_name;

    const char *path;
    struct ct_alloc *allocator;
} _G;


void set_level(uint64_t obj) {
    uint64_t asset = ct_cdb_a0->read_uint64(obj, CT_ID64_0("asset"), 0);
    const char *path = ct_cdb_a0->read_str(obj, CT_ID64_0("path"), 0);

    struct ct_resource_id rid = {.i64 = asset};

    if (_G.ent_name == rid.name) {
        return;
    }

    if(rid.type != CT_ID32_0("entity")) {
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

    uint64_t children = ct_cdb_a0->read_subobject(obj, CT_ID64_0("children"),
                                                  0);

    const uint32_t children_n = ct_cdb_a0->prop_count(children);


    uint64_t components;
    components = ct_cdb_a0->read_subobject(obj, CT_ID64_0("components"), 0);

    const uint32_t component_n = ct_cdb_a0->prop_count(components);
    if (!children_n && !component_n) {
        flags |= DebugUITreeNodeFlags_Leaf;
    }

    char name[128] = {0};
    uint64_t uid = ct_cdb_a0->read_uint64(obj, CT_ID64_0("uid"), 0);
    const char *ent_name = ct_cdb_a0->read_str(obj, CT_ID64_0("name"), NULL);
    if (ent_name) {
        strcpy(name, ent_name);
    } else {
        snprintf(name, CT_ARRAY_LEN(name), "%llu", uid);
    }

    char label[128] = {0};
    snprintf(label, CT_ARRAY_LEN(label),
             ICON_FA_CUBE " ""%s##%llu", name, uid);

    bool open = ct_debugui_a0->TreeNodeEx(label, flags);
    if (ct_debugui_a0->IsItemClicked(0)) {
        ct_selected_object_a0->set_selected_object(obj);
    }

    if (open) {
        const uint32_t component_n = ct_cdb_a0->prop_count(components);
        uint64_t keys[component_n];
        ct_cdb_a0->prop_keys(components, keys);

        for (uint32_t i = 0; i < component_n; ++i) {
            uint64_t key = keys[i];

            uint64_t component = ct_cdb_a0->read_subobject(components, key, 0);
            uint64_t type = ct_cdb_a0->type(component);

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
            snprintf(c_label, CT_ARRAY_LEN(c_label),
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
        ct_cdb_a0->prop_keys(children, keys);

        for (uint32_t i = 0; i < children_n; ++i) {
            uint64_t key = keys[i];
            uint64_t child = ct_cdb_a0->read_subobject(children, key, 0);
            ui_entity_item_begin(child, ++id);
        }
        ui_entity_item_end();
    }
}


#define PROP_ENT_OBJ (CT_ID64_0("ent_obj"))

static void on_debugui(struct ct_dock_i0 *dock) {
    uint64_t selected_object = ct_selected_object_a0->selected_object();
    if (selected_object &&
        (ct_cdb_a0->type(selected_object) == CT_ID64_0("asset"))) {
        set_level(selected_object);
    }

    ct_debugui_a0->LabelText("Entity", "%u", _G.ent_name);

    if (_G.path) {
        struct ct_resource_id rid = (struct ct_resource_id) {
                .type = CT_ID32_0("entity"),
                .name = _G.ent_name,
        };

        uint64_t obj = ct_resource_a0->get(rid);

        if(obj) {
            obj = ct_cdb_a0->read_ref(obj, PROP_ENT_OBJ, 0);

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

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0->system,
            .visible = true
    };

    api->register_api("ct_dock_i0", &ct_dock_i0);
}

static void _shutdown() {
    _G = (struct _G) {0};
}

CETECH_MODULE_DEF(
        level_inspector,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_ebus_a0);
            CETECH_GET_API(api, ct_resource_a0);
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