#include <string.h>

#include <celib/macros.h>
#include "celib/id.h"
#include "celib/api.h"
#include "celib/module.h"
#include "celib/module.h"
#include "celib/memory/memory.h"
#include "celib/memory/allocator.h"
#include "celib/containers/buffer.h"

#include <cetech/asset/asset.h>
#include <cetech/renderer/gfx.h>

#include <cetech/ui/icons_font_awesome.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/editor/editor.h>
#include <cetech/editor/dock.h>
#include <cetech/asset_browser/asset_browser.h>
#include <cetech/explorer/explorer.h>
#include <celib/cdb.h>
#include <cetech/editor/selcted_object.h>
#include <stdio.h>
#include <cetech/editor/editor_ui.h>
#include <celib/log.h>
#include <celib/containers/buffer.h>
#include <celib/yaml_cdb.h>
#include <cetech/asset_io/asset_io.h>
#include <cetech/ui/ui.h>

#define WINDOW_NAME "Property editor"

#define _G property_inspector_global

CE_MODULE(ct_assetdb_a0);

static struct _G {
    bool visible;
} _G;

static uint64_t _combine_hash(uint64_t a,
                              uint64_t b) {
    return a ^ b;
}


typedef void (draw_aspect)(ce_cdb_t0 db,
                           uint64_t obj,
                           uint64_t context);

typedef void (draw_menu_aspect)(uint64_t obj);

static void draw_object(ce_cdb_t0 db,
                        uint64_t obj,
                        uint64_t context);

static void _draw_property(ce_cdb_t0 db,
                           uint64_t obj,
                           uint64_t property,
                           const ce_cdb_prop_def_t0 *def,
                           uint64_t context) {

    const ce_cdb_obj_o0 *o = ce_cdb_a0->read(db, obj);
    ce_cdb_type_e0 type = ce_cdb_a0->prop_type(o, property);
    switch (type) {
        case CE_CDB_TYPE_REF:
            ct_editor_ui_a0->prop_asset(obj, def->name, property,
                                        def->obj_type, context, obj);
            break;
        case CE_CDB_TYPE_FLOAT:
            ct_editor_ui_a0->prop_float(obj, def->name, property, (ui_float_p0) {});
            break;

        case CE_CDB_TYPE_UINT64:
            ct_editor_ui_a0->prop_uin64(obj, def->name, property, (ui_uint64_p0) {});
            break;

        case CE_CDB_TYPE_BOOL:
            ct_editor_ui_a0->prop_bool(obj, def->name, property);
            break;
        case CE_CDB_TYPE_STR:
            ct_editor_ui_a0->prop_str(obj, def->name, property, obj);
            break;
        case CE_CDB_TYPE_SUBOBJECT: {
            const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
            uint64_t subobj = ce_cdb_a0->read_subobject(r, property, 0);
            draw_object(db, subobj, context);
            break;
        }
        case CE_CDB_TYPE_SET_SUBOBJECT: {
            bool open = ct_ui_a0->tree_node_ex(&(ct_ui_tree_node_ex_t0) {
                    .id=_combine_hash(obj, property),
                    .text=def->name,
                    .flags = CT_TREE_NODE_FLAGS_DefaultOpen});

            if (open) {
                const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
                uint64_t n = ce_cdb_a0->read_objset_num(r, property);
                uint64_t k[n];
                ce_cdb_a0->read_objset(r, property, k);
                for (int j = 0; j < n; ++j) {
                    uint64_t subobj = k[j];
                    draw_object(db, subobj, context);
                }
                ct_ui_a0->tree_pop();
            }
        }
            break;

        default:
            break;
    }
}

static void _draw_object(ce_cdb_t0 db,
                         uint64_t obj,
                         uint64_t context) {
    uint64_t type = ce_cdb_a0->obj_type(db, obj);

    draw_aspect *aspect = ce_cdb_a0->get_aspect(type, CT_PROPERTY_EDITOR_ASPECT);
    if (aspect) {
        aspect(db, obj, context);
        return;
    }

    ce_cdb_prop_def_t0 *defs = NULL;
    uint32_t n = 0;
    ce_cdb_a0->obj_type_def(type, &defs, &n);

    if (!defs) {
        return;
    }

    for (uint32_t i = 0; i < n; ++i) {
        const ce_cdb_prop_def_t0 *def = &defs[i];
        uint64_t prop_name = ce_id_a0->id64(def->name);
        _draw_property(db, obj, prop_name, def, context);
    }
}

static void draw_object(ce_cdb_t0 db,
                        uint64_t obj,
                        uint64_t context) {
    if (!obj) {
        return;
    }

    _draw_object(db, obj, context);
}

static void on_debugui(uint64_t content,
                       uint64_t context,
                       uint64_t selected_object) {
    uint64_t obj = selected_object;

    char buffer[256];

    snprintf(buffer, CE_ARRAY_LEN(buffer), "property%llx", 1ULL);

    bool open = ct_editor_ui_a0->ui_prop_header(ICON_FA_FILE" Asset");
    if (open && obj) {
        if (ct_ui_a0->button(&(ct_ui_button_t0) {.text="DDDD"})) {
            char *buffer = NULL;
            ce_yaml_cdb_a0->dump_str(ce_cdb_a0->db(), &buffer, obj, 0);
            ce_log_a0->debug("DDD", "%s", buffer);
            ce_buffer_free(buffer, ce_memory_a0->system);
        }

        ct_editor_ui_a0->ui_prop_body(obj);
        const ce_cdb_obj_o0 *reader_obj = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

        const char *filename = ct_asset_a0->asset_filename(
                ce_cdb_a0->obj_uid(ce_cdb_a0->db(), obj));
        if (filename) {
            strcpy(buffer, filename);

            ct_editor_ui_a0->prop_label("File", 0, NULL, 0);
            ct_editor_ui_a0->prop_value_begin(0, NULL, 0);

            ct_ui_a0->push_item_width(-1);
            ct_ui_a0->text(buffer);
            ct_ui_a0->pop_item_width();
            ct_editor_ui_a0->prop_value_end();
        }

        uint64_t instance_of = ce_cdb_a0->read_instance_of(reader_obj);
        if (instance_of) {
            char name[128] = {0};

            const char *fn = ct_asset_a0->asset_filename(
                    ce_cdb_a0->obj_uid(ce_cdb_a0->db(), instance_of));
            strcpy(buffer, fn);

            ct_editor_ui_a0->prop_label("Inst. of", 0, NULL, 0);
            ct_editor_ui_a0->prop_value_begin(0, NULL, 0);

            sprintf(buffer, ICON_FA_ARROW_UP
                    "##%sprop_open_select_asset", name);

            if (ct_ui_a0->button(&(ct_ui_button_t0) {.text=buffer})) {
                ct_selected_object_a0->set_selected_object(context, instance_of);
            }
            ct_ui_a0->same_line(0, 8);

            if (name[0]) {
                snprintf(buffer, CE_ARRAY_LEN(buffer), "%s", name);
            } else {
                snprintf(buffer, CE_ARRAY_LEN(buffer), "0x%llx", instance_of);
            }

            ct_ui_a0->push_item_width(-1);
            ct_ui_a0->text(buffer);
            ct_ui_a0->pop_item_width();
            ct_editor_ui_a0->prop_value_end();
        }
        ct_editor_ui_a0->ui_prop_body_end();
    }
    ct_editor_ui_a0->ui_prop_header_end(open);

    draw_object(ce_cdb_a0->db(), obj, context);
}

static const char *dock_title() {
    return ICON_FA_TABLE " " WINDOW_NAME;
}

static const char *name(uint64_t dock) {
    return "property_editor";
}


static struct ct_dock_i0 dock_api = {
        .type = CT_PROPERTY_EDITOR_I0,
        .name = name,
        .display_title = dock_title,
        .draw_ui = on_debugui,
};


struct ct_property_editor_a0 property_editor_api = {
        .draw_object =draw_object,
};

struct ct_property_editor_a0 *ct_property_editor_a0 = &property_editor_api;

void CE_MODULE_LOAD(property_inspector)(struct ce_api_a0 *api,
                                        int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_id_a0);
    _G = (struct _G) {
            .visible = true
    };

    api->add_impl(CT_DOCK_I0_STR, &dock_api, sizeof(dock_api));
    api->add_api(CT_PROP_EDITOR_A0_STR, ct_property_editor_a0, sizeof(ct_property_editor_a0));

    ct_dock_a0->create_dock(CT_PROPERTY_EDITOR_I0, true);

}

void CE_MODULE_UNLOAD(property_inspector)(struct ce_api_a0 *api,
                                          int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _G = (struct _G) {};
}
