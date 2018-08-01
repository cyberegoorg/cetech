#include <float.h>
#include <stdio.h>
#include <time.h>

#include <corelib/hashlib.h>
#include <corelib/config.h>
#include <corelib/memory.h>
#include <corelib/api_system.h>
#include <corelib/ydb.h>
#include <corelib/array.inl>
#include <corelib/module.h>

#include <cetech/command_system/command_system.h>
#include <corelib/hash.inl>
#include <corelib/cdb.h>
#include <cetech/debugui/debugui.h>
#include <corelib/fmath.inl>
#include <cetech/asset_browser/asset_browser.h>
#include <cetech/builddb/builddb.h>
#include <corelib/os.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/resource/resource.h>

#include "../editor_ui.h"

#define _SET_BOOL \
    CT_ID64_0("set_bool", 0xc7b18e7df0217558ULL)

#define _SET_STR \
    CT_ID64_0("set_str", 0x5096c6f990f09debULL)

#define _SET_UINT64 \
    CT_ID64_0("set_uint64", 0xdf40f6493b54f476ULL)

#define _SET_VEC3 \
    CT_ID64_0("set_vec3", 0xc9710c4624eccfb0ULL)

#define _SET_FLOAT \
    CT_ID64_0("set_float", 0x3a22b9e23704ab12ULL)

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

    value_new = ct_cdb_a0->read_float(obj, prop_key_hash, value_new);
    value = value_new;

    const float min = !max_f ? -FLT_MAX : min_f;
    const float max = !max_f ? FLT_MAX : max_f;

    if (ct_debugui_a0->DragFloat(label,
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
}

static void ui_bool(uint64_t obj,
                    uint64_t prop_key_hash,
                    const char *label) {
    bool value = false;
    bool value_new = false;

    value_new = ct_cdb_a0->read_bool(obj, prop_key_hash, value_new);
    value = value_new;

    if (ct_debugui_a0->Checkbox(label, &value_new)) {

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
}

static void ui_str(uint64_t obj,
                   uint64_t prop_key_hash,
                   const char *label,
                   uint32_t i) {
    char labelid[128] = {'\0'};

    const char *value = 0;

    value = ct_cdb_a0->read_str(obj, prop_key_hash, NULL);

    char buffer[128] = {'\0'};
    strcpy(buffer, value);

    sprintf(labelid, "%s##prop_str_%d", label, i);


    bool change = false;

    change |= ct_debugui_a0->InputText(labelid,
                                       buffer,
                                       strlen(buffer),
                                       DebugInputTextFlags_ReadOnly,
                                       0, NULL);

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
}

static void ui_str_combo(uint64_t obj,
                         uint64_t prop_key_hash,
                         const char *label,
                         void (*combo_items)(uint64_t obj,
                                             char **items,
                                             uint32_t *items_count),
                         uint32_t i) {
    char labelid[128] = {'\0'};

    const char *value = 0;

    value = ct_cdb_a0->read_str(obj, prop_key_hash, NULL);

    char buffer[128] = {'\0'};
    strcpy(buffer, value);

    char *items = NULL;
    uint32_t items_count = 0;

    combo_items(obj, &items, &items_count);

    int current_item = -1;
    const char *items2[items_count];
    for (int j = 0; j < items_count; ++j) {
        items2[j] = &items[j * 128];
        if (ct_hashlib_a0->id64(items2[j]) ==
            ct_hashlib_a0->id64(value)) {
            current_item = j;
        }
    }

    sprintf(labelid, "%s##combo_%d", label, i);
    bool change = ct_debugui_a0->Combo(labelid,
                                       &current_item, items2,
                                       items_count, -1);

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
        strcpy(cmd.str.old_value, value);

        ct_cmd_system_a0->execute(&cmd.header);
    }
}

static void ui_resource(uint64_t obj,
                        uint64_t prop_key_hash,
                        const char *label,
                        uint64_t resource_type,
                        uint32_t i) {
    char labelid[128] = {'\0'};

    uint64_t value = ct_cdb_a0->read_uint64(obj, prop_key_hash, 0);

    uint64_t resource_obj = ct_resource_a0->get((struct ct_resource_id) {
            .type = resource_type,
            .name = value
    });

    char buffer[128] = {'\0'};

    ct_builddb_a0->get_filename_type_name(buffer, CT_ARRAY_LEN(buffer),
                                          resource_type, value);

    sprintf(labelid, "%s##prop_str_%d", label, i);


    bool change = false;

//    change = ui_select_asset(buffer, labelid,
//                             resource_type,
//                             prop_key_hash);
//
//    ct_debugui_a0->SameLine(0.0f, -1.0f);

    change |= ct_debugui_a0->InputText(labelid,
                                       buffer,
                                       strlen(buffer),
                                       DebugInputTextFlags_ReadOnly,
                                       0, NULL);

    uint64_t new_value = 0;
    if (ct_debugui_a0->BeginDragDropTarget()) {
        const struct DebugUIPayload *payload;
        payload = ct_debugui_a0->AcceptDragDropPayload("asset", 0);

        if (payload) {
            uint64_t drag_obj = *((uint64_t *) payload->Data);

            if (drag_obj) {
                uint64_t asset_type = ct_cdb_a0->read_uint64(drag_obj,
                                                             ASSET_BROWSER_ASSET_TYPE2,
                                                             0);

                uint64_t asset_name = ct_cdb_a0->read_uint64(drag_obj,
                                                             ASSET_BROWSER_ASSET_NAME,
                                                             0);

                if (resource_type == asset_type) {
                    new_value = asset_name;
                    change = true;
                }
            }
        }

        ct_debugui_a0->EndDragDropTarget();
    }


    ct_property_editor_a0->draw(resource_obj);

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

    ct_cdb_a0->read_vec3(obj, prop_key_hash, value_new);
    ct_vec3_move(value, value_new);

    const float min = !min_f ? -FLT_MAX : min_f;
    const float max = !max_f ? FLT_MAX : max_f;

    if (ct_debugui_a0->DragFloat3(label,
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

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(pos_cmd->obj);
    ct_cdb_a0->set_vec3(w, pos_cmd->prop, value);
    ct_cdb_a0->write_commit(w);
}


static void set_float_cmd(const struct ct_cmd *cmd,
                          bool inverse) {
    const struct ct_cdb_cmd_s *pos_cmd = (const struct ct_cdb_cmd_s *) cmd;

    const float value = inverse ? pos_cmd->f.old_value : pos_cmd->f.new_value;

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(pos_cmd->obj);
    ct_cdb_a0->set_float(w, pos_cmd->prop, value);
    ct_cdb_a0->write_commit(w);
}

static void set_uint64_cmd(const struct ct_cmd *cmd,
                           bool inverse) {
    const struct ct_cdb_cmd_s *pos_cmd = (const struct ct_cdb_cmd_s *) cmd;

    const uint64_t value = inverse ? pos_cmd->u64.old_value
                                   : pos_cmd->u64.new_value;

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(pos_cmd->obj);
    ct_cdb_a0->set_uint64(w, pos_cmd->prop, value);
    ct_cdb_a0->write_commit(w);
}

static void set_bool_cmd(const struct ct_cmd *cmd,
                         bool inverse) {
    const struct ct_cdb_cmd_s *pos_cmd = (const struct ct_cdb_cmd_s *) cmd;

    const bool value = inverse ? pos_cmd->b.old_value : pos_cmd->b.new_value;

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(pos_cmd->obj);
    ct_cdb_a0->set_bool(w, pos_cmd->prop, value);
    ct_cdb_a0->write_commit(w);
}

static void set_str_cmd(const struct ct_cmd *_cmd,
                        bool inverse) {
    const struct ct_cdb_cmd_s *pos_cmd = (const struct ct_cdb_cmd_s *) _cmd;

    const char *value = inverse ? pos_cmd->str.old_value
                                : pos_cmd->str.new_value;

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(pos_cmd->obj);
    ct_cdb_a0->set_str(w, pos_cmd->prop, value);
    ct_cdb_a0->write_commit(w);
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

static void _init(struct ct_api_a0 *api) {
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


}

static void _shutdown() {
}

CETECH_MODULE_DEF(
        editor_ui,
        {
            CT_INIT_API(api, ct_memory_a0);
            CT_INIT_API(api, ct_hashlib_a0);
            CT_INIT_API(api, ct_cdb_a0);
            CT_INIT_API(api, ct_cmd_system_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
            _shutdown();
        }
)
