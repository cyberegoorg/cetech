#include <stdio.h>
#include <stdlib.h>

#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/id.h>
#include <celib/memory/memory.h>
#include <celib/api.h>
#include <celib/cdb_yaml.h>
#include <celib/module.h>

#include <celib/cdb.h>


#include <cetech/asset/asset.h>
#include <cetech/ecs/ecs.h>
#include <cetech/editor/selcted_object.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/explorer/explorer.h>
#include <cetech/ui/icons_font_awesome.h>
#include <cetech/ui/ui.h>

static void ui_entity_item_end() {
    ct_ui_a0->tree_pop();
}

static void _spawn_to(uint64_t from,
                      uint64_t to,
                      uint32_t n) {
    uint64_t asset_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), from);
    uint64_t selecled_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), to);

    if ((ENTITY_TYPE == asset_type) &&
        (ENTITY_TYPE == selecled_type)) {

        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), to);
        for (int i = 0; i < n; ++i) {
            uint64_t new_obj = ce_cdb_a0->create_from(ce_cdb_a0->db(), from);
            ce_cdb_a0->objset_add_obj(w, ENTITY_CHILDREN, new_obj);
        }
        ce_cdb_a0->write_commit(w);
    }
}

static void _add(uint64_t selected_obj) {
    uint64_t entity_obj;
    entity_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), ENTITY_TYPE);
    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), selected_obj);
    ce_cdb_a0->objset_add_obj(w, ENTITY_CHILDREN, entity_obj);
    ce_cdb_a0->write_commit(w);
}


void item_btns(uint64_t context,
               uint64_t obj) {
    char label[128] = {0};
    snprintf(label, CE_ARRAY_LEN(label), ICON_FA_PLUS
            "##add_%llu", obj);

    bool add = ct_ui_a0->button(&(ct_ui_button_t0) {.text=label});

    if (add) {
        _add(obj);
    }

    ct_ui_a0->same_line(0, 4);
    snprintf(label, CE_ARRAY_LEN(label), ICON_FA_PLUS" " ICON_FA_FOLDER_OPEN"##add_from%llu", obj);


    bool add_from = ct_ui_a0->button(&(ct_ui_button_t0) {.text=label});

    uint64_t new_value = 0;

    bool changed = ct_property_editor_a0->ui_asset_select_modal(obj,
                                                                obj,
                                                                ENTITY_TYPE,
                                                                &new_value);
    if (add_from) {
        ct_ui_a0->popup_open(obj);
    }

    if (changed && new_value) {
        _spawn_to(new_value, obj, 1);
    }

    uint64_t parent = ce_cdb_a0->parent(ce_cdb_a0->db(), obj);

    if (parent) {
        ct_ui_a0->same_line(0, 4);
        snprintf(label, CE_ARRAY_LEN(label), ICON_FA_MINUS
                "##minus_%llu", obj);
        if (ct_ui_a0->button(&(ct_ui_button_t0) {.text=label})) {
            ce_cdb_a0->destroy_object(ce_cdb_a0->db(), obj);
            ct_selected_object_a0->set_selected_object(context, parent);
        }
    }

}

static uint64_t ui_entity_item_begin(uint64_t selected_obj,
                                     uint64_t obj,
                                     uint32_t id,
                                     uint64_t context) {

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    enum ct_ui_tree_node_flag flags =
            CT_TREE_NODE_FLAGS_OpenOnArrow |
            //                               DebugUITreeNodeFlags_OpenOnDoubleClick |
            //                               DebugUITreeNodeFlags_DefaultOpen;
            0;
    uint64_t new_selected_object = 0;

    bool selected = selected_obj == obj;

    if (selected) {
        flags |= CT_TREE_NODE_FLAGS_Selected;
    }


    uint64_t children_n = ce_cdb_a0->read_objset_num(reader, ENTITY_CHILDREN);
    uint64_t component_n = ce_cdb_a0->read_objset_num(reader, ENTITY_COMPONENTS);

    if (!children_n && !component_n) {
        flags |= CT_TREE_NODE_FLAGS_Leaf;
    }

    char name[128] = {0};
    uint64_t uuid = obj;
    const char *ent_name = ce_cdb_a0->read_str(reader, ENTITY_NAME, NULL);
    if (ent_name) {
        strcpy(name, ent_name);
    } else {
        snprintf(name, CE_ARRAY_LEN(name), "0x%llx", uuid);
    }

    char label[128] = {0};
    snprintf(label, CE_ARRAY_LEN(label), (ICON_FA_CUBE" %s"), name);


    const bool open = ct_ui_a0->tree_node_ex(&(ct_ui_tree_node_ex_t0) {.id=obj, .text=label});

    if (ct_ui_a0->is_item_clicked(0)) {
        new_selected_object = obj;
    }

    if (ct_ui_a0->drag_drop_source_begin(CT_UI_DROP_FLAGS_SourceAllowNullID)) {
        char text[128];
        snprintf(text, CE_ARRAY_LEN(text), ICON_FA_CUBE" %s", name);
        ct_ui_a0->text(text);

        ct_ui_a0->set_drag_drop_payload(&(ct_ui_drop_payload_t0) {
                .type="entity",
                .data= &obj,
                .size = sizeof(uint64_t),
                .cond = CT_UI_COND_Once,
        });

        ct_ui_a0->drag_drop_source_end();
    }

    if (ct_ui_a0->drag_drop_target_begin()) {
        const void *payload = ct_ui_a0->accept_drag_drop_payload("entity");

        if (payload) {
            uint64_t drag_obj = *((uint64_t *) payload);

            if (drag_obj && drag_obj != obj) {
                uint64_t asset_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), drag_obj);

                if (ENTITY_INSTANCE == asset_type) {
                    uint64_t parent = ce_cdb_a0->parent(ce_cdb_a0->db(), drag_obj);
                    ce_cdb_obj_o0 *pw = ce_cdb_a0->write_begin(ce_cdb_a0->db(), parent);
                    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);

                    ce_cdb_a0->move_objset_obj(pw, w, ENTITY_CHILDREN, drag_obj);

                    ce_cdb_a0->write_commit(w);
                    ce_cdb_a0->write_commit(pw);
                }
            }
        }

        ct_ui_a0->drag_drop_target_end();
    }

    if (ct_ui_a0->drag_drop_target_begin()) {
        const void *payload = ct_ui_a0->accept_drag_drop_payload("asset");

        if (payload) {
            uint64_t drag_obj = *((uint64_t *) payload);

            if (drag_obj) {
                _spawn_to(drag_obj, obj, 1);
            }
        }
        ct_ui_a0->drag_drop_target_end();
    }

    if (open) {
        uint64_t keys[component_n];
        ce_cdb_a0->read_objset(reader, ENTITY_COMPONENTS, keys);

        for (uint32_t i = 0; i < component_n; ++i) {
            uint64_t component = keys[i];
            uint64_t type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), component);

            struct ct_ecs_component_i0 *component_i;
            component_i = ct_ecs_c_a0->get_interface(type);
            if (!component_i || !component_i->display_name) {
                continue;
            }

            const char *component_display_name = component_i->display_name;
            const char *component_icon = component_i->icon ? component_i->icon
                                                           : ICON_FA_PUZZLE_PIECE;

            char item_text[128];
            snprintf(item_text, CE_ARRAY_LEN(item_text), "%s %s",
                     component_icon, component_display_name);


            enum ct_ui_tree_node_flag c_flags = CT_TREE_NODE_FLAGS_Leaf;

            bool c_selected;
            c_selected = selected_obj == component;

            if (c_selected) {
                c_flags |= CT_TREE_NODE_FLAGS_Selected;
            }

            if (ct_ui_a0->tree_node_ex(&(ct_ui_tree_node_ex_t0) {
                    .id=component,
                    .text=item_text,
                    .flags=c_flags})) {
                if (ct_ui_a0->is_item_clicked(0)) {
                    new_selected_object = component;
                }

                ct_ui_a0->tree_pop();
            }
        }
    }

    if (open) {
        uint64_t keys[children_n];
        ce_cdb_a0->read_objset(reader, ENTITY_CHILDREN, keys);

        for (uint32_t i = 0; i < children_n; ++i) {
            uint64_t key = keys[i];
            uint64_t new_selected_object2 = ui_entity_item_begin(selected_obj,
                                                                 key, ++id,
                                                                 context);
            if (new_selected_object2) {
                new_selected_object = new_selected_object2;
            }
        }
        ui_entity_item_end();
    }

    return new_selected_object;
}

static void draw_menu(uint64_t selected_obj,
                      uint64_t context) {
    if (!selected_obj) {
        return;
    }

    uint64_t type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), selected_obj);

    if (type == ENTITY_TYPE) {
        item_btns(context, selected_obj);
    }
}

static uint64_t draw_ui(uint64_t top_level_obj,
                        uint64_t selected_obj,
                        uint64_t context) {
    if (!top_level_obj) {
        return 0;
    }

    if (!selected_obj) {
        return 0;
    }

    draw_menu(selected_obj, context);

    uint64_t ret = ui_entity_item_begin(selected_obj, top_level_obj, rand(), context);

    return ret;
}

void CE_MODULE_LOAD(entity_explorer)(struct ce_api_a0 *api,
                                     int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_asset_a0);
    CE_INIT_API(api, ce_cdb_yaml_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ce_cdb_a0);

    ce_cdb_a0->set_aspect(ENTITY_TYPE, CT_EXPLORER_ASPECT, draw_ui);
}

void CE_MODULE_UNLOAD(entity_explorer)(struct ce_api_a0 *api,
                                       int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
