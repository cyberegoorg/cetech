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

#include <cetech/ecs/ecs.h>
#include <cetech/debugui/debugui.h>
#include <cetech/playground/asset_browser.h>
#include <cetech/playground/explorer.h>
#include <cetech/playground/playground.h>
#include <cetech/resource/resource.h>
#include <corelib/ebus.h>
#include <corelib/macros.h>

#define WINDOW_NAME "Explorer"
#define PLAYGROUND_MODULE_NAME CT_ID64_0("explorer")

#define _G explorer_globals
static struct _G {
    bool visible;

    uint64_t selected_obj;
    uint32_t ent_name;
    struct ct_entity entity;
    struct ct_world world;

    const char *path;
    struct ct_alloc *allocator;
} _G;


void set_level(struct ct_world world,
               struct ct_entity level,
               uint64_t name,
               uint64_t root,
               const char *path) {

    CT_UNUSED(root);

    if (_G.ent_name == name) {
        return;
    }

    _G.ent_name = name;
    _G.entity = level;
    _G.world = world;
    _G.path = path;
}

static struct ct_explorer_a0 level_inspector_api = {
        .set_level = set_level,
};

struct ct_explorer_a0 *ct_explorer_a0 = &level_inspector_api;

static void ui_entity_item_end() {
    ct_debugui_a0->TreePop();
}

static struct ct_component_i0 *get_component_inteface(uint64_t cdb_type) {
    struct ct_api_entry it = ct_api_a0->first("ct_component_i0");
    while (it.api) {
        struct ct_component_i0 *i = (it.api);

        if (cdb_type == i->cdb_type()) {
            return i;
        }

        it = ct_api_a0->next(it);
    }

    return NULL;
};

static void ui_entity_item_begin(uint64_t obj,
                                 uint32_t id) {

    ImGuiTreeNodeFlags flags = DebugUITreeNodeFlags_OpenOnArrow |
                               DebugUITreeNodeFlags_OpenOnDoubleClick;

    bool selected = _G.selected_obj == obj;
    if (selected) {
        flags |= DebugUITreeNodeFlags_Selected;
    }

    uint64_t children = ct_cdb_a0->read_subobject(obj, CT_ID64_0("children"),
                                                  0);

    const uint32_t children_n = ct_cdb_a0->prop_count(children);

    if (!children_n) {
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
    snprintf(label, CT_ARRAY_LEN(label), "%s##%d", name, 0);

    bool open = ct_debugui_a0->TreeNodeEx(label, flags);
    if (ct_debugui_a0->IsItemClicked(0)) {
        uint64_t event = ct_cdb_a0->create_object(ct_cdb_a0->global_db(),
                                                  EXPLORER_ENTITY_SELECT_EVENT);

        ct_cdb_obj_o *w = ct_cdb_a0->write_begin(event);
        ct_cdb_a0->set_uint64(w, CT_ID64_0("world"), _G.world.h);
        ct_cdb_a0->set_str(w, CT_ID64_0("filename"), _G.path);
        ct_cdb_a0->set_uint64(w, CT_ID64_0("entity"), _G.entity.h);
        ct_cdb_a0->set_ref(w, CT_ID64_0("obj"), obj);
        ct_cdb_a0->write_commit(w);

        ct_ebus_a0->broadcast(EXPLORER_EBUS, event);

        _G.selected_obj = obj;
    }

    if (open) {
        uint64_t components;
        components = ct_cdb_a0->read_subobject(obj, CT_ID64_0("components"), 0);

        const uint32_t component_n = ct_cdb_a0->prop_count(components);
        uint64_t keys[component_n];
        ct_cdb_a0->prop_keys(components, keys);

        for (uint32_t i = 0; i < component_n; ++i) {
            uint64_t key = keys[i];


            uint64_t component = ct_cdb_a0->read_subobject(components, key, 0);
            uint64_t type = ct_cdb_a0->type(component);

            struct ct_component_i0 *c = get_component_inteface(type);
            if (!c->get_interface) {
                continue;
            }

            struct ct_editor_component_i0 *editor;
            editor = c->get_interface(EDITOR_COMPONENT);

            if (!editor) {
                continue;
            }

            ImGuiTreeNodeFlags flags = DebugUITreeNodeFlags_OpenOnArrow |
                                       DebugUITreeNodeFlags_OpenOnDoubleClick |
                                       DebugUITreeNodeFlags_Leaf;

            char label[128] = {0};
            snprintf(label, CT_ARRAY_LEN(label), "%s##%d",
                     editor->display_name(), rand());
            bool open = ct_debugui_a0->TreeNodeEx(label, flags);

            if (open) {
                ct_debugui_a0->TreePop();
            }
        }
    }

    if (open) {
        uint64_t keys[children_n];
        ct_cdb_a0->prop_keys(children, keys);

        for (uint32_t i = 0; i < children_n; ++i) {
            uint64_t key = keys[i];
            uint64_t child = ct_cdb_a0->read_subobject(children, key, 0);
            ui_entity_item_begin(child, rand());
        }
        ui_entity_item_end();
    }
}


#define PROP_ENT_OBJ (CT_ID64_0("ent_obj"))

static void on_debugui(uint64_t event) {
    if (ct_debugui_a0->BeginDock(WINDOW_NAME, &_G.visible, 0)) {

        ct_debugui_a0->LabelText("Entity", "%u", _G.ent_name);

        if (_G.path) {
            struct ct_resource_id rid = (struct ct_resource_id) {
                    .type = CT_ID32_0("entity"),
                    .name = _G.ent_name,
            };

            uint64_t obj = ct_resource_a0->get(rid);
            obj = ct_cdb_a0->read_ref(obj, PROP_ENT_OBJ, 0);

            ui_entity_item_begin(obj, rand());
        }
    }

    ct_debugui_a0->EndDock();
}

static void on_menu_window(uint64_t event) {
    CT_UNUSED(event);

    ct_debugui_a0->MenuItem2(WINDOW_NAME, NULL, &_G.visible, true);
}

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0->main_allocator(),
            .visible = true
    };

    api->register_api("ct_explorer_a0", &level_inspector_api);

    ct_ebus_a0->connect(PLAYGROUND_EBUS, PLAYGROUND_UI_EVENT, on_debugui, 0);
    ct_ebus_a0->connect(PLAYGROUND_EBUS, PLAYGROUND_UI_MAINMENU_EVENT,
                        on_menu_window, 0);

    ct_ebus_a0->create_ebus(EXPLORER_EBUS_NAME, EXPLORER_EBUS);
}

static void _shutdown() {
    ct_ebus_a0->disconnect(PLAYGROUND_EBUS, PLAYGROUND_UI_EVENT, on_debugui);
    ct_ebus_a0->disconnect(PLAYGROUND_EBUS, PLAYGROUND_UI_MAINMENU_EVENT,
                           on_menu_window);

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