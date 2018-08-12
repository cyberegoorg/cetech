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

#include "cetech/editor/editor_ui.h"

#define _SET_BOOL \
    CE_ID64_0("set_bool", 0xc7b18e7df0217558ULL)

#define _SET_STR \
    CE_ID64_0("set_str", 0x5096c6f990f09debULL)

#define _SET_UINT64 \
    CE_ID64_0("set_uint64", 0xdf40f6493b54f476ULL)

#define _SET_VEC3 \
    CE_ID64_0("set_vec3", 0xc9710c4624eccfb0ULL)

#define _SET_FLOAT \
    CE_ID64_0("set_float", 0x3a22b9e23704ab12ULL)

//static bool ui_select_asset(char *buffer,
//                            const char *id,
//                            uint32_t asset_type,
//                            uint64_t key) {
//    char id_str[512] = {0};
//    sprintf(id_str, ">>##%s", id);
//
//    if (ct_debugui_a0->Button(id_str, (float[2]) {0.0f})) {
//        if (ct_asset_browser_a0->get_selected_asset_type() != asset_type) {
//            return false;
//        }
//
//        char selected_asset[128] = {0};
//        ct_asset_browser_a0->get_selected_asset_name(selected_asset);
//
//        strcpy(buffer, selected_asset);
//
//        return true;
//    }
//
//    return false;
//}

static void ui_float(uint64_t obj,
                     uint64_t prop_key_hash,
                     const char *label,
                     float min_f,
                     float max_f) {
    float value = 0;
    float value_new = 0;

    value_new = ce_cdb_a0->read_float(obj, prop_key_hash, value_new);
    value = value_new;

    const float min = !max_f ? -FLT_MAX : min_f;
    const float max = !max_f ? FLT_MAX : max_f;

    ct_debugui_a0->Text("%s", label);
    ct_debugui_a0->NextColumn();

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_float_%d", label, 0);

    if (ct_debugui_a0->DragFloat(labelid,
                                 &value_new, 1.0f,
                                 min, max,
                                 "%.3f", 1.0f)) {

        struct ct_cdb_cmd_s cmd = {
                .header = {
                        .size = sizeof(struct ct_cdb_cmd_s),
                        .type = _SET_FLOAT,
                },

                .prop = prop_key_hash,
                .obj = obj,
                .f.new_value = value_new,
                .f.old_value = value,
        };

        ct_cmd_system_a0->execute(&cmd.header);
    }
    ct_debugui_a0->NextColumn();
}

static void ui_bool(uint64_t obj,
                    uint64_t prop_key_hash,
                    const char *label) {
    bool value = false;
    bool value_new = false;

    value_new = ce_cdb_a0->read_bool(obj, prop_key_hash, value_new);
    value = value_new;

    ct_debugui_a0->Text("%s", label);
    ct_debugui_a0->NextColumn();

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_float_%d", label, 0);

    if (ct_debugui_a0->Checkbox(labelid, &value_new)) {

        struct ct_cdb_cmd_s cmd = {
                .header = {
                        .size = sizeof(struct ct_cdb_cmd_s),
                        .type = _SET_BOOL,
                },

                .prop = prop_key_hash,
                .obj = obj,
                .b.new_value = value_new,
                .b.old_value = value,
        };

        ct_cmd_system_a0->execute(&cmd.header);
    }

    ct_debugui_a0->NextColumn();
}

static void ui_str(uint64_t obj,
                   uint64_t prop_key_hash,
                   const char *label,
                   uint32_t i) {
    char labelid[128] = {'\0'};

    const char *value = 0;

    value = ce_cdb_a0->read_str(obj, prop_key_hash, NULL);

    char buffer[128] = {'\0'};
    strcpy(buffer, value);


    sprintf(labelid, "##%sprop_str_%d", label, i);

    ct_debugui_a0->Text("%s", label);
    ct_debugui_a0->NextColumn();

    bool change = false;


    ct_debugui_a0->PushItemWidth(-1);
    change |= ct_debugui_a0->InputText(labelid,
                                       buffer,
                                       strlen(buffer),
                                       DebugInputTextFlags_ReadOnly,
                                       0, NULL);
    ct_debugui_a0->PopItemWidth();

    if (change) {
        struct ct_cdb_cmd_s cmd = {
                .header = {
                        .size = sizeof(struct ct_cdb_cmd_s),
                        .type = _SET_STR,
                },

                .prop = prop_key_hash,
                .obj = obj,
        };

        strcpy(cmd.str.new_value, buffer);
        strcpy(cmd.str.old_value, value);

        ct_cmd_system_a0->execute(&cmd.header);
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
    value = ce_cdb_a0->read_str(obj, prop_key_hash, NULL);

    char *items = NULL;
    uint32_t items_count = 0;

    combo_items(obj, &items, &items_count);

    int current_item = -1;

    const char *items2[items_count];
    memset(items2, 0, sizeof(const char *)* items_count);

    for (int j = 0; j < items_count; ++j) {
        items2[j] = &items[j * 128];
        if(value) {
            if (ce_id_a0->id64(items2[j]) == ce_id_a0->id64(value)) {
                current_item = j;
            }
        }
    }

    ct_debugui_a0->Text("%s", label);
    ct_debugui_a0->NextColumn();

    char labelid[128] = {'\0'};

    char buffer[128] = {'\0'};

    if(value) {
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
        struct ct_cdb_cmd_s cmd = {
                .header = {
                        .size = sizeof(struct ct_cdb_cmd_s),
                        .type = _SET_STR,
                },

                .prop = prop_key_hash,
                .obj = obj,
        };

        strcpy(cmd.str.new_value, buffer);
        if(value) {
            strcpy(cmd.str.old_value, value);
        }

        ct_cmd_system_a0->execute(&cmd.header);
    }
    ct_debugui_a0->NextColumn();
}

static void ui_resource(uint64_t obj,
                        uint64_t prop_key_hash,
                        const char *label,
                        uint64_t resource_type,
                        uint32_t i) {

    char labelid[128] = {'\0'};

    uint64_t value = ce_cdb_a0->read_uint64(obj, prop_key_hash, 0);

    uint64_t resource_obj = ct_resource_a0->get((struct ct_resource_id) {
            .type = resource_type,
            .name = value
    });

    char buffer[128] = {'\0'};

    ct_builddb_a0->get_filename_type_name(buffer, CE_ARRAY_LEN(buffer),
                                          resource_type, value);

    sprintf(labelid, "##%sprop_str_%d", label, i);


    bool change = false;

//    change = ui_select_asset(buffer, labelid,
//                             resource_type,
//                             prop_key_hash);
//
//    ct_debugui_a0->SameLine(0.0f, -1.0f);
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
        ct_property_editor_a0->draw(resource_obj);
        ct_debugui_a0->TreePop();
    }

    if (change) {
        struct ct_cdb_cmd_s cmd = {
                .header = {
                        .size = sizeof(struct ct_cdb_cmd_s),
                        .type = _SET_UINT64,
                },

                .prop = prop_key_hash,
                .obj = obj,
        };

        cmd.u64.new_value = new_value;
        cmd.u64.old_value = value;

        ct_cmd_system_a0->execute(&cmd.header);
    }
}

static void ui_vec3(uint64_t obj,
                    uint64_t prop_key_hash,
                    const char *label,
                    float min_f,
                    float max_f) {
    float value[3] = {0};
    float value_new[3] = {0};

    ce_cdb_a0->read_vec3(obj, prop_key_hash, value_new);
    ce_vec3_move(value, value_new);

    const float min = !min_f ? -FLT_MAX : min_f;
    const float max = !max_f ? FLT_MAX : max_f;

    ct_debugui_a0->Text("%s", label);
    ct_debugui_a0->NextColumn();

    char labelid[128] = {'\0'};
    sprintf(labelid, "##%sprop_vec3_%d", label, 0);

    ct_debugui_a0->PushItemWidth(-1);
    if (ct_debugui_a0->DragFloat3(labelid,
                                  value_new, 1.0f,
                                  min, max,
                                  "%.3f", 1.0f)) {

        struct ct_cdb_cmd_s cmd = {
                .header = {
                        .size = sizeof(struct ct_cdb_cmd_s),
                        .type = _SET_VEC3,
                },

                .prop = prop_key_hash,
                .obj = obj,

                .vec3.new_value = {value_new[0], value_new[1], value_new[2]},
                .vec3.old_value = {value[0], value[1], value[2]},
        };

        ct_cmd_system_a0->execute(&cmd.header);
    }
    ct_debugui_a0->PopItemWidth();

    ct_debugui_a0->NextColumn();
}


static struct ct_editor_ui_a0 editor_ui_a0 = {
        .ui_float = ui_float,
        .ui_str = ui_str,
        .ui_str_combo = ui_str_combo,
        .ui_resource = ui_resource,
        .ui_vec3 = ui_vec3,
        .ui_bool = ui_bool,
};

struct ct_editor_ui_a0 *ct_editor_ui_a0 = &editor_ui_a0;


static void set_vec3_cmd(const struct ct_cmd *cmd,
                         bool inverse) {
    const struct ct_cdb_cmd_s *pos_cmd = (const struct ct_cdb_cmd_s *) cmd;

    const float *value = inverse ? pos_cmd->vec3.old_value
                                 : pos_cmd->vec3.new_value;

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(pos_cmd->obj);
    ce_cdb_a0->set_vec3(w, pos_cmd->prop, value);
    ce_cdb_a0->write_commit(w);
}


static void set_float_cmd(const struct ct_cmd *cmd,
                          bool inverse) {
    const struct ct_cdb_cmd_s *pos_cmd = (const struct ct_cdb_cmd_s *) cmd;

    const float value = inverse ? pos_cmd->f.old_value : pos_cmd->f.new_value;

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(pos_cmd->obj);
    ce_cdb_a0->set_float(w, pos_cmd->prop, value);
    ce_cdb_a0->write_commit(w);
}

static void set_uint64_cmd(const struct ct_cmd *cmd,
                           bool inverse) {
    const struct ct_cdb_cmd_s *pos_cmd = (const struct ct_cdb_cmd_s *) cmd;

    const uint64_t value = inverse ? pos_cmd->u64.old_value
                                   : pos_cmd->u64.new_value;

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(pos_cmd->obj);
    ce_cdb_a0->set_uint64(w, pos_cmd->prop, value);
    ce_cdb_a0->write_commit(w);
}

static void set_bool_cmd(const struct ct_cmd *cmd,
                         bool inverse) {
    const struct ct_cdb_cmd_s *pos_cmd = (const struct ct_cdb_cmd_s *) cmd;

    const bool value = inverse ? pos_cmd->b.old_value : pos_cmd->b.new_value;

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(pos_cmd->obj);
    ce_cdb_a0->set_bool(w, pos_cmd->prop, value);
    ce_cdb_a0->write_commit(w);
}

static void set_str_cmd(const struct ct_cmd *_cmd,
                        bool inverse) {
    const struct ct_cdb_cmd_s *pos_cmd = (const struct ct_cdb_cmd_s *) _cmd;

    const char *value = inverse ? pos_cmd->str.old_value
                                : pos_cmd->str.new_value;

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(pos_cmd->obj);
    ce_cdb_a0->set_str(w, pos_cmd->prop, value);
    ce_cdb_a0->write_commit(w);
}

static void cmd_description(char *buffer,
                            uint32_t buffer_size,
                            const struct ct_cmd *cmd,
                            bool inverse) {
    const struct ct_cdb_cmd_s *pos_cmd = (const struct ct_cdb_cmd_s *) cmd;

    switch (cmd->type) {
        case _SET_VEC3:
            snprintf(buffer, buffer_size,
                     "Set vec3 [%f, %f, %f]",
                     pos_cmd->vec3.new_value[0],
                     pos_cmd->vec3.new_value[1],
                     pos_cmd->vec3.new_value[2]);
            break;

        case _SET_FLOAT:
            snprintf(buffer, buffer_size,
                     "Set float %f",
                     pos_cmd->f.new_value);
            break;

        case _SET_STR:
            snprintf(buffer, buffer_size,
                     "Set str %s",
                     pos_cmd->str.new_value);
            break;

        default:
            break;
    }
}

static void _init(struct ce_api_a0 *api) {
    ct_cmd_system_a0->register_cmd_execute(_SET_VEC3,
                                           (struct ct_cmd_fce) {
                                                   .execute = set_vec3_cmd,
                                                   .description = cmd_description});

    ct_cmd_system_a0->register_cmd_execute(_SET_STR,
                                           (struct ct_cmd_fce) {
                                                   .execute = set_str_cmd,
                                                   .description = cmd_description});

    ct_cmd_system_a0->register_cmd_execute(_SET_BOOL,
                                           (struct ct_cmd_fce) {
                                                   .execute = set_bool_cmd,
                                                   .description = cmd_description});

    ct_cmd_system_a0->register_cmd_execute(_SET_FLOAT,
                                           (struct ct_cmd_fce) {
                                                   .execute = set_float_cmd,
                                                   .description = cmd_description});

    ct_cmd_system_a0->register_cmd_execute(_SET_UINT64,
                                           (struct ct_cmd_fce) {
                                                   .execute = set_uint64_cmd,
                                                   .description = cmd_description});

    api->register_api("ct_editor_ui_a0", ct_editor_ui_a0);
}

static void _shutdown() {
}

CE_MODULE_DEF(
        editor_ui,
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
