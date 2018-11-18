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

#include <cetech/editor/command_system.h>
#include <celib/hash.inl>
#include <celib/cdb.h>
#include <cetech/gfx/debugui.h>
#include <celib/fmath.inl>
#include <cetech/editor/asset_browser.h>
#include <cetech/resource/builddb.h>
#include <celib/os.h>
#include <cetech/editor/property_editor.h>
#include <cetech/resource/resource.h>
#include <cetech/sourcedb/sourcedb.h>
#include <cetech/gfx/private/iconfontheaders/icons_font_awesome.h>

#include "cetech/sourcedb/sourcedb_ui.h"


static void _revert_keys(uint64_t *keys,
                         uint64_t n) {
    uint64_t end = n - 1;
    for (uint64_t c = 0; c < n / 2; c++) {
        uint64_t t = keys[c];
        keys[c] = keys[end];
        keys[end] = t;
        end--;
    }
}

static void _collect_keys(struct ct_resource_id rid,
                          uint64_t obj,
                          uint64_t **keys,
                          struct ce_alloc *alloc) {

    do {
        if (ce_cdb_a0->read_uint64(obj, ASSET_NAME, 0) == rid.name) {
            break;
        }

        uint64_t k = ce_cdb_a0->key(obj);
        ce_array_push(*keys, k, alloc);

        obj = ce_cdb_a0->parent(obj);
    } while (true);

    _revert_keys(*keys, ce_array_size(*keys));
}

static uint64_t _find_recursive(uint64_t obj,
                                uint64_t *keys) {
    uint64_t n = ce_array_size(keys);
    return ce_cdb_a0->read_subobject_deep(obj, keys, n, obj);
}

static void _prop_label(struct ct_resource_id rid,
                        const char *label,
                        uint64_t _obj,
                        uint64_t prop_key_hash) {
    if (ce_cdb_a0->prefab(_obj)
        && ce_cdb_a0->prop_exist_norecursive(_obj, prop_key_hash)) {

        char lbl[256] = {};
        snprintf(lbl, CE_ARRAY_LEN(lbl), "%s##revert_%llu_%llu",
                 ICON_FA_RECYCLE, _obj, prop_key_hash);

        bool remove_change = ct_debugui_a0->Button(lbl,
                                                   (float[2]) {});
        if (remove_change) {
            ce_cdb_obj_o *w = ce_cdb_a0->write_begin(_obj);
            ce_cdb_a0->delete_property(w, prop_key_hash);
            ce_cdb_a0->write_commit(w);
        }
        ct_debugui_a0->SameLine(0, 4);
    }

    ct_debugui_a0->Text("%s", label);
    ct_debugui_a0->NextColumn();
}


static void ui_float(struct ct_resource_id rid,
                     uint64_t obj,
                     uint64_t prop_key_hash,
                     const char *label,
                     float min_f,
                     float max_f) {
    float value = 0;
    float value_new = 0;

    uint64_t *keys = NULL;
    _collect_keys(rid, obj, &keys, ce_memory_a0->system);

    obj = _find_recursive(ct_sourcedb_a0->get(rid), keys);

    if (!obj) {
        return;
    }

    value_new = ce_cdb_a0->read_float(obj, prop_key_hash, value_new);
    value = value_new;

    const float min = !max_f ? -FLT_MAX : min_f;
    const float max = !max_f ? FLT_MAX : max_f;

    _prop_label(rid, label, obj, prop_key_hash);

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_float_%d", label, 0);

    if (ct_debugui_a0->DragFloat(labelid,
                                 &value_new, 1.0f,
                                 min, max,
                                 "%.3f", 1.0f)) {
        ct_sourcedb_a0->set_float(rid, prop_key_hash, keys, ce_array_size(keys),
                                  value, value_new);
    }

    ct_debugui_a0->NextColumn();
}

static void ui_bool(struct ct_resource_id rid,
                    uint64_t obj,
                    uint64_t prop_key_hash,
                    const char *label) {
    bool value = false;
    bool value_new = false;

    uint64_t *keys = NULL;
    _collect_keys(rid, obj, &keys, ce_memory_a0->system);

    obj = _find_recursive(ct_sourcedb_a0->get(rid), keys);

    value_new = ce_cdb_a0->read_bool(obj, prop_key_hash, value_new);
    value = value_new;

    _prop_label(rid, label, obj, prop_key_hash);

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_float_%d", label, 0);

    if (ct_debugui_a0->Checkbox(labelid, &value_new)) {
        ct_sourcedb_a0->set_bool(rid, prop_key_hash, keys, ce_array_size(keys),
                                 value, value_new);
    }

    ct_debugui_a0->NextColumn();
}


static void ui_str(struct ct_resource_id rid,
                   uint64_t obj,
                   uint64_t prop_key_hash,
                   const char *label,
                   uint32_t i) {
    char labelid[128] = {'\0'};

    const char *value = 0;

    uint64_t *keys = NULL;
    _collect_keys(rid, obj, &keys, ce_memory_a0->system);

    obj = _find_recursive(ct_sourcedb_a0->get(rid), keys);

    value = ce_cdb_a0->read_str(obj, prop_key_hash, "");

    char buffer[128] = {'\0'};
    strcpy(buffer, value);

    sprintf(labelid, "##%sprop_str_%d", label, i);

    _prop_label(rid, label, obj, prop_key_hash);

    bool change = false;

    ct_debugui_a0->PushItemWidth(-1);
    change |= ct_debugui_a0->InputText(labelid,
                                       buffer,
                                       CE_ARRAY_LEN(buffer),
                                       0,
                                       0, NULL);
    ct_debugui_a0->PopItemWidth();

    if (change) {
        ct_sourcedb_a0->set_str(rid, prop_key_hash,
                                keys, ce_array_size(keys),
                                value, buffer);
    }

    ct_debugui_a0->NextColumn();
}

static void ui_str_combo(struct ct_resource_id rid,
                         uint64_t obj,
                         uint64_t prop_key_hash,
                         const char *label,
                         void (*combo_items)(uint64_t obj,
                                             char **items,
                                             uint32_t *items_count),
                         uint32_t i) {

    const char *value = 0;

    uint64_t *keys = NULL;
    _collect_keys(rid, obj, &keys, ce_memory_a0->system);

    obj = _find_recursive(ct_sourcedb_a0->get(rid), keys);

    if (!obj) {
        return;
    }

    value = ce_cdb_a0->read_str(obj, prop_key_hash, NULL);

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

    _prop_label(rid, label, obj, prop_key_hash);

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
        ct_sourcedb_a0->set_str(rid, prop_key_hash,
                                keys, ce_array_size(keys),
                                value, buffer);
    }
    ct_debugui_a0->NextColumn();
}

static void ui_resource(struct ct_resource_id rid,
                        uint64_t obj,
                        uint64_t prop_key_hash,
                        const char *label,
                        uint64_t resource_type,
                        uint32_t i) {

    uint64_t *keys = NULL;
    _collect_keys(rid, obj, &keys, ce_memory_a0->system);

    obj = _find_recursive(ct_sourcedb_a0->get(rid), keys);

    if (!obj) {
        return;
    }

    const char *value = ce_cdb_a0->read_str(obj, prop_key_hash, 0);
    uint64_t value_id = ce_id_a0->id64(value);

    uint64_t resource_obj = ct_sourcedb_a0->get((struct ct_resource_id) {
            .type = resource_type,
            .name = value_id,
    });

    char buffer[128] = {'\0'};
    sprintf(buffer, "%s", value);

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_str_%d", label, i);


    bool change = false;

    ct_debugui_a0->Separator();
    bool resource_open = ct_debugui_a0->TreeNodeEx(label,
                                                   0);

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

            if (drag_obj) {
                uint64_t asset_type = ce_cdb_a0->read_uint64(drag_obj,
                                                             ASSET_TYPE,
                                                             0);

                uint64_t asset_name = ce_cdb_a0->read_uint64(drag_obj,
                                                             ASSET_NAME,
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
        ct_property_editor_a0->draw((struct ct_resource_id) {
                .type = resource_type,
                .name = value_id,
        }, resource_obj);
        ct_debugui_a0->TreePop();
    }

    if (change) {
        const char *new_value_str = ce_id_a0->str_from_id64(new_value);
        ct_sourcedb_a0->set_str(rid, prop_key_hash, keys, ce_array_size(keys),
                                value, new_value_str);
    }
}

static void ui_vec3(struct ct_resource_id rid,
                    uint64_t _obj,
                    uint64_t prop_key_hash,
                    const char *label,
                    float min_f,
                    float max_f) {
    float value[3] = {};
    float value_new[3] = {};

    uint64_t *keys = NULL;
    _collect_keys(rid, _obj, &keys, ce_memory_a0->system);

    uint64_t obj = _find_recursive(ct_sourcedb_a0->get(rid), keys);

    if (!obj) {
        return;
    }

    ce_cdb_a0->read_vec3(obj, prop_key_hash, value_new);
    ce_vec3_move(value, value_new);

    const float min = !min_f ? -FLT_MAX : min_f;
    const float max = !max_f ? FLT_MAX : max_f;

    _prop_label(rid, label, obj, prop_key_hash);

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_vec3_%d", label, 0);

    ct_debugui_a0->PushItemWidth(-1);
    if (ct_debugui_a0->DragFloat3(labelid,
                                  value_new, 1.0f,
                                  min, max,
                                  "%.3f", 1.0f)) {
        ct_sourcedb_a0->set_vec3(rid, prop_key_hash, keys, ce_array_size(keys),
                                 value, value_new);
    }

    ct_debugui_a0->PopItemWidth();


    ct_debugui_a0->NextColumn();

}

static void ui_vec4(struct ct_resource_id rid,
                    uint64_t obj,
                    uint64_t prop_key_hash,
                    const char *label,
                    float min_f,
                    float max_f) {
    float value[4] = {};
    float value_new[4] = {};

    uint64_t *keys = NULL;
    _collect_keys(rid, obj, &keys, ce_memory_a0->system);

    obj = _find_recursive(ct_sourcedb_a0->get(rid), keys);

    ce_cdb_a0->read_vec4(obj, prop_key_hash, value_new);
    ce_vec4_move(value, value_new);

    const float min = !min_f ? -FLT_MAX : min_f;
    const float max = !max_f ? FLT_MAX : max_f;

    _prop_label(rid, label, obj, prop_key_hash);

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_vec3_%d", label, 0);

    ct_debugui_a0->PushItemWidth(-1);
    if (ct_debugui_a0->DragFloat4(labelid,
                                  value_new, 1.0f,
                                  min, max,
                                  "%.3f", 1.0f)) {
        ct_sourcedb_a0->set_vec4(rid, prop_key_hash,
                                 keys, ce_array_size(keys),
                                 value,
                                 value_new);
    }
    ct_debugui_a0->PopItemWidth();

    ct_debugui_a0->NextColumn();
}

static void ui_color(struct ct_resource_id rid,
                     uint64_t obj,
                     uint64_t prop_key_hash,
                     const char *label,
                     float min_f,
                     float max_f) {
    float value[4] = {};
    float value_new[4] = {};

    uint64_t *keys = NULL;
    _collect_keys(rid, obj, &keys, ce_memory_a0->system);

    obj = _find_recursive(ct_sourcedb_a0->get(rid), keys);

    ce_cdb_a0->read_vec4(obj, prop_key_hash, value_new);
    ce_vec4_move(value, value_new);

    _prop_label(rid, label, obj, prop_key_hash);

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_vec3_%d", label, 0);


    ct_debugui_a0->PushItemWidth(-1);
    if (ct_debugui_a0->ColorEdit4(labelid,
                                  value_new, 1)) {
        ct_sourcedb_a0->set_vec4(rid, prop_key_hash,
                                 keys, ce_array_size(keys),
                                 value,
                                 value_new);
    }
    ct_debugui_a0->PopItemWidth();

    ct_debugui_a0->NextColumn();
}

static struct ct_sourcedb_ui_a0 editor_ui_a0 = {
        .ui_float = ui_float,
        .ui_str = ui_str,
        .ui_str_combo = ui_str_combo,
        .ui_resource = ui_resource,
        .ui_vec3 = ui_vec3,
        .ui_vec4 = ui_vec4,
        .ui_color= ui_color,
        .ui_bool = ui_bool,
};

struct ct_sourcedb_ui_a0 *ct_sourcedb_ui_a0 = &editor_ui_a0;


static void _init(struct ce_api_a0 *api) {
    api->register_api("ct_sourcedb_ui_a0", ct_sourcedb_ui_a0);
}

static void _shutdown() {
}

CE_MODULE_DEF(
        sourcedb_ui,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ct_cmd_system_a0);
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
