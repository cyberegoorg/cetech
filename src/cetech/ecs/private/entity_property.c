#include <float.h>
#include <stdio.h>

#include <corelib/hashlib.h>
#include <corelib/config.h>
#include <corelib/memory.h>
#include <corelib/api_system.h>
#include <corelib/ydb.h>
#include <corelib/array.inl>
#include <corelib/module.h>

#include <cetech/debugui/debugui.h>
#include <cetech/resource/resource.h>
#include <cetech/ecs/ecs.h>

#include <cetech/property_editor/property_editor.h>
#include <cetech/asset_browser/asset_browser.h>
#include <cetech/ecs/entity_property.h>
#include <cetech/explorer/explorer.h>
#include <corelib/ebus.h>
#include <corelib/fmath.inl>
#include <cetech/command_system/command_system.h>
#include <corelib/hash.inl>
#include <cetech/selected_object/selected_object.h>


#define _G entity_property_global

static struct _G {
    uint64_t active_entity;
    struct ct_entity top_entity;
    struct ct_world active_world;

    struct ct_hash_t components;

    struct ct_alloc *allocator;
    uint64_t obj;
} _G;


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

        struct ct_ent_cmd_s cmd = {
                .header = {
                        .size = sizeof(struct ct_ent_cmd_s),
                        .type = CT_ID64_0("set_float"),
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

        struct ct_ent_cmd_s cmd = {
                .header = {
                        .size = sizeof(struct ct_ent_cmd_s),
                        .type = CT_ID64_0("set_bool"),
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
        struct ct_ent_cmd_s cmd = {
                .header = {
                        .size = sizeof(struct ct_ent_cmd_s),
                        .type = CT_ID64_0("set_str"),
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
        if (CT_ID64_0(items2[j]) == CT_ID64_0(value)) {
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
        struct ct_ent_cmd_s cmd = {
                .header = {
                        .size = sizeof(struct ct_ent_cmd_s),
                        .type = CT_ID64_0("set_str"),
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
                        uint32_t resource_type,
                        uint32_t i) {
    char labelid[128] = {'\0'};

    const char *value = 0;

    value = ct_cdb_a0->read_str(obj, prop_key_hash, NULL);

    char buffer[128] = {'\0'};
    strcpy(buffer, value);

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

    if (change) {
        struct ct_ent_cmd_s cmd = {
                .header = {
                        .size = sizeof(struct ct_ent_cmd_s),
                        .type = CT_ID64_0("set_str"),
                },

                .prop = prop_key_hash,
                .obj = obj,
        };

        strcpy(cmd.str.new_value, buffer);
        strcpy(cmd.str.old_value, value);

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

        struct ct_ent_cmd_s cmd = {
                .header = {
                        .size = sizeof(struct ct_ent_cmd_s),
                        .type = CT_ID64_0("set_vec3"),
                },

                .prop = prop_key_hash,
                .obj = obj,

                .vec3.new_value = {value_new[0], value_new[1], value_new[2]},
                .vec3.old_value = {value[0],value[1],value[2]},
        };

        ct_cmd_system_a0->execute(&cmd.header);
    }
}

static struct ct_component_i0 *get_component_interface(uint64_t cdb_type) {
    struct ct_api_entry it = ct_api_a0->first("ct_component_i0");
    while (it.api) {
        struct ct_component_i0 *i = (it.api);

        if (cdb_type == i->cdb_type()) {
            return i;
        }

        it = ct_api_a0->next(it);
    }

    return NULL;
};

static void on_component(uint64_t obj) {
    uint64_t type = ct_cdb_a0->type(obj);

    struct ct_component_i0 *c = get_component_interface(type);
    if (!c->get_interface) {
        return;
    }

    struct ct_editor_component_i0 *editor = c->get_interface(EDITOR_COMPONENT);

    if (!ct_debugui_a0->CollapsingHeader(editor->display_name(),
                                         DebugUITreeNodeFlags_DefaultOpen)) {
        return;
    }

    if (!editor->property_editor) {
        return;
    }

    editor->property_editor(obj);
}

static void on_debugui() {
    uint64_t obj = ct_selected_object_a0->selected_object();
    if (!obj) {
        return;
    }

    uint64_t obj_type = ct_cdb_a0->type(obj);

    if (CT_ID64_0("entity") == obj_type) {
        if (ct_debugui_a0->CollapsingHeader("Entity",
                                            DebugUITreeNodeFlags_DefaultOpen)) {
            ct_debugui_a0->LabelText("Entity", "%llu", _G.active_entity);
        }

        uint64_t components_obj;
        components_obj = ct_cdb_a0->read_subobject(obj,
                                                   CT_ID64_0("components"), 0);


        uint64_t n = ct_cdb_a0->prop_count(components_obj);
        uint64_t components_name[n];
        ct_cdb_a0->prop_keys(components_obj, components_name);

        for (uint64_t j = 0; j < n; ++j) {
            uint64_t name = components_name[j];

            uint64_t c_obj;
            c_obj = ct_cdb_a0->read_subobject(components_obj, name, 0);

            on_component(c_obj);
        }

    } else if (get_component_interface(obj_type)) {
        on_component(obj);
    }
}

//void on_entity_click(uint64_t event) {
//    ct_property_editor_a0->set_active(on_debugui);
//
//    struct ct_world world = {
//            ct_cdb_a0->read_uint64(event, CT_ID64_0("world"), 0)};
//    const char *filename = ct_cdb_a0->read_str(event, CT_ID64_0("filename"), 0);
//    struct ct_entity entity = {
//            ct_cdb_a0->read_uint64(event, CT_ID64_0("entity"), 0)};
//    uint64_t obj = {ct_cdb_a0->read_ref(event, CT_ID64_0("obj"), 0)};
//
//    _G.active_world = world;
//    _G.top_entity = entity;
//    _G.obj = obj;
//}


static struct ct_entity_property_a0 entity_property_a0 = {
        .ui_float = ui_float,
        .ui_str = ui_str,
        .ui_str_combo = ui_str_combo,
        .ui_resource = ui_resource,
        .ui_vec3 = ui_vec3,
        .ui_bool = ui_bool,
};

static void set_vec3_cmd(const struct ct_cmd *cmd,
                         bool inverse) {
    const struct ct_ent_cmd_s *pos_cmd = (const struct ct_ent_cmd_s *) cmd;

    const float *value = inverse ? pos_cmd->vec3.old_value
                                 : pos_cmd->vec3.new_value;

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(pos_cmd->obj);
    ct_cdb_a0->set_vec3(w, pos_cmd->prop, value);
    ct_cdb_a0->write_commit(w);
}

static struct ct_property_editor_i0 ct_property_editor_i0 = {
        .draw = on_debugui,
};

static void set_float_cmd(const struct ct_cmd *cmd,
                          bool inverse) {
    const struct ct_ent_cmd_s *pos_cmd = (const struct ct_ent_cmd_s *) cmd;

    const float value = inverse ? pos_cmd->f.old_value : pos_cmd->f.new_value;

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(pos_cmd->obj);
    ct_cdb_a0->set_float(w, pos_cmd->prop, value);
    ct_cdb_a0->write_commit(w);
}

static void set_bool_cmd(const struct ct_cmd *cmd,
                          bool inverse) {
    const struct ct_ent_cmd_s *pos_cmd = (const struct ct_ent_cmd_s *) cmd;

    const bool value = inverse ? pos_cmd->b.old_value : pos_cmd->b.new_value;

    ct_cdb_obj_o *w = ct_cdb_a0->write_begin(pos_cmd->obj);
    ct_cdb_a0->set_bool(w, pos_cmd->prop, value);
    ct_cdb_a0->write_commit(w);
}

static void set_str_cmd(const struct ct_cmd *_cmd,
                        bool inverse) {
    const struct ct_ent_cmd_s *pos_cmd = (const struct ct_ent_cmd_s *) _cmd;

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
    const struct ct_ent_cmd_s *pos_cmd = (const struct ct_ent_cmd_s *) cmd;

    if (cmd->type == CT_ID64_0("set_vec3")) {
        snprintf(buffer, buffer_size,
                 "Set vec3 [%f, %f, %f]",
                 pos_cmd->vec3.new_value[0],
                 pos_cmd->vec3.new_value[1],
                 pos_cmd->vec3.new_value[2]);

    } else if (cmd->type == CT_ID64_0("set_float")) {
        snprintf(buffer, buffer_size,
                 "Set float %f",
                 pos_cmd->f.new_value);

    } else if (cmd->type == CT_ID64_0("set_str")) {
        snprintf(buffer, buffer_size,
                 "Set str %s",
                 pos_cmd->str.new_value);
    }
}

struct ct_entity_property_a0 *ct_entity_property_a0 = &entity_property_a0;

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0->system
    };

    api->register_api("ct_property_editor_i0", &ct_property_editor_i0);


    ct_cmd_system_a0->register_cmd_execute(
            CT_ID64_0("set_vec3"),
            (struct ct_cmd_fce) {
                    .execute = set_vec3_cmd,
                    .description = cmd_description});

    ct_cmd_system_a0->register_cmd_execute(
            CT_ID64_0("set_str"),
            (struct ct_cmd_fce) {
                    .execute = set_str_cmd,
                    .description = cmd_description});

    ct_cmd_system_a0->register_cmd_execute(
            CT_ID64_0("set_bool"),
            (struct ct_cmd_fce) {
                    .execute = set_bool_cmd,
                    .description = cmd_description});

    ct_cmd_system_a0->register_cmd_execute(
            CT_ID64_0("set_float"),
            (struct ct_cmd_fce) {
                    .execute = set_float_cmd,
                    .description = cmd_description});

}

static void _shutdown() {
    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        entity_property,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_property_editor_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_ebus_a0);
            CETECH_GET_API(api, ct_cmd_system_a0);
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
