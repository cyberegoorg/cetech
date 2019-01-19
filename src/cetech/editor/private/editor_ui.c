#include <float.h>
#include <stdio.h>
#include <time.h>

#include <celib/hashlib.h>
#include <celib/config.h>
#include <celib/memory.h>
#include <celib/api_system.h>
#include <celib/ydb.h>
#include <celib/array.inl>
#include <celib/module.h>


#include <celib/hash.inl>
#include <celib/cdb.h>

#include <celib/fmath.inl>
#include <cetech/editor/resource_browser.h>
#include <cetech/resource/resourcedb.h>
#include <celib/os.h>
#include <cetech/editor/property.h>
#include <cetech/resource/resource.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <stdlib.h>
#include <cetech/editor/resource_preview.h>
#include <cetech/controlers/controlers.h>
#include <cetech/controlers/keyboard.h>

#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>

#include "cetech/editor/editor_ui.h"

static bool prop_revert_btn(uint64_t _obj,
                            const uint64_t *props,
                            uint64_t props_n) {
    const ce_cdb_obj_o *r = ce_cdb_a0->read(ce_cdb_a0->db(), _obj);
    uint64_t instance_of = ce_cdb_a0->read_instance_of(r);

    if (instance_of) {
        const ce_cdb_obj_o *ir = ce_cdb_a0->read(ce_cdb_a0->db(), instance_of);

        bool need_revert = false;
        for (int i = 0; i < props_n; ++i) {
            if (!ce_cdb_a0->prop_equal(r, ir, props[i])) {
                need_revert = true;
                break;
            }
        }

        if (need_revert) {
            char lbl[256] = {};
            snprintf(lbl, CE_ARRAY_LEN(lbl), "%s##revert_%llu_%llu",
                     ICON_FA_RECYCLE, _obj, props[0]);

            bool remove_change = ct_debugui_a0->Button(lbl,
                                                       (float[2]) {});

            if (remove_change) {
                ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), _obj);
                for (int i = 0; i < props_n; ++i) {
                    ce_cdb_a0->prop_copy(ir, w, props[i]);
                }
                ce_cdb_a0->write_commit(w);
            }

            return true;
        }
    }
    return false;
}


static void _prop_label(const char *label,
                        uint64_t _obj,
                        const uint64_t *props,
                        uint64_t props_n) {
    if (prop_revert_btn(_obj, props, props_n)) {
        ct_debugui_a0->SameLine(0, 4);
    }

    ct_debugui_a0->Text("%s", label);
    ct_debugui_a0->NextColumn();
}


static void resource_tooltip(struct ct_resource_id resourceid,
                             const char *path,
                             float size[2]) {
    ct_debugui_a0->Text("%s", path);

    uint64_t type = ct_resourcedb_a0->get_resource_type(resourceid);

    struct ct_resource_i0 *ri = ct_resource_a0->get_interface(type);

    if (!ri || !ri->get_interface) {
        return;
    }

    struct ct_resource_preview_i0 *ai = \
                (struct ct_resource_preview_i0 *) (ri->get_interface(
            RESOURCE_PREVIEW_I));

    uint64_t obj = resourceid.uid;

    if(ai) {
        if (ai->tooltip) {
            ai->tooltip(obj, size);
        }

        ct_resource_preview_a0->set_background_resource(resourceid);
        ct_resource_preview_a0->draw_background_texture(size);
    }

}

static void ui_float(uint64_t obj,
                     uint64_t prop_key_hash,
                     const char *label,
                     struct ui_float_p0 params) {
    float value = 0;
    float value_new = 0;

    if (!obj) {
        return;
    }

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    value_new = ce_cdb_a0->read_float(reader, prop_key_hash, value_new);
    value = value_new;

    const float min = !params.max_f ? -FLT_MAX : params.min_f;
    const float max = !params.max_f ? FLT_MAX : params.max_f;

    _prop_label(label, obj, &prop_key_hash, 1);

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_float_%d", label, 0);

    if (ct_debugui_a0->DragFloat(labelid,
                                 &value_new, 1.0f,
                                 min, max,
                                 "%.3f", 1.0f)) {

        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_float(w, prop_key_hash, value_new);
        ce_cdb_a0->write_commit(w);
    }

    ct_debugui_a0->NextColumn();
}

static void ui_bool(uint64_t obj,
                    uint64_t prop_key_hash,
                    const char *label) {
    bool value = false;
    bool value_new = false;

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    value_new = ce_cdb_a0->read_bool(reader, prop_key_hash, value_new);
    value = value_new;

    _prop_label(label, obj, &prop_key_hash, 1);

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_float_%d", label, 0);

    if (ct_debugui_a0->Checkbox(labelid, &value_new)) {
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_bool(w, prop_key_hash, value_new);
        ce_cdb_a0->write_commit(w);
    }

    ct_debugui_a0->NextColumn();
}


static void ui_str(uint64_t obj,
                   uint64_t prop_key_hash,
                   const char *label,
                   uint32_t i) {
    char labelid[128] = {'\0'};

    const char *value = 0;

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    value = ce_cdb_a0->read_str(reader, prop_key_hash, "");

    char buffer[128] = {'\0'};
    strcpy(buffer, value);

    sprintf(labelid, "##%sprop_str_%d", label, i);

    _prop_label(label, obj, &prop_key_hash, 1);

    bool change = false;

    ct_debugui_a0->PushItemWidth(-1);
    change |= ct_debugui_a0->InputText(labelid,
                                       buffer,
                                       CE_ARRAY_LEN(buffer),
                                       0,
                                       0, NULL);
    ct_debugui_a0->PopItemWidth();

    if (change) {
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_str(w, prop_key_hash, buffer);
        ce_cdb_a0->write_commit(w);
    }

    ct_debugui_a0->NextColumn();
}

static void ui_str_combo(uint64_t obj,
                         uint64_t prop_key_hash,
                         const char *label,
                         void (*combo_items)(uint64_t obj,
                                             char **items,
                                             uint32_t *items_count),
                         uint32_t i) {

    const char *value = 0;

    if (!obj) {
        return;
    }

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    value = ce_cdb_a0->read_str(reader, prop_key_hash, NULL);

    char *items = NULL;
    uint32_t items_count = 0;

    combo_items(obj, &items, &items_count);

    int current_item = -1;

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

    _prop_label(label, obj, &prop_key_hash, 1);

    char labelid[128] = {'\0'};

    char buffer[128] = {'\0'};

    if (value) {
        strcpy(buffer, value);
    }

    sprintf(labelid, "##%scombo_%d", label, i);
    ct_debugui_a0->PushItemWidth(-1);
    bool change = ct_debugui_a0->Combo(labelid,
                                       &current_item, items2,
                                       items_count, -1);
    ct_debugui_a0->PopItemWidth();

    if (change) {
        strcpy(buffer, items2[current_item]);
    }

    if (change) {
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_str(w, prop_key_hash, buffer);
        ce_cdb_a0->write_commit(w);
    }
    ct_debugui_a0->NextColumn();
}


static char modal_buffer[128] = {};

static bool resource_select_modal(const char *modal_id,
                                  uint64_t id,
                                  uint64_t resource_type,
                                  uint64_t *selected_resource,
                                  uint32_t *count) {
    bool changed = false;
    bool open = true;

    ct_debugui_a0->SetNextWindowSize((float[2]) {512, 512}, 0);
    if (ct_debugui_a0->BeginPopupModal(modal_id, &open, 0)) {
        struct ct_controlers_i0 *kb = ct_controlers_a0->get(CONTROLER_KEYBOARD);

        if (kb->button_pressed(0, kb->button_index("escape"))) {
            ct_debugui_a0->CloseCurrentPopup();
            ct_debugui_a0->EndPopup();
            return false;
        }

        char labelidi[128] = {'\0'};
        sprintf(labelidi, "##modal_input%llu", id);

        ct_debugui_a0->InputText(labelidi,
                                 modal_buffer,
                                 CE_ARRAY_LEN(modal_buffer),
                                 0,
                                 0, NULL);

        if (count) {
            int c = *count;
            ct_debugui_a0->InputInt("Count", &c, 1, 1, 0);
            *count = (uint32_t) c;
        }

        const char *resource_type_s = ce_id_a0->str_from_id64(resource_type);
        char **resources = NULL;

        ct_resourcedb_a0->get_resource_by_type(modal_buffer, resource_type_s,
                                               &resources,
                                               ce_memory_a0->system);

        uint32_t dir_n = ce_array_size(resources);
        for (int i = 0; i < dir_n; ++i) {
            const char *name = resources[i];

            if (!strlen(name)) {
                continue;
            }

            bool selected = ct_debugui_a0->Selectable(name, false, 0,
                                                      (float[2]) {0.0f});

            if (ct_debugui_a0->IsItemHovered(0)) {
                struct ct_resource_id r = {
                        .uid=ct_resourcedb_a0->get_uid(name, resource_type_s)
                };

                ct_debugui_a0->BeginTooltip();
                ct_editor_ui_a0->resource_tooltip(r, name,
                                                  (float[2]) {256, 256});
                ct_debugui_a0->EndTooltip();
            }

            if (selected) {
                *selected_resource = ct_resourcedb_a0->get_uid(name,
                                                               resource_type_s);
                changed = true;

                modal_buffer[0] = '\0';
                break;
            }
        }

        ct_resourcedb_a0->get_resource_by_type_clean(resources,
                                                     ce_memory_a0->system);
        ct_debugui_a0->EndPopup();
    }

    return changed;
}

static void ui_resource(uint64_t obj,
                        uint64_t prop_key_hash,
                        const char *label,
                        uint64_t resource_type,
                        uint32_t i) {
    if (!obj) {
        return;
    }

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    uint64_t uid = ce_cdb_a0->read_ref(reader, prop_key_hash, 0);

    uint64_t resource_obj = uid;

    const ce_cdb_obj_o *r = ce_cdb_a0->read(ce_cdb_a0->db(), resource_obj);
    const char *resource_name = ce_cdb_a0->read_str(r, ASSET_NAME_PROP, "");

    char buffer[128] = {'\0'};
    sprintf(buffer, "%s", resource_name);

    char labelid[128] = {'\0'};
    char modal_id[128] = {'\0'};

    bool change = false;

    ct_debugui_a0->Separator();
    bool resource_open = ct_debugui_a0->TreeNodeEx(label, 0);

    ct_debugui_a0->NextColumn();
    ct_debugui_a0->PushItemWidth(-1);

    sprintf(modal_id, "select...##%sselect_resource_%d", label, i);

    sprintf(labelid, ICON_FA_FOLDER_OPEN
            "##%sprop_select_resource_%d", label, i);
    if (ct_debugui_a0->Button(labelid, (float[2]) {0.0f})) {
        ct_debugui_a0->OpenPopup(modal_id);
    };

    uint64_t new_value = 0;

    change = resource_select_modal(modal_id, obj + prop_key_hash,
                                   resource_type, &new_value, NULL);


    ct_debugui_a0->SameLine(0.0f, 0.0f);

    sprintf(labelid, "##%sprop_str_%d", label, i);

    ct_debugui_a0->InputText(labelid,
                             buffer,
                             strlen(buffer),
                             DebugInputTextFlags_ReadOnly,
                             0, NULL);

    ct_debugui_a0->PopItemWidth();
    ct_debugui_a0->NextColumn();

    if (ct_debugui_a0->BeginDragDropTarget()) {
        const struct DebugUIPayload *payload;
        payload = ct_debugui_a0->AcceptDragDropPayload("asset", 0);

        if (payload) {
            uint64_t drag_obj = *((uint64_t *) payload->Data);

            if (drag_obj) {
                const ce_cdb_obj_o *dreader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                              drag_obj);
                uint64_t asset_type = ce_cdb_a0->obj_type(dreader);

                if (resource_type == asset_type) {
                    new_value = drag_obj;
                    change = true;
                }
            }
        }

        ct_debugui_a0->EndDragDropTarget();
    }

    if (resource_open) {
        ct_property_editor_a0->draw(resource_obj);
        ct_debugui_a0->TreePop();
    }

    if (change) {
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_ref(w, prop_key_hash, new_value);
        ce_cdb_a0->write_commit(w);
    }

}

static void ui_vec3(uint64_t obj,
                    const uint64_t prop_key_hash[3],
                    const char *label,
                    struct ui_vec3_p0 params) {
    if (!obj) {
        return;
    }

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    float value[3] = {
            ce_cdb_a0->read_float(reader, prop_key_hash[0], 0.0f),
            ce_cdb_a0->read_float(reader, prop_key_hash[1], 0.0f),
            ce_cdb_a0->read_float(reader, prop_key_hash[2], 0.0f),
    };

    float value_new[3] = {};
    ce_vec3_move(value_new, value);

    const float min = !params.min_f ? -FLT_MAX : params.min_f;
    const float max = !params.max_f ? FLT_MAX : params.max_f;

    _prop_label(label, obj, prop_key_hash, 3);

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_vec3_%d", label, 0);

    ct_debugui_a0->PushItemWidth(-1);
    if (ct_debugui_a0->DragFloat3(labelid,
                                  value_new, 1.0f,
                                  min, max,
                                  "%.3f", 1.0f)) {
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_float(w, prop_key_hash[0], value_new[0]);
        ce_cdb_a0->set_float(w, prop_key_hash[1], value_new[1]);
        ce_cdb_a0->set_float(w, prop_key_hash[2], value_new[2]);
        ce_cdb_a0->write_commit(w);
    }

    ct_debugui_a0->PopItemWidth();


    ct_debugui_a0->NextColumn();

}

static void ui_vec4(uint64_t obj,
                    const uint64_t prop_key_hash[4],
                    const char *label,
                    struct ui_vec4_p0 params) {
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    float value[4] = {
            ce_cdb_a0->read_float(reader, prop_key_hash[0], 0.0f),
            ce_cdb_a0->read_float(reader, prop_key_hash[1], 0.0f),
            ce_cdb_a0->read_float(reader, prop_key_hash[2], 0.0f),
            ce_cdb_a0->read_float(reader, prop_key_hash[3], 0.0f),
    };

    float value_new[4] = {};
    ce_vec4_move(value_new, value);

    const float min = !params.min_f ? -FLT_MAX : params.min_f;
    const float max = !params.max_f ? FLT_MAX : params.max_f;

    _prop_label(label, obj, prop_key_hash, 4);

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_vec3_%d", label, 0);

    ct_debugui_a0->PushItemWidth(-1);

    bool changed;
    if (params.color) {
        changed = ct_debugui_a0->ColorEdit4(labelid,
                                            value_new, 1);
    } else {
        changed = ct_debugui_a0->DragFloat4(labelid,
                                            value_new, 1.0f,
                                            min, max,
                                            "%.3f", 1.0f);
    }

    if (changed) {
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);
        ce_cdb_a0->set_float(w, prop_key_hash[0], value_new[0]);
        ce_cdb_a0->set_float(w, prop_key_hash[1], value_new[1]);
        ce_cdb_a0->set_float(w, prop_key_hash[2], value_new[2]);
        ce_cdb_a0->set_float(w, prop_key_hash[3], value_new[3]);
        ce_cdb_a0->write_commit(w);
    }
    ct_debugui_a0->PopItemWidth();

    ct_debugui_a0->NextColumn();
}

static uint64_t lock_selected_obj(uint64_t dock,
                                  uint64_t selected_obj) {
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), dock);

    uint64_t locked_object = ce_cdb_a0->read_ref(reader, CT_LOCKED_OBJ, 0);
    bool checked = locked_object != 0;
    if (ct_debugui_a0->Checkbox(ICON_FA_LOCK, &checked)) {
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), dock);
        if (checked) {
            ce_cdb_a0->set_ref(w, CT_LOCKED_OBJ, selected_obj);
        } else {
            ce_cdb_a0->remove_property(w, CT_LOCKED_OBJ);
        }
        ce_cdb_a0->write_commit(w);
    }

    return locked_object;
}

static struct ct_editor_ui_a0 editor_ui_a0 = {
        .prop_float = ui_float,
        .prop_str = ui_str,
        .prop_str_combo = ui_str_combo,
        .prop_resource = ui_resource,
        .prop_vec3 = ui_vec3,
        .prop_vec4 = ui_vec4,
        .prop_bool = ui_bool,
        .prop_revert_btn = prop_revert_btn,
        .resource_tooltip = resource_tooltip,
        .resource_select_modal = resource_select_modal,
        .lock_selected_obj = lock_selected_obj,

};

struct ct_editor_ui_a0 *ct_editor_ui_a0 = &editor_ui_a0;


static void _init(struct ce_api_a0 *api) {
    api->register_api(CT_RESOURCE_UI_API, ct_editor_ui_a0);
}

static void _shutdown() {
}

CE_MODULE_DEF(
        sourcedb_ui,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ce_cdb_a0);
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
