#include <stdio.h>
#include <stdlib.h>

#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/id.h>
#include <celib/memory/memory.h>
#include <celib/api.h>
#include <celib/ydb.h>
#include <celib/module.h>

#include <celib/cdb.h>

#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/resource/resource.h>
#include <cetech/ecs/ecs.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/explorer/explorer.h>

#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/editor/selcted_object.h>
#include <cetech/editor/editor_ui.h>

static void ui_entity_item_end() {
    ct_debugui_a0->TreePop();
}

static void _spawn_to(uint64_t from,
                      uint64_t to,
                      uint32_t n) {
    uint64_t asset_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), from);
    uint64_t selecled_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), to);

    if ((ENTITY_RESOURCE_ID == asset_type) &&
        (ENTITY_RESOURCE_ID == selecled_type)) {

        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), to);
        for (int i = 0; i < n; ++i) {
            uint64_t new_obj = ce_cdb_a0->create_from(ce_cdb_a0->db(), from);
            ce_cdb_a0->objset_add_obj(w, ENTITY_CHILDREN, new_obj);
        }
        ce_cdb_a0->write_commit(w);
    }
}

static void _add(uint64_t selected_obj) {
//    for (int i = 0; i < 10; ++i) {
    uint64_t entity_obj;
    entity_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), ENTITY_RESOURCE_ID);
    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), selected_obj);
    ce_cdb_a0->objset_add_obj(w, ENTITY_CHILDREN, entity_obj);
    ce_cdb_a0->write_commit(w);
//    }

}


void item_btns(uint64_t context,
               uint64_t uid) {
    char label[128] = {0};
    snprintf(label, CE_ARRAY_LEN(label), ICON_FA_PLUS
            "##add_%llu", uid);

    bool add = ct_debugui_a0->Button(label, &(ce_vec2_t) {0.0f});

    if (add) {
        _add(uid);
    }

    ct_debugui_a0->SameLine(0, 4);
    snprintf(label, CE_ARRAY_LEN(label),
             ICON_FA_PLUS
                     " "
                     ICON_FA_FOLDER_OPEN
                     "##add_from%llu", uid);


    bool add_from = ct_debugui_a0->Button(label, &(ce_vec2_t) {0.0f});

    char modal_id[128] = {'\0'};
    sprintf(modal_id, "select...##select_resource_%llu", uid);

    uint64_t new_value = 0;

    static uint32_t count = 1;
    bool changed = ct_editor_ui_a0->resource_select_modal(modal_id,
                                                          uid,
                                                          ENTITY_RESOURCE_ID,
                                                          &new_value,
                                                          &count);
    if (add_from) {
        ct_debugui_a0->OpenPopup(modal_id);
    }

    if (changed && new_value) {
        _spawn_to(new_value, uid, count);
    }

    uint64_t parent = ce_cdb_a0->parent(ce_cdb_a0->db(), uid);

    if (parent) {
        ct_debugui_a0->SameLine(0, 4);
        snprintf(label, CE_ARRAY_LEN(label), ICON_FA_MINUS
                "##minus_%llu", uid);
        if (ct_debugui_a0->Button(label, &(ce_vec2_t) {0.0f})) {
            ce_cdb_a0->destroy_object(ce_cdb_a0->db(), uid);
            ct_selected_object_a0->set_selected_object(context, parent);
        }
    }

}

static uint64_t ui_entity_item_begin(uint64_t selected_obj,
                                     uint64_t obj,
                                     uint32_t id,
                                     uint64_t context) {

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    ImGuiTreeNodeFlags flags = 0 |
                               DebugUITreeNodeFlags_OpenOnArrow |
                               //                               DebugUITreeNodeFlags_OpenOnDoubleClick |
                               //                               DebugUITreeNodeFlags_DefaultOpen;
                               0;
    uint64_t new_selected_object = 0;

    bool selected = selected_obj == obj;

    if (selected) {
        flags |= DebugUITreeNodeFlags_Selected;
    }


    uint64_t children_n = ce_cdb_a0->read_objset_num(reader, ENTITY_CHILDREN);
    uint64_t component_n = ce_cdb_a0->read_objset_num(reader, ENTITY_COMPONENTS);

    if (!children_n && !component_n) {
        flags |= DebugUITreeNodeFlags_Leaf;
    }

    char name[128] = {0};
    uint64_t uid = obj;
    const char *ent_name = ce_cdb_a0->read_str(reader, ENTITY_NAME, NULL);
    if (ent_name) {
        strcpy(name, ent_name);
    } else {
        snprintf(name, CE_ARRAY_LEN(name), "0x%llx", uid);
    }

    char label[128] = {0};
    snprintf(label, CE_ARRAY_LEN(label),
             (ICON_FA_CUBE
                     " ""%s##%llu"), name, uid);
    const bool open = ct_debugui_a0->TreeNodeEx(label, flags);
    if (ct_debugui_a0->IsItemClicked(0)) {
        new_selected_object = obj;
    }

    if (ct_debugui_a0->BeginDragDropSource(
            DebugUIDragDropFlags_SourceAllowNullID)) {

        ct_debugui_a0->Text(ICON_FA_CUBE" %s", name);

        ct_debugui_a0->SetDragDropPayload("entity",
                                          &obj,
                                          sizeof(uint64_t),
                                          DebugUICond_Once);
        ct_debugui_a0->EndDragDropSource();
    }

    if (ct_debugui_a0->BeginDragDropTarget()) {
        const struct DebugUIPayload *payload;
        payload = ct_debugui_a0->AcceptDragDropPayload("entity", 0);

        if (payload) {
            uint64_t drag_obj = *((uint64_t *) payload->Data);

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

        ct_debugui_a0->EndDragDropTarget();
    }

    if (ct_debugui_a0->BeginDragDropTarget()) {

        const struct DebugUIPayload *payload;
        payload = ct_debugui_a0->AcceptDragDropPayload("asset", 0);

        if (payload) {
            uint64_t drag_obj = *((uint64_t *) payload->Data);

            if (drag_obj) {
                _spawn_to(drag_obj, obj, 1);
            }
        }
        ct_debugui_a0->EndDragDropTarget();
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

            const char *component_display_name = component_i->display_name();

            ImGuiTreeNodeFlags c_flags = DebugUITreeNodeFlags_Leaf;

            bool c_selected;
            c_selected = selected_obj == component;

            if (c_selected) {
                c_flags |= DebugUITreeNodeFlags_Selected;
            }

            char c_label[128] = {0};
            snprintf(c_label, CE_ARRAY_LEN(c_label), "##component_%llu", component);

            //
            snprintf(c_label, CE_ARRAY_LEN(c_label), "%s##component_%llu",
                     component_display_name, component);

            if (ct_debugui_a0->TreeNodeEx(c_label, c_flags)) {
                if (ct_debugui_a0->IsItemClicked(0)) {
                    new_selected_object = component;
                }

                ct_debugui_a0->TreePop();
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

    if (type == ENTITY_RESOURCE_ID) {
        item_btns(context, selected_obj);
    }
}

static uint64_t cdb_type() {
    return ENTITY_RESOURCE_ID;
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

    uint64_t ret = ui_entity_item_begin(selected_obj, top_level_obj, rand(), context);

    return ret;
}

void CE_MODULE_LOAD(entity_explorer)(struct ce_api_a0 *api,
                                     int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_debugui_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ce_cdb_a0);

    static struct ct_explorer_i0 entity_explorer = {
            .cdb_type = cdb_type,
            .draw_ui = draw_ui,
            .draw_menu = draw_menu,
    };

    api->add_impl(CT_EXPLORER_I, &entity_explorer, sizeof(entity_explorer));
}

void CE_MODULE_UNLOAD(entity_explorer)(struct ce_api_a0 *api,
                                       int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
