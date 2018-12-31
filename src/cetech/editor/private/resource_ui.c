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
#include <cetech/debugui/debugui.h>
#include <celib/fmath.inl>
#include <cetech/editor/resource_browser.h>
#include <cetech/resource/builddb.h>
#include <celib/os.h>
#include <cetech/editor/property.h>
#include <cetech/resource/resource.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <stdlib.h>

#include "cetech/editor/resource_ui.h"


static void _prop_label(const char *label,
                        uint64_t _obj,
                        uint64_t prop_key_hash) {
//    if (ce_cdb_a0->prefab(_obj)
//        && ce_cdb_a0->prop_exist_norecursive(_obj, prop_key_hash)) {
//
//        char lbl[256] = {};
//        snprintf(lbl, CE_ARRAY_LEN(lbl), "%s##revert_%llu_%llu",
//                 ICON_FA_RECYCLE, _obj, prop_key_hash);
//
//        bool remove_change = ct_debugui_a0->Button(lbl,
//                                                   (float[2]) {});
//        if (remove_change) {
//            ce_cdb_obj_o *w = ce_cdb_a0->write_begin(_obj);
//            ce_cdb_a0->delete_property(w, prop_key_hash);
//            ce_cdb_a0->write_commit(w);
//        }
//        ct_debugui_a0->SameLine(0, 4);
//    }

    ct_debugui_a0->Text("%s", label);
    ct_debugui_a0->NextColumn();
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

    _prop_label(label, obj, prop_key_hash);

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

    _prop_label(label, obj, prop_key_hash);

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

    _prop_label(label, obj, prop_key_hash);

    bool change = false;

    ct_debugui_a0->PushItemWidth(-1);
    change |= ct_debugui_a0->InputText(labelid,
                                       buffer,
                                       CE_ARRAY_LEN(buffer),
                                       0,
                                       0, NULL);
    ct_debugui_a0->PopItemWidth();

    if (change) {
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),obj);
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

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(),obj);
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

    _prop_label(label, obj, prop_key_hash);

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
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),obj);
        ce_cdb_a0->set_str(w, prop_key_hash, buffer);
        ce_cdb_a0->write_commit(w);
    }
    ct_debugui_a0->NextColumn();
}

static void ui_resource(uint64_t obj,
                        uint64_t prop_key_hash,
                        const char *label,
                        uint64_t resource_type,
                        uint32_t i) {
    if (!obj) {
        return;
    }

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(),obj);

    uint64_t uid = ce_cdb_a0->read_ref(reader, prop_key_hash, 0);

    uint64_t resource_obj = uid;

    const ce_cdb_obj_o *r = ce_cdb_a0->read(ce_cdb_a0->db(),resource_obj);
    const char* resource_name = ce_cdb_a0->read_str(r, ASSET_NAME_PROP, "");

    char buffer[128] = {'\0'};
    sprintf(buffer, "%s", resource_name);

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_str_%d", label, i);


    bool change = false;

    ct_debugui_a0->Separator();
    bool resource_open = ct_debugui_a0->TreeNodeEx(label,0);

    ct_debugui_a0->NextColumn();
    ct_debugui_a0->PushItemWidth(-1);

    change |= ct_debugui_a0->InputText(labelid,
                                       buffer,
                                       strlen(buffer),
                                       DebugInputTextFlags_ReadOnly,
                                       0, NULL);
    ct_debugui_a0->PopItemWidth();
    ct_debugui_a0->NextColumn();

    uint64_t new_value = 0;
    if (ct_debugui_a0->BeginDragDropTarget()) {
        const struct DebugUIPayload *payload;
        payload = ct_debugui_a0->AcceptDragDropPayload("asset", 0);

        if (payload) {
            uint64_t drag_obj = *((uint64_t *) payload->Data);

            const ce_cdb_obj_o *dreader = ce_cdb_a0->read(ce_cdb_a0->db(),drag_obj);

            if (drag_obj) {
                uint64_t asset_type = ce_cdb_a0->read_uint64(dreader,
                                                             RESOURCE_TYPE,
                                                             0);

                uint64_t asset_name = ce_cdb_a0->read_uint64(dreader,
                                                             RESOURCE_NAME,
                                                             0);

                if (resource_type == asset_type) {
                    new_value = asset_name;
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
        const char *new_value_str = ce_id_a0->str_from_id64(new_value);
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),obj);
        ce_cdb_a0->set_str(w, prop_key_hash, new_value_str);
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

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(),obj);

    float value[3] = {
            ce_cdb_a0->read_float(reader, prop_key_hash[0], 0.0f),
            ce_cdb_a0->read_float(reader, prop_key_hash[1], 0.0f),
            ce_cdb_a0->read_float(reader, prop_key_hash[2], 0.0f),
    };

    float value_new[3] = {};
    ce_vec3_move(value_new, value);

    const float min = !params.min_f ? -FLT_MAX : params.min_f;
    const float max = !params.max_f ? FLT_MAX : params.max_f;

    _prop_label(label, obj, prop_key_hash[0]);

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_vec3_%d", label, 0);

    ct_debugui_a0->PushItemWidth(-1);
    if (ct_debugui_a0->DragFloat3(labelid,
                                  value_new, 1.0f,
                                  min, max,
                                  "%.3f", 1.0f)) {
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),obj);
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
//    uint64_t *keys = NULL;
//    _collect_keys(rid, obj, &keys, ce_memory_a0->system);
//
//    obj = _find_recursive(ct_sourcedb_a0->get(rid), keys);

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(),obj);

    float value[4] = {
            ce_cdb_a0->read_float(reader, prop_key_hash[0], 0.0f),
            ce_cdb_a0->read_float(reader, prop_key_hash[1], 0.0f),
            ce_cdb_a0->read_float(reader, prop_key_hash[2], 0.0f),
            ce_cdb_a0->read_float(reader, prop_key_hash[3], 0.0f),
    };

    float value_new[4] = {};
    ce_vec3_move(value_new, value);

    const float min = !params.min_f ? -FLT_MAX : params.min_f;
    const float max = !params.max_f ? FLT_MAX : params.max_f;

    _prop_label(label, obj, prop_key_hash[0]);

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
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(),obj);
        ce_cdb_a0->set_float(w, prop_key_hash[0], value_new[0]);
        ce_cdb_a0->set_float(w, prop_key_hash[1], value_new[1]);
        ce_cdb_a0->set_float(w, prop_key_hash[2], value_new[2]);
        ce_cdb_a0->set_float(w, prop_key_hash[3], value_new[3]);
        ce_cdb_a0->write_commit(w);
    }
    ct_debugui_a0->PopItemWidth();

    ct_debugui_a0->NextColumn();
}

static struct ct_resource_ui_a0 editor_ui_a0 = {
        .ui_float = ui_float,
        .ui_str = ui_str,
        .ui_str_combo = ui_str_combo,
        .ui_resource = ui_resource,
        .ui_vec3 = ui_vec3,
        .ui_vec4 = ui_vec4,
        .ui_bool = ui_bool,
};

struct ct_resource_ui_a0 *ct_resource_ui_a0 = &editor_ui_a0;


static void _init(struct ce_api_a0 *api) {
    api->register_api("ct_resource_ui_a0", ct_resource_ui_a0);
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
