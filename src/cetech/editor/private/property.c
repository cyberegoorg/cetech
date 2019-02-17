#include <string.h>


#include <celib/macros.h>
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include "celib/module.h"

#include <cetech/resource/resource.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/editor/property.h>
#include <cetech/editor/editor.h>
#include <cetech/editor/dock.h>
#include <cetech/editor/resource_browser.h>
#include <cetech/editor/explorer.h>
#include <celib/cdb.h>
#include <cetech/editor/selcted_object.h>
#include <stdio.h>
#include <cetech/editor/editor_ui.h>

#define WINDOW_NAME "Property editor"

#define _G property_inspector_global
static struct _G {
    bool visible;
} _G;

static void draw(uint64_t obj,
                 uint64_t context) ;

static void _generic_prop_draw(uint64_t obj,
                               uint64_t context) {
    uint64_t type = ce_cdb_a0->obj_type(ce_cdb_a0->db(), obj);

    const ce_cdb_type_def_t0 *defs = ce_cdb_a0->obj_type_def(type);

    if (!defs) {
        return;
    }

    for (uint32_t i = 0; i < defs->num; ++i) {
        const ce_cdb_prop_def_t0 *def = &defs->defs[i];
        enum ce_cdb_type_e0 type = def->type;
        uint64_t prop_name = ce_id_a0->id64(def->name);

        switch (type) {
            case CDB_TYPE_NONE:
                break;
            case CDB_TYPE_UINT64:
                break;
            case CDB_TYPE_PTR:
                break;
            case CDB_TYPE_REF:
                ct_editor_ui_a0->prop_resource(obj, prop_name, def->name,
                                               def->obj_type, context, obj);
                break;

            case CDB_TYPE_FLOAT:
                ct_editor_ui_a0->prop_float(obj, prop_name, def->name, (ui_float_p0) {});
                break;
            case CDB_TYPE_BOOL:
                ct_editor_ui_a0->prop_bool(obj, prop_name, def->name);
                break;
            case CDB_TYPE_STR:
                ct_editor_ui_a0->prop_str(obj, prop_name, def->name, obj);
                break;
            case CDB_TYPE_SUBOBJECT:{
                const ce_cdb_obj_o0* r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
                uint64_t subobj = ce_cdb_a0->read_subobject(r, prop_name, 0);
                draw(subobj, context);
            }
                break;

            case CDB_TYPE_BLOB:
                break;
        }
    }
}

static void draw(uint64_t obj,
                 uint64_t context) {
    if (!obj) {
        return;
    }

    ce_api_entry_t0 it = ce_api_a0->first(PROPERTY_EDITOR_INTERFACE);

    while (it.api) {
        struct ct_property_editor_i0 *i = (it.api);

        if (i && i->cdb_type
            && (i->cdb_type() == ce_cdb_a0->obj_type(ce_cdb_a0->db(), obj))) {

            if (i->draw_ui) {
                i->draw_ui(obj, context);
                return;
            }
            break;
        }

        it = ce_api_a0->next(it);
    }

    _generic_prop_draw(obj, context);
}

static struct ct_property_editor_i0 *get_interface(uint64_t obj) {
    if (!obj) {
        return NULL;
    }

    ce_api_entry_t0 it = ce_api_a0->first(PROPERTY_EDITOR_INTERFACE);

    while (it.api) {
        struct ct_property_editor_i0 *i = (it.api);

        if (i && i->cdb_type
            && (i->cdb_type() == ce_cdb_a0->obj_type(ce_cdb_a0->db(), obj))) {

            return i;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
}

static void draw_menu(uint64_t obj) {
    if (!obj) {
        return;
    }

    ce_api_entry_t0 it = ce_api_a0->first(PROPERTY_EDITOR_INTERFACE);

    while (it.api) {
        struct ct_property_editor_i0 *i = (it.api);

        if (i && i->cdb_type
            && (i->cdb_type() == ce_cdb_a0->obj_type(ce_cdb_a0->db(), obj))) {

            if (i->draw_menu) {
                i->draw_menu(obj);
            }
            return;
        }

        it = ce_api_a0->next(it);
    }
}

static void on_debugui(uint64_t dock) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), dock);

    const uint64_t context = ce_cdb_a0->read_uint64(reader, PROP_DOCK_CONTEXT,
                                                    0);

    uint64_t obj = ct_selected_object_a0->selected_object(context);
    uint64_t locked_object = ce_cdb_a0->read_ref(reader, CT_LOCKED_OBJ, 0);
    if (locked_object) {
        obj = locked_object;
    }

    char buffer[256];

    snprintf(buffer, CE_ARRAY_LEN(buffer), "property%llx", dock);


    ct_editor_ui_a0->ui_prop_header(ICON_FA_FILE" Resource");
    ct_debugui_a0->Separator();

    if (obj) {
        const ce_cdb_obj_o0 *reader_obj = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                          obj);

        const char *name = ce_cdb_a0->read_str(reader_obj,
                                               ASSET_NAME_PROP,
                                               NULL);
        if (name) {
            snprintf(buffer, CE_ARRAY_LEN(buffer), "%s", name);
            ct_debugui_a0->Text("Name");

            ct_debugui_a0->SameLine(0, 2);

            ct_debugui_a0->PushItemWidth(-1);
            ct_debugui_a0->InputText("##NameResourceProp",
                                     buffer,
                                     strlen(buffer),
                                     DebugInputTextFlags_ReadOnly,
                                     0, NULL);
            ct_debugui_a0->PopItemWidth();
        }

        uint64_t instance_of = ce_cdb_a0->read_instance_of(reader_obj);
        if (instance_of) {
            const ce_cdb_obj_o0 *inst_r = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                          instance_of);

            const char *name = ce_cdb_a0->read_str(inst_r,
                                                   ASSET_NAME_PROP,
                                                   NULL);

            if (name) {
                snprintf(buffer, CE_ARRAY_LEN(buffer), "%s", name);
            } else {
                snprintf(buffer, CE_ARRAY_LEN(buffer), "0x%llx",
                         instance_of);
            }

            ct_debugui_a0->Text("Inst. of");

            ct_debugui_a0->SameLine(0, 2);

            ct_debugui_a0->PushItemWidth(-1);
            ct_debugui_a0->InputText("##InstanceOfResourceProp",
                                     buffer,
                                     strlen(buffer),
                                     DebugInputTextFlags_ReadOnly,
                                     0, NULL);
            ct_debugui_a0->PopItemWidth();
        }
    }

    draw(obj, context);
}

static void on_menu(uint64_t dock) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), dock);
    const uint64_t context = ce_cdb_a0->read_uint64(reader, PROP_DOCK_CONTEXT,
                                                    0);

    ct_dock_a0->context_btn(dock);
    ct_debugui_a0->SameLine(0, -1);

    uint64_t obj = ct_selected_object_a0->selected_object(context);

    uint64_t locked_object = ct_editor_ui_a0->lock_selected_obj(dock, obj);
    if (locked_object) {
        obj = locked_object;
    }

    draw_menu(obj);

}


static const char *dock_title() {
    return ICON_FA_TABLE " " WINDOW_NAME;
}

static const char *name(uint64_t dock) {
    return "property_editor";
}

static uint64_t cdb_type() {
    return PROPERTY_EDITOR_INTERFACE;
};


static struct ct_dock_i0 dock_api = {
        .cdb_type = cdb_type,
        .name = name,
        .display_title = dock_title,
        .draw_ui = on_debugui,
        .draw_menu = on_menu,
};


struct ct_property_editor_a0 property_editor_api = {
        .draw =draw,
        .get_interface = get_interface,
};

struct ct_property_editor_a0 *ct_property_editor_a0 = &property_editor_api;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .visible = true
    };

    api->register_api(DOCK_INTERFACE, &dock_api, sizeof(dock_api));
    api->register_api(CT_PROP_EDITOR_API, ct_property_editor_a0, sizeof(ct_property_editor_a0));

    ct_dock_a0->create_dock(PROPERTY_EDITOR_INTERFACE, true);
}

static void _shutdown() {
    _G = (struct _G) {};
}

void CE_MODULE_LOAD(property_inspector)(struct ce_api_a0 *api,
                                        int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_id_a0);
    _init(api);
}

void CE_MODULE_UNLOAD(property_inspector)(struct ce_api_a0 *api,
                                          int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
    _shutdown();
}
