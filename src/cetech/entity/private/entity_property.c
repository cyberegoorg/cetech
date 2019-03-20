#include <float.h>
#include <stdio.h>

#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/id.h>
#include <celib/config.h>
#include <celib/memory/memory.h>
#include <celib/api.h>
#include <celib/ydb.h>
#include <celib/containers/array.h>
#include <celib/module.h>

#include <celib/math/math.h>
#include <celib/containers/hash.h>
#include <celib/ydb.h>
#include <celib/cdb.h>
#include <celib/log.h>
#include <celib/containers/buffer.h>

#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/resource/resource.h>
#include <cetech/ecs/ecs.h>
#include <cetech/editor/property.h>
#include <cetech/editor/resource_browser.h>
#include <cetech/editor/explorer.h>
#include <cetech/editor/editor_ui.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/controlers/controlers.h>
#include <cetech/controlers/keyboard.h>
#include <fnmatch.h>


#define _G entity_property_global

static struct _G {
    ce_hash_t components;

    ce_alloc_t0 *allocator;
    uint64_t obj;
} _G;

static struct ct_component_i0 *get_component_interface(uint64_t cdb_type) {
    ce_api_entry_t0 it = ce_api_a0->first(COMPONENT_INTERFACE);
    while (it.api) {
        struct ct_component_i0 *i = (it.api);

        if (cdb_type == i->cdb_type()) {
            return i;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
};

static void draw_component(uint64_t obj,
                           uint64_t context) {
    uint64_t type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), obj);

    ct_component_i0 *c = get_component_interface(type);


    if (!c || !c->get_interface) {
        return;
    }


    ct_editor_component_i0 *editor = c->get_interface(EDITOR_COMPONENT);

    char buffer[128] = {};
    snprintf(buffer, CE_ARRAY_LEN(buffer), "%s %llx", editor->display_name(), obj);
    ct_editor_ui_a0->ui_prop_header(buffer);

    ct_debugui_a0->SameLine(0, 8);

    ct_debugui_a0->PushIDI((void *) obj);
    if (ct_debugui_a0->Button(ICON_FA_MINUS, &(ce_vec2_t) {})) {
        ce_cdb_a0->destroy_object(ce_cdb_a0->db(), obj);
    }

    ct_debugui_a0->PopID();

    ct_debugui_a0->Separator();

    ct_property_editor_a0->draw(obj, context);
}

static void _entity_ui(uint64_t obj) {
    ct_editor_ui_a0->ui_prop_header(ICON_FA_CUBE" Entity");
    ct_debugui_a0->Separator();

    ct_debugui_a0->Text("UID");

    ct_debugui_a0->Indent(0);
    ct_debugui_a0->PushItemWidth(-1);
    char buffer[128] = {};
    snprintf(buffer, CE_ARRAY_LEN(buffer), "0x%llx", obj);
    ct_debugui_a0->InputText("##EntityUID",
                             buffer,
                             strlen(buffer),
                             DebugInputTextFlags_ReadOnly,
                             0, NULL);
    ct_debugui_a0->PopItemWidth();
    ct_debugui_a0->Unindent(0);

    ct_editor_ui_a0->prop_str(obj, ENTITY_NAME, "Name", 11111111);
}

static void draw_ui(uint64_t obj,
                    uint64_t context) {
    if (!obj) {
        return;
    }

    _entity_ui(obj);

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    uint64_t n = ce_cdb_a0->read_objset_num(reader, ENTITY_COMPONENTS);
    uint64_t keys[n];
    ce_cdb_a0->read_objset(reader, ENTITY_COMPONENTS, keys);

    for (int i = 0; i < n; ++i) {
        uint64_t component = keys[i];
        draw_component(component, context);
    }
}


static char modal_buffer[128] = {};

static bool _component_exist(uint64_t obj,
                             uint64_t component_type) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    uint64_t n = ce_cdb_a0->read_objset_num(reader, ENTITY_COMPONENTS);
    uint64_t keys[n];
    ce_cdb_a0->read_objset(reader, ENTITY_COMPONENTS, keys);

    for (int i = 0; i < n; ++i) {
        uint64_t type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), keys[i]);
        if (type == component_type) {
            return true;
        }
    }

    return false;
}

static void _add_comp_modal(const char *modal_id,
                            uint64_t obj) {
    bool open = true;
    ct_debugui_a0->SetNextWindowSize(&(ce_vec2_t) {512, 512}, 0);
    if (ct_debugui_a0->BeginPopupModal(modal_id, &open, 0)) {
        struct ct_controlers_i0 *kb = ct_controlers_a0->get(CONTROLER_KEYBOARD);

        if (kb->button_pressed(0, kb->button_index("escape"))) {
            ct_debugui_a0->CloseCurrentPopup();
            ct_debugui_a0->EndPopup();
            return;
        }
        char labelidi[128] = {'\0'};
        sprintf(labelidi, "##modal_comp_input%llu", obj);

        ct_debugui_a0->InputText(labelidi,
                                 modal_buffer,
                                 CE_ARRAY_LEN(modal_buffer),
                                 0,
                                 0, NULL);


        struct ce_api_entry_t0 it = ce_api_a0->first(COMPONENT_I);
        while (it.api) {
            struct ct_component_i0 *i = (it.api);
            struct ct_editor_component_i0 *ei;

            if (!i->get_interface) {
                goto next;
            }

            ei = i->get_interface(EDITOR_COMPONENT);


            uint64_t component_type = i->cdb_type();
            if (ei->display_name && !_component_exist(obj, component_type)) {
                const char *label = ei->display_name();

                if (modal_buffer[0]) {
                    char filter[256] = {};
                    snprintf(filter, CE_ARRAY_LEN(filter), "*%s*", modal_buffer);

                    if (0 != fnmatch(filter, label, FNM_CASEFOLD)) {
                        goto next;
                    }
                }


                bool add = ct_debugui_a0->Selectable(label, false, 0,
                                                     &(ce_vec2_t) {});

                if (add) {
                    uint64_t component = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                                                  component_type);

                    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
                    ce_cdb_a0->objset_add_obj(w, ENTITY_COMPONENTS, component);
                    ce_cdb_a0->write_commit(w);
                    modal_buffer[0] = '\0';
                }
            }
            next:
            it = ce_api_a0->next(it);
        }

        ct_debugui_a0->EndPopup();
    }
}


void draw_menu(uint64_t obj) {
    if (!obj) {
        return;
    }

    ct_debugui_a0->SameLine(0.0f, -1);

    bool add = ct_debugui_a0->Button(ICON_FA_PLUS" "ICON_FA_FOLDER_OPEN,
                                     &(ce_vec2_t) {});


    char modal_id[128] = {'\0'};
    sprintf(modal_id, "select...##select_comp_%llu", obj);

    _add_comp_modal(modal_id, obj);

    if (add) {
        ct_debugui_a0->OpenPopup(modal_id);
    }

}


static uint64_t cdb_type() {
    return ENTITY_RESOURCE_ID;
}

static struct ct_property_editor_i0 property_editor_api = {
        .cdb_type = cdb_type,
        .draw_ui = draw_ui,
        .draw_menu = draw_menu,
};


void CE_MODULE_LOAD(entity_property)(struct ce_api_a0 *api,
                                     int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_debugui_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ce_cdb_a0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system
    };

    api->register_api(CT_PROPERTY_EDITOR_INTERFACE, &property_editor_api, sizeof(property_editor_api));

}

void CE_MODULE_UNLOAD(entity_property)(struct ce_api_a0 *api,
                                       int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
    _G = (struct _G) {};
}
