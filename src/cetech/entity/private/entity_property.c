#include <stdio.h>

#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/id.h>
#include <celib/memory/memory.h>
#include <celib/api.h>
#include <celib/yaml_cdb.h>
#include <celib/module.h>

#include <celib/containers/hash.h>
#include <celib/cdb.h>

#include <cetech/renderer/gfx.h>

#include <cetech/asset/asset.h>
#include <cetech/ecs/ecs.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/explorer/explorer.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/ui/icons_font_awesome.h>
#include <cetech/controlers/controlers.h>
#include <cetech/controlers/keyboard.h>
#include <fnmatch.h>
#include <cetech/ui/ui.h>


#define _G entity_property_global

static struct _G {
    ce_hash_t components;

    ce_alloc_t0 *allocator;
    uint64_t obj;

    uint64_t input_component_id;
} _G;

static struct ct_ecs_component_i0 *get_component_interface(uint64_t cdb_type) {
    ce_api_entry_t0 it = ce_api_a0->first(CT_ECS_COMPONENT_I0);
    while (it.api) {
        struct ct_ecs_component_i0 *i = (it.api);

        if (cdb_type == i->cdb_type) {
            return i;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
};

static void draw_component(ce_cdb_t0 db,
                           uint64_t obj,
                           uint64_t context) {
    uint64_t type = ce_cdb_a0->obj_type(db, obj);

    ct_ecs_component_i0 *c = get_component_interface(type);


    if (!c || !c->display_name || c->is_system_state) {
        return;
    }


    char buffer[128] = {};
    snprintf(buffer, CE_ARRAY_LEN(buffer), "%s", c->display_name);

//    float w = ct_ui_a0->get_content_region_avail().x;
    bool open = ct_property_editor_a0->ui_header_begin(buffer, obj);

    if (open) {
        ct_ui_a0->push_id(obj);
        if (ct_ui_a0->button(&(ct_ui_button_t0) {.text=ICON_FA_MINUS})) {
            ce_cdb_a0->destroy_object(db, obj);
        }
        ct_ui_a0->pop_id();

        ct_ui_a0->separator();

        ct_property_editor_a0->ui_body_begin(obj);
        ct_property_editor_a0->draw_object(db, obj, context);
        ct_property_editor_a0->ui_body_end(obj);
    }
    ct_property_editor_a0->ui_header_end(open, obj);
}

static void _entity_ui(uint64_t obj) {
    bool open = ct_property_editor_a0->ui_header_begin(ICON_FA_CUBE" Entity", obj);

    if (open) {
        ct_property_editor_a0->ui_body_begin(obj);

        ct_property_editor_a0->ui_label("UID", 0, NULL, 0);
        ct_property_editor_a0->ui_value_begin(0, NULL, 0);
        char buffer[128] = {};
        snprintf(buffer, CE_ARRAY_LEN(buffer), "0x%llx", obj);
        ct_ui_a0->text(buffer);
        ct_property_editor_a0->ui_value_end();

        ct_property_editor_a0->ui_str(obj, "Name", ENTITY_NAME, 11111111);

        ct_property_editor_a0->ui_body_end(obj);
    }
    ct_property_editor_a0->ui_header_end(open, obj);
}


static char modal_buffer[128] = {};

static bool _component_exist(ce_cdb_t0 db,
                             uint64_t obj,
                             uint64_t component_type) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(db, obj);
    uint64_t n = ce_cdb_a0->read_objset_num(reader, ENTITY_COMPONENTS);
    uint64_t keys[n];
    ce_cdb_a0->read_objset(reader, ENTITY_COMPONENTS, keys);

    for (int i = 0; i < n; ++i) {
        uint64_t type = ce_cdb_a0->obj_type(db, keys[i]);
        if (type == component_type) {
            return true;
        }
    }

    return false;
}


static void _add_comp_popup(ce_cdb_t0 db,
                            uint64_t modal_id,
                            uint64_t obj) {
    if (ct_ui_a0->popup_begin(&(ct_ui_popup_t0){.id=modal_id})) {
        struct ct_controler_i0 *kb = ct_controlers_a0->get(CONTROLER_KEYBOARD);

        if (kb->button_pressed(0, kb->button_index("escape"))) {
            ct_ui_a0->popup_close_current();
            ct_ui_a0->popup_end();
            return;
        }
        char labelidi[128] = {'\0'};
        sprintf(labelidi, "##modal_comp_input%llu", obj);

        ct_ui_a0->input_text(&(ct_ui_input_text_t0) {.id=_G.input_component_id},
                             modal_buffer,
                             CE_ARRAY_LEN(modal_buffer));

        struct ce_api_entry_t0 it = ce_api_a0->first(CT_ECS_COMPONENT_I0);
        while (it.api) {
            struct ct_ecs_component_i0 *i = (it.api);

            if (!i->display_name || i->is_system_state) {
                goto next;
            }

            uint64_t component_type = i->cdb_type;
            if (i->display_name && !_component_exist(db, obj, component_type)) {
                const char *label = i->display_name;

                if (modal_buffer[0]) {
                    char filter[256] = {};
                    snprintf(filter, CE_ARRAY_LEN(filter), "*%s*", modal_buffer);

                    if (0 != fnmatch(filter, label, FNM_CASEFOLD)) {
                        goto next;
                    }
                }

                bool add = ct_ui_a0->selectable(&(ct_ui_selectable_t0){.text=label});

                if (add) {
                    uint64_t component = ce_cdb_a0->create_object(db, component_type);

                    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(db, obj);
                    ce_cdb_a0->objset_add_obj(w, ENTITY_COMPONENTS, component);
                    ce_cdb_a0->write_commit(w);
                    modal_buffer[0] = '\0';
                }
            }
            next:
            it = ce_api_a0->next(it);
        }

        ct_ui_a0->popup_end();
    }
}


void draw_menu(uint64_t obj) {
    if (!obj) {
        return;
    }


    bool add = ct_ui_a0->button(&(ct_ui_button_t0) {.text=ICON_FA_PLUS" "ICON_FA_FOLDER_OPEN});

    _add_comp_popup(ce_cdb_a0->db(), obj, obj);

    if (add) {
        ct_ui_a0->popup_open(obj);
    }

}

static void draw_ui(ce_cdb_t0 db,
                    uint64_t obj,
                    uint64_t context) {
    if (!obj) {
        return;
    }

    draw_menu(obj);

    _entity_ui(obj);

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(db, obj);

    uint64_t n = ce_cdb_a0->read_objset_num(reader, ENTITY_COMPONENTS);
    uint64_t keys[n];
    ce_cdb_a0->read_objset(reader, ENTITY_COMPONENTS, keys);

    for (int i = 0; i < n; ++i) {
        uint64_t component = keys[i];
        draw_component(db, component, context);
    }
}


void CE_MODULE_LOAD(entity_property)(struct ce_api_a0 *api,
                                     int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_asset_a0);
    CE_INIT_API(api, ce_yaml_cdb_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ce_cdb_a0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .input_component_id = ct_ui_a0->generate_id(),
    };

    ce_cdb_a0->set_aspect(ENTITY_TYPE, CT_PROPERTY_EDITOR_ASPECT, draw_ui);
}

void CE_MODULE_UNLOAD(entity_property)(struct ce_api_a0 *api,
                                       int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
    _G = (struct _G) {};
}
