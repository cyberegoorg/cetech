#include <float.h>
#include <stdio.h>

#include <cetech/kernel/hashlib/hashlib.h>
#include <cetech/kernel/config/config.h>
#include <cetech/kernel/memory/memory.h>
#include <cetech/kernel/api/api_system.h>
#include <cetech/kernel/yaml/ydb.h>
#include <cetech/kernel/containers/array.h>
#include <cetech/kernel/module/module.h>

#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/resource/resource.h>
#include <cetech/engine/ecs/ecs.h>

#include <cetech/playground/property_editor.h>
#include <cetech/playground/asset_browser.h>
#include <cetech/playground/entity_property.h>
#include <cetech/playground/explorer.h>
#include <cetech/kernel/ebus/ebus.h>
#include <cetech/kernel/math/fmath.h>
#include <cetech/playground/command_system.h>
#include <cetech/kernel/containers/hash.h>


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_property_editor_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_explorer_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_ecs_a0);
CETECH_DECL_API(ct_ebus_a0);
CETECH_DECL_API(ct_cmd_system_a0);


#define _G entity_property_global

static struct _G {
    uint64_t active_entity;
    struct ct_entity top_entity;
    struct ct_world active_world;

    struct ct_hash_t components;

    const char *filename;
    struct ct_alloc *allocator;
    struct ct_cdb_obj_t *obj;
} _G;


static bool ui_select_asset(char *buffer,
                            const char *id,
                            uint32_t asset_type,
                            uint64_t key) {
    char id_str[512] = {0};
    sprintf(id_str, ">>##%s", id);

    if (ct_debugui_a0.Button(id_str, (float[2]) {0.0f})) {
        if (ct_asset_browser_a0.get_selected_asset_type() != asset_type) {
            return false;
        }

        char selected_asset[128] = {0};
        ct_asset_browser_a0.get_selected_asset_name(selected_asset);

        strcpy(buffer, selected_asset);

        return true;
    }

    return false;
}

static void ui_float(struct ct_cdb_obj_t *obj,
                     uint64_t prop_key_hash,
                     struct ct_component_prop_map *prop) {
    float value = 0;
    float value_new = 0;

    ct_cdb_a0.read_float(obj, prop_key_hash, value_new);
    value = value_new;

    const float min = !prop->limit.max_f ? -FLT_MAX
                                         : prop->limit.min_f;
    const float max = !prop->limit.max_f ? FLT_MAX
                                         : prop->limit.max_f;

    if (ct_debugui_a0.DragFloat(prop->ui_name,
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

        ct_cmd_system_a0.execute(&cmd.header);
    }
}

static void ui_str(struct ct_cdb_obj_t *obj,
                   uint64_t prop_key_hash,
                   struct ct_component_prop_map *prop,
                   uint32_t i) {
    char labelid[128] = {'\0'};

    const char *value = 0;

    value = ct_cdb_a0.read_str(obj, prop_key_hash, NULL);

    char buffer[128] = {'\0'};
    strcpy(buffer, value);

    sprintf(labelid, "%s##prop_str_%d", prop->ui_name, i);


    bool change = false;
    if (prop->resource.type) {
        change = ui_select_asset(buffer, labelid,
                                 CT_ID32_0(prop->resource.type),
                                 prop_key_hash);
        ct_debugui_a0.SameLine(0.0f, -1.0f);
    }


    if (prop->combo.combo_items) {
        char *items = NULL;
        uint32_t items_count = 0;

        prop->combo.combo_items(obj, &items, &items_count);

        int current_item = -1;
        const char *items2[items_count];
        for (int j = 0; j < items_count; ++j) {
            items2[j] = &items[j * 128];
            if (CT_ID64_0(items2[j]) == CT_ID64_0(value)) {
                current_item = j;
            }
        }

        sprintf(labelid, "%s##combo_%d", prop->ui_name, i);
        change = ct_debugui_a0.Combo(labelid, &current_item, items2,
                                     items_count, -1);

        if (change) {
            strcpy(buffer, items2[current_item]);
        }

    } else {
        change |= ct_debugui_a0.InputText(labelid,
                                          buffer,
                                          strlen(buffer),
                                          DebugInputTextFlags_ReadOnly,
                                          0, NULL);
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

        ct_cmd_system_a0.execute(&cmd.header);
    }
}

static void ui_vec3(struct ct_cdb_obj_t *obj,
                    uint64_t prop_key_hash,
                    struct ct_component_prop_map *prop) {
    float value[3] = {0};
    float value_new[3] = {0};

    ct_cdb_a0.read_vec3(obj, prop_key_hash, value_new);
    ct_vec3_move(value, value_new);

    const float min = !prop->limit.min_f ? -FLT_MAX
                                         : prop->limit.min_f;
    const float max = !prop->limit.max_f ? FLT_MAX
                                         : prop->limit.max_f;

    if (ct_debugui_a0.DragFloat3(prop->ui_name,
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

                .vec3.new_value = {value_new[0],
                                   value_new[1],
                                   value_new[2]},

                .vec3.old_value = {value[0],
                                   value[1],
                                   value[2]},
        };

        ct_cmd_system_a0.execute(&cmd.header);
    }
}

static void on_component(struct ct_world world,
                         struct ct_cdb_obj_t *obj,
                         uint64_t comp_name) {
    struct ct_component_info *info = ct_ecs_a0.component_info(comp_name);

    if(!info){
        return;
    }

    if (!ct_debugui_a0.CollapsingHeader(info->component_name,
                                        DebugUITreeNodeFlags_DefaultOpen)) {
        return;
    }

    for (int i = 0; i < info->prop_count; ++i) {
        struct ct_component_prop_map *prop = &info->prop_map[i];
        uint64_t prop_key_hash = CT_ID64_0(prop->key);
        switch (prop->type) {
            case CDB_TYPE_VEC3:
                ui_vec3(obj, prop_key_hash, prop);
                break;

            case CDB_TYPE_FLOAT:
                ui_float(obj, prop_key_hash, prop);
                break;

            case CDB_TYPE_STR:
                ui_str(obj, prop_key_hash, prop, i);
                break;

            default:
                break;
        }
    }
}

static void on_debugui() {
    if (!_G.filename) {
        return;
    }

    if (ct_debugui_a0.Button("Save", (float[2]) {0.0f})) {
        ct_ydb_a0.save(_G.filename);
    }
    ct_debugui_a0.SameLine(0.0f, -1.0f);

    ct_debugui_a0.InputText("asset",
                            (char *) _G.filename, strlen(_G.filename),
                            DebugInputTextFlags_ReadOnly, 0, NULL);

    if (ct_debugui_a0.CollapsingHeader("Entity",
                                       DebugUITreeNodeFlags_DefaultOpen)) {
        ct_debugui_a0.LabelText("Entity", "%llu", _G.active_entity);
    }

    struct ct_resource_id rid;
    ct_resource_a0.type_name_from_filename(_G.filename, &rid, NULL);

    struct ct_cdb_obj_t *components_obj;
    components_obj = ct_cdb_a0.read_subobject(_G.obj, CT_ID64_0("components"),
                                              NULL);


    uint64_t n = ct_cdb_a0.prop_count(components_obj);
    uint64_t components_name[n];
    ct_cdb_a0.prop_keys(components_obj, components_name);

    for (uint64_t j = 0; j < n; ++j) {
        uint64_t name = components_name[j];

        ct_ep_on_component clb;
        clb = (ct_ep_on_component) ct_hash_lookup(&_G.components, name, 0);

        struct ct_cdb_obj_t * c_obj;
        c_obj = ct_cdb_a0.read_subobject(components_obj, name, NULL);

        clb ? clb(_G.active_world, _G.obj) : on_component(_G.active_world,
                                                          c_obj, name);
    }
}


void on_entity_click(struct ct_cdb_obj_t *event) {
    ct_property_editor_a0.set_active(on_debugui);

    struct ct_world world = {
            ct_cdb_a0.read_uint64(event, CT_ID64_0("world"), 0)};
    const char *filename = ct_cdb_a0.read_str(event, CT_ID64_0("filename"), 0);
    struct ct_entity entity = {
            ct_cdb_a0.read_uint64(event, CT_ID64_0("entity"), 0)};
    struct ct_cdb_obj_t *obj = {ct_cdb_a0.read_ref(event, CT_ID64_0("obj"), 0)};

    _G.active_world = world;
    _G.top_entity = entity;
    _G.filename = filename;
    _G.obj = obj;
}


void register_on_component_(uint64_t type,
                            ct_ep_on_component on_component) {
    ct_hash_add(&_G.components, type, (uint64_t) on_component, _G.allocator);
}

void unregister_on_component_(uint64_t type) {
}


static struct ct_entity_property_a0 entity_property_a0 = {
        .register_component = register_on_component_,
        .unregister_component = unregister_on_component_,
};


static void set_vec3_cmd(const struct ct_cmd *cmd,
                         bool inverse) {
    const struct ct_ent_cmd_s *pos_cmd = (const struct ct_ent_cmd_s *) cmd;

    const float *value = inverse ? pos_cmd->vec3.old_value
                                 : pos_cmd->vec3.new_value;

    ct_cdb_obj_o *w = ct_cdb_a0.write_begin(pos_cmd->obj);
    ct_cdb_a0.set_vec3(w, pos_cmd->prop, value);
    ct_cdb_a0.write_commit(w);
}

static void set_float_cmd(const struct ct_cmd *cmd,
                          bool inverse) {
    const struct ct_ent_cmd_s *pos_cmd = (const struct ct_ent_cmd_s *) cmd;

    const float value = inverse ? pos_cmd->f.old_value : pos_cmd->f.new_value;

    ct_cdb_obj_o *w = ct_cdb_a0.write_begin(pos_cmd->obj);
    ct_cdb_a0.set_float(w, pos_cmd->prop, value);
    ct_cdb_a0.write_commit(w);
}

static void set_str_cmd(const struct ct_cmd *_cmd,
                        bool inverse) {
    const struct ct_ent_cmd_s *pos_cmd = (const struct ct_ent_cmd_s *) _cmd;

    const char *value = inverse ? pos_cmd->str.old_value
                                : pos_cmd->str.new_value;

    ct_cdb_obj_o *w = ct_cdb_a0.write_begin(pos_cmd->obj);
    ct_cdb_a0.set_str(w, pos_cmd->prop, value);
    ct_cdb_a0.write_commit(w);
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

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator()
    };

    api->register_api("ct_entity_property_a0", &entity_property_a0);

    ct_ebus_a0.connect(EXPLORER_EBUS, EXPLORER_ENTITY_SELECT_EVENT,
                       on_entity_click, 0);

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("set_vec3"),
            (struct ct_cmd_fce) {
                    .execute = set_vec3_cmd,
                    .description = cmd_description});

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("set_str"),
            (struct ct_cmd_fce) {
                    .execute = set_str_cmd,
                    .description = cmd_description});

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("set_float"),
            (struct ct_cmd_fce) {
                    .execute = set_float_cmd,
                    .description = cmd_description});

}

static void _shutdown() {
    ct_ebus_a0.disconnect(EXPLORER_EBUS, EXPLORER_ENTITY_SELECT_EVENT,
                          on_entity_click);

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
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_explorer_a0);
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
