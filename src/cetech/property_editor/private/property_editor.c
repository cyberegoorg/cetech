#include <float.h>

#include <celib/macros.h>
#include "celib/id.h"
#include "celib/api.h"
#include "celib/module.h"
#include "celib/memory/allocator.h"
#include <celib/cdb_yaml.h>
#include <celib/cdb.h>
#include <celib/config.h>

#include <cetech/asset/asset.h>
#include <cetech/ui/icons_font_awesome.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/editor/dock.h>
#include <cetech/explorer/explorer.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/ui/ui.h>
#include <cetech/asset_io/asset_io.h>
#include <cetech/controlers/controlers.h>
#include <cetech/controlers/keyboard.h>
#include <celib/memory/memory.h>
#include <celib/containers/array.h>
#include <cetech/asset_preview/asset_preview.h>
#include <cetech/editor/selcted_object.h>
#include <cetech/transform/transform.h>
#include <cetech/cdb_types/cdb_types.h>

#include "include/nfd/nfd.h"

#define WINDOW_NAME "Property editor"

#define _G property_inspector_global

CE_MODULE(ct_assetdb_a0);

static struct _G {
    bool visible;
} _G;

static uint64_t _combine_hash(uint64_t a,
                              uint64_t b) {
    return a ^ b * 11;
}


typedef void (draw_aspect)(ce_cdb_t0 db,
                           uint64_t obj,
                           const char *title,
                           uint64_t context);

typedef void (draw_menu_aspect)(uint64_t obj);


static bool ui_revert_btn(uint64_t _obj,
                          const uint64_t *props,
                          uint64_t props_n) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(), _obj);
    uint64_t instance_of = ce_cdb_a0->read_instance_of(r);

    bool need_revert = false;
    if (instance_of) {
        const ce_cdb_obj_o0 *ir = ce_cdb_a0->read(ce_cdb_a0->db(), instance_of);

        for (int i = 0; i < props_n; ++i) {
            if (!ce_cdb_a0->prop_equal(r, ir, props[i])) {
                need_revert = true;
                break;
            }
        }
    }

    char lbl[256] = {};
    snprintf(lbl, CE_ARRAY_LEN(lbl), "%s##revert_%llu_%llu",
             ICON_FA_RECYCLE, _obj, props[0]);

    if (!need_revert) {
        ct_ui_a0->push_item_flag(CT_UI_ITEM_FLAGS_Disabled, true);
        ct_ui_a0->push_style_var(CT_UI_STYLE_VAR_Alpha, 0.5f);
    }


    bool remove_change = ct_ui_a0->button(&(ct_ui_button_t0) {.text=lbl});

    if (!need_revert) {
        ct_ui_a0->pop_item_flag();
        ct_ui_a0->pop_style_var(1);
    }

    if (need_revert && ct_ui_a0->is_item_hovered(0)) {
        const ce_cdb_obj_o0 *ir = ce_cdb_a0->read(ce_cdb_a0->db(), instance_of);

        char v_str[256] = {};
        uint32_t offset = 0;

        for (int i = 0; i < props_n; ++i) {
            uint64_t prop = props[i];
            enum ce_cdb_type_e0 ptype = ce_cdb_a0->prop_type(ir, prop);

            switch (ptype) {
                case CE_CDB_TYPE_UINT64: {
                    uint64_t u = ce_cdb_a0->read_uint64(ir,
                                                        prop,
                                                        0);

                    offset += snprintf(v_str + offset,
                                       CE_ARRAY_LEN(v_str) - offset,
                                       "%llu ", u);
                }
                    break;

                case CE_CDB_TYPE_REF: {
                    uint64_t ref = ce_cdb_a0->read_ref(ir, prop, 0);
                    if (!ref) {
                        break;
                    }

                    const char *fn = ct_asset_a0->asset_filename(
                            ce_cdb_a0->obj_uid(ce_cdb_a0->db(), ref));
                    if (fn) {
                        offset += snprintf(v_str + offset,
                                           CE_ARRAY_LEN(v_str) - offset,
                                           "%s ", fn);
                    }
                }
                    break;

                case CE_CDB_TYPE_FLOAT: {
                    float f = ce_cdb_a0->read_float(ir,
                                                    prop,
                                                    0);

                    offset += snprintf(v_str + offset,
                                       CE_ARRAY_LEN(v_str) - offset,
                                       "%f ", f);
                }
                    break;

                case CE_CDB_TYPE_STR: {
                    const char *str = ce_cdb_a0->read_str(ir,
                                                          prop,
                                                          NULL);

                    offset += snprintf(v_str + offset,
                                       CE_ARRAY_LEN(v_str) - offset,
                                       "%s ", str);
                }


                    break;

                default:
                    break;
            }
        }

        if (v_str[0]) {
            ct_ui_a0->tooltip_begin();
            char text[128];
            snprintf(text, CE_ARRAY_LEN(text), "Parent value: %s", v_str);
            ct_ui_a0->text(text);
            ct_ui_a0->tooltip_end();
        }

    }

    if (need_revert && remove_change) {
        const ce_cdb_obj_o0 *ir = ce_cdb_a0->read(ce_cdb_a0->db(), instance_of);
        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), _obj);
        for (int i = 0; i < props_n; ++i) {
            ce_cdb_a0->prop_copy(ir, w, props[i]);
        }
        ce_cdb_a0->write_commit(w);
    }

    return true;
}


static void ui_label(const char *label,
                     uint64_t obj,
                     const uint64_t *props,
                     uint64_t props_n) {
    if (label) {
        ct_ui_a0->text(label);
    }

    ct_ui_a0->next_column();
}


static void ui_value_begin(uint64_t obj,
                           const uint64_t *props,
                           uint64_t props_n) {
    if (obj) {
        ui_revert_btn(obj, props, props_n);
        ct_ui_a0->same_line(0, 2);
    }
}

static void ui_value_end(uint64_t obj,
                         const uint64_t *props,
                         uint64_t props_n) {
    ct_ui_a0->next_column();
}

static void ui_float(uint64_t obj,
                     const char *label,
                     uint64_t prop,
                     struct ui_float_p0 params) {
    float value = 0;
    float value_new = 0;

    if (!obj) {
        return;
    }

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    value_new = ce_cdb_a0->read_float(reader, prop, value_new);
    value = value_new;

    const float min = !params.max_f ? -FLT_MAX : params.min_f;
    const float max = !params.max_f ? FLT_MAX : params.max_f;

    ui_label(label, obj, &prop, 1);

    ui_value_begin(obj, &prop, 1);

    if (ct_ui_a0->drag_float(&(ct_ui_drag_float_t0) {
            .id=_combine_hash(obj, prop),
            .v_min = min,
            .v_max = max,
    }, &value_new)) {
        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_float(w, prop, value_new);
        ce_cdb_a0->write_commit(w);
    }
    ui_value_end(obj, &prop, 1);
}

static void ui_uint64(uint64_t obj,
                      const char *label,
                      uint64_t prop,
                      struct ui_uint64_p0 params) {
    uint64_t value = 0;
    int value_new = 0;

    if (!obj) {
        return;
    }

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    value_new = ce_cdb_a0->read_uint64(reader, prop, value_new);
    value = value_new;

    const uint64_t min = !params.max_f ? -UINT64_MAX : params.min_f;
    const uint64_t max = !params.max_f ? UINT64_MAX : params.max_f;

    ui_label(label, obj, &prop, 1);

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_float_%d", label, 0);

    ui_value_begin(obj, &prop, 1);

    if (ct_ui_a0->drag_int(&(ct_ui_drag_int_t0) {
            .id=_combine_hash(obj, prop),
            .v_min = min,
            .v_max = max,
    }, &value_new)) {
        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_uint64(w, prop, value_new);
        ce_cdb_a0->write_commit(w);
    }
    ui_value_end(obj, &prop, 1);
}

static void ui_bool(uint64_t obj,
                    const char *label,
                    uint64_t prop) {
    bool value = false;
    bool value_new = false;

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    value_new = ce_cdb_a0->read_bool(reader, prop, value_new);
    value = value_new;

    ui_label(label, obj, &prop, 1);

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_float_%d", label, 0);

    ui_value_begin(obj, &prop, 1);

    if (ct_ui_a0->checkbox(&(ct_ui_checkbox_t0) {.text=labelid}, &value_new)) {
        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_bool(w, prop, value_new);
        ce_cdb_a0->write_commit(w);
    }

    ui_value_end(obj, &prop, 1);

}


static void ui_str(uint64_t obj,
                   const char *label,
                   uint64_t prop,
                   uint32_t i) {
    char labelid[128] = {'\0'};

    const char *value = 0;

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    value = ce_cdb_a0->read_str(reader, prop, "");

    char buffer[128] = {'\0'};

    if (value) {
        strcpy(buffer, value);
    }


    sprintf(labelid, "##%sprop_str_%d", label, i);

    ui_label(label, obj, &prop, 1);

    bool change = false;

    const uint64_t id = _combine_hash(obj, prop);

    ui_value_begin(obj, &prop, 1);
    ct_ui_a0->push_item_width(-1);
    change |= ct_ui_a0->input_text(&(ct_ui_input_text_t0) {.id=id},
                                   buffer,
                                   CE_ARRAY_LEN(buffer));
    ct_ui_a0->pop_item_width();
    ui_value_end(obj, &prop, 1);

    if (change) {
        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_str(w, prop, buffer);
        ce_cdb_a0->write_commit(w);
    }
}

static void ui_filename(uint64_t obj,
                        const char *label,
                        uint64_t prop,
                        const char *filter,
                        uint32_t i) {
    char labelid[128] = {'\0'};

    const char *value = 0;

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    value = ce_cdb_a0->read_str(reader, prop, "");

    char buffer[128] = {'\0'};

    if (value) {
        strcpy(buffer, value);
    }

    ui_label(label, obj, &prop, 1);

    ui_value_begin(obj, &prop, 1);

    // Open btn

    sprintf(labelid, ICON_FA_FOLDER_OPEN
            "##%sprop_select_filename_%d", label, i);

    const char *str = NULL;
    if (ct_ui_a0->button(&(ct_ui_button_t0) {.text=labelid})) {
        const char *source_dir = ce_config_a0->read_str(CONFIG_SRC, "");

        nfdchar_t *outPath = NULL;
        nfdresult_t result = NFD_OpenDialog(filter, source_dir, &outPath);

        if (result == NFD_OKAY) {
            uint32_t len = strlen(source_dir);
            str = strstr(outPath, source_dir + 2);
            if (str != NULL) {
                str = (str + (len - 1));
            }
        }
    }

    ct_ui_a0->same_line(0, 2);

    sprintf(labelid, "##%sprop_str_%d", label, i);

    ct_ui_a0->push_item_width(-1);
    ct_ui_a0->text(buffer);
    ct_ui_a0->pop_item_width();


    ui_value_end(obj, &prop, 1);

    if (str != NULL) {
        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_str(w, prop, str);
        ce_cdb_a0->write_commit(w);
    }
}

static void ui_str_combo(uint64_t obj,
                         const char *label,
                         uint64_t prop,
                         void (*combo_items)(uint64_t obj,
                                             char **items,
                                             uint32_t *items_count),
                         uint32_t i) {
    const char *value = 0;

    if (!obj) {
        return;
    }

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    value = ce_cdb_a0->read_str(reader, prop, NULL);

    char *items = NULL;
    uint32_t items_count = 0;

    combo_items(obj, &items, &items_count);

    int32_t current_item = -1;

    const char *items2[items_count];
    memset(items2, 0, sizeof(const char *) * items_count);

    for (int j = 0; j < items_count; ++j) {
        items2[j] = &items[j * 128];
        if (value) {
            if (ce_id_a0->id64(items2[j]) == ce_id_a0->id64(value)) {
                current_item = j;
            }
        }
    }

    ui_label(label, obj, &prop, 1);

    char labelid[128] = {'\0'};

    char buffer[128] = {'\0'};

    if (value) {
        strcpy(buffer, value);
    }

    sprintf(labelid, "##%scombo_%d", label, i);
    ui_value_begin(obj, &prop, 1);
    ct_ui_a0->push_item_width(-1);
    bool change = ct_ui_a0->combo(&(ct_ui_combo_t0) {
            .label=labelid,
            .items=items2,
            .items_count=items_count
    }, &current_item);

    ct_ui_a0->pop_item_width();
    ui_value_end(obj, &prop, 1);


    if (change) {
        strcpy(buffer, items2[current_item]);

        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_str(w, prop, buffer);
        ce_cdb_a0->write_commit(w);
    }

}

static void ui_str_combo2(uint64_t obj,
                          const char *label,
                          uint64_t prop,
                          const char *const *items,
                          uint32_t items_count,
                          uint32_t i) {
    const char *value = 0;

    if (!obj) {
        return;
    }

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    value = ce_cdb_a0->read_str(reader, prop, NULL);

    int32_t current_item = -1;

    for (int j = 0; j < items_count; ++j) {
        if (value) {
            if (ce_id_a0->id64(items[j]) == ce_id_a0->id64(value)) {
                current_item = j;
            }
        }
    }

    ui_label(label, obj, &prop, 1);

    char labelid[128] = {'\0'};

    char buffer[128] = {'\0'};

    if (value) {
        strcpy(buffer, value);
    }

    sprintf(labelid, "##%scombo_%d", label, i);
    ui_value_begin(obj, &prop, 1);
    ct_ui_a0->push_item_width(-1);
    bool change = ct_ui_a0->combo(&(ct_ui_combo_t0) {
            .label=labelid,
            .items=items,
            .items_count=items_count
    }, &current_item);

    ct_ui_a0->pop_item_width();
    ui_value_end(obj, &prop, 1);


    if (change) {
        strcpy(buffer, items[current_item]);

        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_str(w, prop, buffer);
        ce_cdb_a0->write_commit(w);
    }

}

static char modal_buffer[128] = {};

static bool ui_asset_select_modal(uint64_t modal_id,
                                  uint64_t id,
                                  uint64_t asset_type,
                                  uint64_t *selected_asset) {
    bool changed = false;


//    ct_debugui_a0->SetNextWindowSize(&(ce_vec2_t) {512, 512}, 0);
    if (ct_ui_a0->popup_begin(&(ct_ui_popup_t0) {.id=modal_id})) {
        struct ct_controler_i0 *kb = ct_controlers_a0->get(CONTROLER_KEYBOARD);

        if (kb->button_pressed(0, kb->button_index("escape"))) {
            ct_ui_a0->popup_close_current();
            ct_ui_a0->popup_end();
            return false;
        }

        char labelidi[128] = {'\0'};
        sprintf(labelidi, "##modal_input%llu", id);

        ct_ui_a0->input_text(&(ct_ui_input_text_t0) {.id=id},
                             modal_buffer,
                             CE_ARRAY_LEN(modal_buffer));

        const char *asset_type_s = ce_id_a0->str_from_id64(asset_type);
        char **assets = NULL;

        ct_asset_a0->list_assets_by_type(modal_buffer, asset_type_s,
                                         &assets,
                                         ce_memory_a0->system);

        uint32_t dir_n = ce_array_size(assets);
        for (int i = 0; i < dir_n; ++i) {
            const char *name = assets[i];

            if (!strlen(name)) {
                continue;
            }

            bool selected = ct_ui_a0->selectable(&(ct_ui_selectable_t0) {.text=name});

            struct ce_cdb_uuid_t0 r = ct_asset_a0->filename_asset(name);

            if (ct_ui_a0->is_item_hovered(0)) {

                ct_ui_a0->tooltip_begin();
                ct_asset_preview_a0->asset_tooltip(r, name, (ce_vec2_t) {256, 256});
                ct_ui_a0->tooltip_end();
            }

            if (selected) {
                uint64_t obj = ce_cdb_a0->obj_from_uid(ce_cdb_a0->db(), r);
                *selected_asset = obj;
                changed = true;

                modal_buffer[0] = '\0';
                break;
            }
        }

        ct_asset_a0->clean_assets_list(assets, ce_memory_a0->system);
        ct_ui_a0->popup_end();
    }

    return changed;
}


static void ui_body_begin() {
    ct_ui_a0->columns(0, 2, true);
}

static void ui_body_end() {
    ct_ui_a0->columns_end();
}

static bool ui_header_begin(const char *name,
                            uint64_t id) {
//    ct_ui_a0->separator();

    ui_body_end();
    bool open = ct_ui_a0->collapsing_header(&(ct_ui_collapsing_header_t0) {
            .id=id,
            .text=name,
            .flags = CT_TREE_NODE_FLAGS_DefaultOpen});
    if (open) {
        ct_ui_a0->indent();
    }
    return open;
}

static void ui_header_end(bool open,
                          uint64_t id) {
    if (open) {
        ct_ui_a0->unindent();
    }
}

static void ui_asset(uint64_t obj,
                     const char *label,
                     uint64_t prop,
                     uint64_t asset_type,
                     uint64_t context,
                     uint32_t i) {
    if (!obj) {
        return;
    }

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    uint64_t ref_obj = ce_cdb_a0->read_ref(reader, prop, 0);

    ce_cdb_uuid_t0 ref_obj_uid = ce_cdb_a0->obj_uid(ce_cdb_a0->db(), ref_obj);
    char buffer[128] = {'\0'};

    const char *fn = ct_asset_a0->asset_filename(ref_obj_uid);
    if (fn) {
        strcpy(buffer, fn);
    }

    char labelid[128] = {'\0'};
    char modal_id[128] = {'\0'};

    bool change = false;

    ct_asset_i0 *ri = ct_asset_a0->get_interface(asset_type);


    uint64_t new_value = 0;

    sprintf(modal_id, ICON_FA_FOLDER_OPEN
            " ""select...##select_asset_%d", i);

    change = ui_asset_select_modal(obj + prop, obj + prop,
                                   asset_type, &new_value);


    const char *icon = ri && ri->display_icon ? ri->display_icon : NULL;
    if (icon) {
        snprintf(labelid, CE_ARRAY_LEN(labelid), "%s %s", icon, label);
    } else {
        snprintf(labelid, CE_ARRAY_LEN(labelid), "%s", label);
    }

    ui_label(labelid, obj, &prop, 1);

    ui_value_begin(obj, &prop, 1);

    // Select btn
    sprintf(labelid, ICON_FA_ARROW_UP
            "##%sprop_open_select_asset_%d", label, i);
    if (ct_ui_a0->button(&(ct_ui_button_t0) {.text=labelid})) {
        ct_selected_object_a0->set_selected_object(context, ref_obj);
    };

    ct_ui_a0->same_line(0, 2);

    // Open btn
    sprintf(labelid, ICON_FA_FOLDER_OPEN
            "##%sprop_select_asset_%d", label, i);
    ct_ui_a0->same_line(0, 2);

    if (ct_ui_a0->button(&(ct_ui_button_t0) {.text=labelid})) {
        ct_ui_a0->popup_open(obj + prop);
    };

    ct_ui_a0->same_line(0, 2);

    sprintf(labelid, "##%sasset_prop_str_%d", label, i);
    ct_ui_a0->push_item_width(-1);
    ct_ui_a0->text(buffer);
    ct_ui_a0->pop_item_width();

    if (ct_ui_a0->drag_drop_target_begin()) {
        const void *payload = ct_ui_a0->accept_drag_drop_payload("asset");

        if (payload) {
            uint64_t drag_obj = *((uint64_t *) payload);

            if (drag_obj) {

                uint64_t drag_asset_type = ce_cdb_a0->obj_type(ce_cdb_a0->db(),
                                                               drag_obj);

                if (drag_asset_type == asset_type) {
                    new_value = drag_obj;
                    change = true;
                }
            }
        }

        ct_ui_a0->drag_drop_target_end();
    }

    ui_value_end(obj, &prop, 1);
    if (change) {
        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_ref(w, prop, new_value);
        ce_cdb_a0->write_commit(w);
    }

}

static void ui_vec3(uint64_t obj,
                    const char *label,
                    const uint64_t prop[3],
                    struct ui_vec3_p0 params) {
    if (!obj) {
        return;
    }

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    ce_vec3_t value = {
            .x = ce_cdb_a0->read_float(reader, prop[0], 0.0f),
            .y = ce_cdb_a0->read_float(reader, prop[1], 0.0f),
            .z = ce_cdb_a0->read_float(reader, prop[2], 0.0f),
    };

    ce_vec3_t value_new = value;

    const float min = !params.min_f ? -FLT_MAX : params.min_f;
    const float max = !params.max_f ? FLT_MAX : params.max_f;

    ui_label(label, obj, prop, 3);

    ui_value_begin(obj, prop, 3);
    ct_ui_a0->push_item_width(-1);

    if (ct_ui_a0->drag_float3(&(ct_ui_drag_float_t0) {
            .id=_combine_hash(obj, prop[0]),
            .v_min = min,
            .v_max = max,
    }, &value_new)) {
        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_float(w, prop[0], value_new.x);
        ce_cdb_a0->set_float(w, prop[1], value_new.y);
        ce_cdb_a0->set_float(w, prop[2], value_new.z);
        ce_cdb_a0->write_commit(w);
    }

    ct_ui_a0->pop_item_width();
    ui_value_end(obj, prop, 3);
}

static void ui_vec2(uint64_t obj,
                    const char *label,
                    const uint64_t prop[2],
                    struct ui_vec2_p0 params) {
    if (!obj) {
        return;
    }

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    ce_vec2_t value = {
            .x = ce_cdb_a0->read_float(reader, prop[0], 0.0f),
            .y = ce_cdb_a0->read_float(reader, prop[1], 0.0f),
    };

    ce_vec2_t value_new = value;

    const float min = !params.min_f ? -FLT_MAX : params.min_f;
    const float max = !params.max_f ? FLT_MAX : params.max_f;

    ui_label(label, obj, prop, 3);

    ui_value_begin(obj, prop, 3);
    ct_ui_a0->push_item_width(-1);
    if (ct_ui_a0->drag_float2(&(ct_ui_drag_float_t0) {
            .id=_combine_hash(obj, prop[0]),
            .v_min = min,
            .v_max = max,
    }, &value_new)) {
        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_float(w, prop[0], value_new.x);
        ce_cdb_a0->set_float(w, prop[1], value_new.y);
        ce_cdb_a0->write_commit(w);
    }

    ct_ui_a0->pop_item_width();
    ui_value_end(obj, prop, 3);
}

static void ui_vec4(uint64_t obj,
                    const char *label,
                    const uint64_t prop[4],
                    struct ui_vec4_p0 params) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    ce_vec4_t value = {
            .x = ce_cdb_a0->read_float(reader, prop[0], 0.0f),
            .y = ce_cdb_a0->read_float(reader, prop[1], 0.0f),
            .z = ce_cdb_a0->read_float(reader, prop[2], 0.0f),
            .w = ce_cdb_a0->read_float(reader, prop[3], 0.0f),
    };

    ce_vec4_t value_new = value;

    const float min = !params.min_f ? -FLT_MAX : params.min_f;
    const float max = !params.max_f ? FLT_MAX : params.max_f;

    ui_label(label, obj, prop, 4);


    ui_value_begin(obj, prop, 4);

    ct_ui_a0->push_item_width(-1);

    bool changed;
    if (params.color) {
        changed = ct_ui_a0->color_edit(&(struct ct_ui_color_edit_t0) {
                .id=_combine_hash(obj, prop[0])
        }, &value_new);

    } else {
        changed = ct_ui_a0->drag_float4(&(ct_ui_drag_float_t0) {
                .id=_combine_hash(obj, prop[0]),
                .v_min = min,
                .v_max = max,
        }, &value_new);
    }

    if (changed) {
        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_float(w, prop[0], value_new.x);
        ce_cdb_a0->set_float(w, prop[1], value_new.y);
        ce_cdb_a0->set_float(w, prop[2], value_new.z);
        ce_cdb_a0->set_float(w, prop[3], value_new.w);
        ce_cdb_a0->write_commit(w);
    }
    ct_ui_a0->pop_item_width();
    ui_value_end(obj, prop, 4);
}

static void ui_disabled_begin() {
    ct_ui_a0->push_item_flag(CT_UI_ITEM_FLAGS_Disabled, true);
    ct_ui_a0->push_style_var(CT_UI_STYLE_VAR_Alpha, 0.5f);
}

static void ui_disabled_end() {
    ct_ui_a0->pop_item_flag();
    ct_ui_a0->pop_style_var(1);
}


static void _draw_object(ce_cdb_t0 db,
                         uint64_t obj,
                         const char *title,
                         uint64_t context);

static void _draw_property(ce_cdb_t0 db,
                           uint64_t obj,
                           uint64_t property,
                           const ce_cdb_prop_def_t0 *def,
                           uint64_t context,
                           bool recursive) {
    const ce_cdb_obj_o0 *o = ce_cdb_a0->read(db, obj);
    ce_cdb_type_e0 type = ce_cdb_a0->prop_type(o, property);
    switch (type) {
        case CE_CDB_TYPE_REF:
            ui_asset(obj, def->name, property,
                     def->obj_type, context, obj);
            break;
        case CE_CDB_TYPE_FLOAT:
            ui_float(obj, def->name, property, (ui_float_p0) {});
            break;

        case CE_CDB_TYPE_UINT64:
            ui_uint64(obj, def->name, property, (ui_uint64_p0) {});
            break;

        case CE_CDB_TYPE_BOOL:
            ui_bool(obj, def->name, property);
            break;

        case CE_CDB_TYPE_STR:
            ui_str(obj, def->name, property, obj);
            break;
        case CE_CDB_TYPE_SUBOBJECT: {
            if (!recursive) break;

            const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
            uint64_t subobj = ce_cdb_a0->read_subobject(r, property, 0);
            uint64_t subobj_type = ce_cdb_a0->obj_type(db, subobj);
            const char *title = ce_id_a0->str_from_id64(property);

            bool open = true;

            draw_aspect *aspect = ce_cdb_a0->get_aspect(subobj_type, CT_PROPERTY_EDITOR_ASPECT);
            if (!aspect) {
                open = ui_header_begin(def->name, _combine_hash(obj, property));
                ui_body_begin();
            }


            if (open) {

                _draw_object(db, subobj, title, context);
                ui_body_end();
            }

            if (!aspect) {
                ui_header_end(open, _combine_hash(obj, property));
            }


            break;
        }
        case CE_CDB_TYPE_SET_SUBOBJECT: {
            if (!recursive) break;

            bool open = ui_header_begin(def->name, _combine_hash(obj, property));

            if (open) {
                const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
                uint64_t n = ce_cdb_a0->read_objset_num(r, property);
                uint64_t k[n];
                ce_cdb_a0->read_objset(r, property, k);
                for (int j = 0; j < n; ++j) {
                    uint64_t subobj = k[j];

                    const ce_cdb_obj_o0 *sub_r = ce_cdb_a0->read(db, subobj);
                    const char *name = ce_cdb_a0->read_str(sub_r, ce_id_a0->id64("name"), NULL);
                    const char *text = name ? name : ce_id_a0->str_from_id64(
                            ce_cdb_a0->obj_type(db, subobj));

                    bool sub_open = ui_header_begin(text, _combine_hash(subobj, property));

                    if (sub_open) {
                        ui_body_begin();
                        _draw_object(db, subobj, text, context);
                        ui_body_end();
                    }
                    ui_header_end(sub_open, _combine_hash(subobj, property));
                }
//                ct_ui_a0->tree_pop();
            }
            ui_header_end(open, _combine_hash(obj, property));
        }
            break;

        default:
            break;
    }
}

static void _draw_object(ce_cdb_t0 db,
                         uint64_t obj,
                         const char *title,
                         uint64_t context) {
    uint64_t type = ce_cdb_a0->obj_type(db, obj);

    draw_aspect *aspect = ce_cdb_a0->get_aspect(type, CT_PROPERTY_EDITOR_ASPECT);
    if (aspect) {
        aspect(db, obj, title, context);
        return;
    }

    ct_cdb_type_def_t0 defs = ce_cdb_a0->obj_type_def(type);

    if (!defs.n) {
        return;
    }

    for (uint32_t i = 0; i < defs.n; ++i) {
        const ce_cdb_prop_def_t0 *def = &defs.props[i];

        if (def->flags & CE_CDB_PROP_FLAG_RUNTIME) {
            continue;
        }

        uint64_t prop_name = ce_id_a0->id64(def->name);
        _draw_property(db, obj, prop_name, def, context, true);
    }
}

static void on_debugui(uint64_t content,
                       uint64_t context,
                       uint64_t selected_object) {
    ce_cdb_t0 db = ce_cdb_a0->db();
    const ce_cdb_obj_o0 *sub_r = ce_cdb_a0->read(db, selected_object);
    const char *name = ce_cdb_a0->read_str(sub_r, ce_id_a0->id64("name"), NULL);
    const char *text = name ? name : ce_id_a0->str_from_id64(
            ce_cdb_a0->obj_type(db, selected_object));

    ui_body_begin();
    _draw_object(db, selected_object, text, context);
    ui_body_end();
}

static void _ui_vec2(ce_cdb_t0 db,
                     uint64_t obj,
                     const char *title,
                     uint64_t context) {
    ui_vec2(obj, title,
            (uint64_t[]) {PROP_VEC_X, PROP_VEC_Y},
            (ui_vec2_p0) {});
}

static void _ui_vec3(ce_cdb_t0 db,
                     uint64_t obj,
                     const char *title,
                     uint64_t context) {
    ui_vec3(obj, title,
            (uint64_t[]) {PROP_VEC_X, PROP_VEC_Y, PROP_VEC_Z},
            (ui_vec3_p0) {});
}

static void _ui_vec4(ce_cdb_t0 db,
                     uint64_t obj,
                     const char *title,
                     uint64_t context) {
    ui_vec4(obj, title,
            (uint64_t[]) {PROP_VEC_X, PROP_VEC_Y, PROP_VEC_Z, PROP_VEC_W},
            (ui_vec4_p0) {});
}

static void _ui_color4(ce_cdb_t0 db,
                       uint64_t obj,
                       const char *title,
                       uint64_t context) {
    ui_vec4(obj, title,
            (uint64_t[]) {PROP_COLOR_R, PROP_COLOR_G, PROP_COLOR_B, PROP_COLOR_A},
            (ui_vec4_p0) {.color = true});
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
        .draw_object =_draw_object,
        .ui_float = ui_float,
        .ui_uin64 = ui_uint64,
        .ui_str = ui_str,
        .ui_filename = ui_filename,
        .ui_str_combo = ui_str_combo,
        .ui_str_combo2 = ui_str_combo2,
        .ui_asset = ui_asset,
        .ui_vec2 = ui_vec2,
        .ui_vec3 = ui_vec3,
        .ui_vec4 = ui_vec4,
        .ui_bool = ui_bool,
        .ui_revert_btn = ui_revert_btn,
        .ui_asset_select_modal = ui_asset_select_modal,
        .ui_header_begin = ui_header_begin,
        .ui_header_end = ui_header_end,
        .ui_body_begin = ui_body_begin,
        .ui_body_end = ui_body_end,
        .ui_disabled_begin = ui_disabled_begin,
        .ui_disabled_end = ui_disabled_end,
        .ui_value_begin = ui_value_begin,
        .ui_value_end = ui_value_end,
        .ui_label = ui_label,
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

    ce_cdb_a0->set_aspect(VEC2_CDB_TYPE, CT_PROPERTY_EDITOR_ASPECT, _ui_vec2);
    ce_cdb_a0->set_aspect(VEC3_CDB_TYPE, CT_PROPERTY_EDITOR_ASPECT, _ui_vec3);
    ce_cdb_a0->set_aspect(VEC4_CDB_TYPE, CT_PROPERTY_EDITOR_ASPECT, _ui_vec4);
    ce_cdb_a0->set_aspect(COLOR4_CDB_TYPE, CT_PROPERTY_EDITOR_ASPECT, _ui_color4);

    ct_dock_a0->create_dock(CT_PROPERTY_EDITOR_I0, true);

}

void CE_MODULE_UNLOAD(property_inspector)(struct ce_api_a0 *api,
                                          int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _G = (struct _G) {};
}
