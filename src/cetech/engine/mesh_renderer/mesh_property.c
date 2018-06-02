//==============================================================================
// Include
//==============================================================================

#include "cetech/kernel/memory/allocator.h"
#include "cetech/kernel/containers/buffer.h"
#include "cetech/kernel/hashlib/hashlib.h"
#include "cetech/kernel/memory/memory.h"
#include "cetech/kernel/api/api_system.h"
#include "cetech/kernel/log/log.h"
#include <cetech/kernel/module/module.h>
#include <cetech/kernel/yaml/ydb.h>

#include "cetech/engine/resource/resource.h"
#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/ecs/ecs.h>
#include <cetech/engine/transform/transform.h>
#include <cetech/engine/scene/scene.h>
#include <cetech/engine/mesh_renderer/mesh_renderer.h>

#include <cetech/playground/command_system.h>
#include <cetech/playground//entity_property.h>
#include <cetech/playground/asset_browser.h>


//==============================================================================
// GLobals
//==============================================================================

#define _G mesh_property_globals
static struct _G {
} _G;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_entity_property_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_cmd_system_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_mesh_renderer_a0);
CETECH_DECL_API(ct_scene_a0);
CETECH_DECL_API(ct_ecs_a0);
CETECH_DECL_API(ct_cdb_a0);


static void cmd_set_str(const struct ct_cmd *_cmd,
                        bool inverse) {
    const struct ct_ent_cmd_s *pos_cmd = (const struct ct_ent_cmd_s *) _cmd;

    const char *value = inverse ? pos_cmd->str.old_value
                                : pos_cmd->str.new_value;

    struct ct_cdb_obj_t *w = ct_cdb_a0.write_begin(pos_cmd->obj);
    ct_cdb_a0.set_uint64(w, pos_cmd->prop, CT_ID64_0(value));
    ct_cdb_a0.write_commit(w);
}

static void cmd_description(char *buffer,
                            uint32_t buffer_size,
                            const struct ct_cmd *cmd,
                            bool inverse) {

    const uint64_t set_scene = CT_ID64_0("mesh_set_scene");
    const uint64_t set_mesh = CT_ID64_0("mesh_set_mesh");
    const uint64_t set_node = CT_ID64_0("mesh_set_node");
    const uint64_t set_material = CT_ID64_0("mesh_set_material");

    struct ct_ent_cmd_s *ent_cmd = (struct ct_ent_cmd_s *) cmd;

    if (cmd->type == set_scene) {

        snprintf(buffer, buffer_size,
                 "Set ent mesh scene %s -> %s",
                 ent_cmd->str.old_value, ent_cmd->str.new_value);

    } else if (cmd->type == set_mesh) {
        snprintf(buffer, buffer_size,
                 "Set ent mesh %s -> %s",
                 ent_cmd->str.old_value, ent_cmd->str.new_value);

    } else if (cmd->type == set_node) {
        snprintf(buffer, buffer_size,
                 "Set ent mesh node%s -> %s",
                 ent_cmd->str.old_value, ent_cmd->str.new_value);

    } else if (cmd->type == set_material) {
        snprintf(buffer, buffer_size,
                 "Set ent mesh material%s -> %s",
                 ent_cmd->str.old_value, ent_cmd->str.new_value);
    }
}


static bool ui_select_asset(struct ct_cdb_obj_t *object,
                            const char *id,
                            uint64_t asset_type,
                            uint64_t key) {
    char id_str[512] = {0};
    sprintf(id_str, ">>##%s", id);

    if (ct_debugui_a0.Button(id_str, (float[2]) {0.0f})) {
        if (ct_asset_browser_a0.get_selected_asset_type() != asset_type) {
            return false;
        }

        char selected_asset[128] = {0};
        ct_asset_browser_a0.get_selected_asset_name(selected_asset);

        uint64_t k = CT_ID64_0(selected_asset);
        struct ct_cdb_obj_t *wr = ct_cdb_a0.write_begin(object);
        ct_cdb_a0.set_uint64(wr, key, k);
        ct_cdb_a0.write_commit(wr);

        return true;
    }

    return false;
}

static void ui_scene(struct ct_cdb_obj_t *ent_obj,
                     uint64_t scene) {
    char labelid[128] = {'\0'};
    sprintf(labelid, "mp_scene");

    char scene_buffer[128] = {'\0'};
//    ui_select_asset(scene_buffer, labelid, "scene");

    snprintf(scene_buffer, CT_ARRAY_LEN(scene_buffer), "%llu", scene);
//    ct_debugui_a0.SameLine(0.0f, -1.0f);
    ct_debugui_a0.InputText("scene",
                            (char *) scene_buffer, strlen(scene_buffer),
                            DebugInputTextFlags_ReadOnly, 0, NULL);
}

static void on_component(struct ct_world world,
                         struct ct_cdb_obj_t *obj) {

    if (!ct_debugui_a0.CollapsingHeader("Mesh renderer",
                                        DebugUITreeNodeFlags_DefaultOpen)) {
        return;
    }

    char labelid[128] = {'\0'};

    //==========================================================================
    // Scene
    //==========================================================================
    uint64_t scene = ct_cdb_a0.read_uint64(obj, PROP_SCENE, 0);
    ui_scene(obj, scene);

    //==========================================================================
    // Geometries
    //==========================================================================

    uint64_t mesh = ct_cdb_a0.read_uint64(obj, PROP_MESH_ID, 0);
    uint64_t node = ct_cdb_a0.read_uint64(obj, PROP_NODE, 0);
    uint64_t mid = ct_cdb_a0.read_uint64(obj, PROP_MATERIAL_ID, 0);

    struct ct_resource_id rid = (struct ct_resource_id) {
            .type = CT_ID32_0("material"),
            .name = (uint32_t) mid,
    };

    struct ct_cdb_obj_t *material = ct_resource_a0.get_obj(rid);


    //======================================================================
    // Material
    //======================================================================
    char material_buffer[128] = {'\0'};
    strcpy(material_buffer,
           ct_cdb_a0.read_str(material, CT_ID64_0("asset_name"), ""));

    sprintf(labelid, "mp_select_material_%d", 0);
    ui_select_asset(obj, labelid,
                    CT_ID64_0("material"), PROP_MATERIAL_ID);

    ct_debugui_a0.SameLine(0.0f, -1.0f);
    sprintf(labelid, "material##mp_material_%d", 0);
    ct_debugui_a0.InputText(labelid,
                            material_buffer,
                            strlen(material_buffer),
                            DebugInputTextFlags_ReadOnly, 0, NULL);

    //======================================================================
    // Mesh
    //======================================================================
    {
        char *items;
        uint32_t items_count;
        ct_scene_a0.get_all_geometries(scene, &items, &items_count);

        int item2 = -1;
        int item3 = -1;
        const char *items2[items_count];
        for (int j = 0; j < items_count; ++j) {
            items2[j] = &items[j * 128];
            if (CT_ID64_0(items2[j]) == mesh) {
                item3 = item2 = j;
            }
        }

        sprintf(labelid, "mesh##mp_mesh_%d", 0);
        if (ct_debugui_a0.Combo(labelid, &item2, items2, items_count, -1)) {
            struct ct_ent_cmd_s cmd = {
                    .header = {
                            .size = sizeof(struct ct_ent_cmd_s),
                            .type = CT_ID64_0("mesh_set_mesh"),
                    },
                    .obj = obj,
                    .prop = PROP_MESH_ID,
            };


            strcpy(cmd.str.new_value, items2[item2]);
            strcpy(cmd.str.old_value, items2[item3]);

            ct_cmd_system_a0.execute(&cmd.header);
        }
    }

    //======================================================================
    // Node
    //======================================================================
    {
        char *items;
        uint32_t items_count;
        ct_scene_a0.get_all_nodes(scene, &items, &items_count);

        int item2 = -1;
        int item3 = -1;
        const char *items2[items_count];
        for (int j = 0; j < items_count; ++j) {
            items2[j] = &items[j * 128];
            if (CT_ID64_0(items2[j]) == node) {
                item3 = item2 = j;
            }
        }

        sprintf(labelid, "node##mp_node_%d", 0);
        if (ct_debugui_a0.Combo(labelid, &item2, items2, items_count, -1)) {
            struct ct_ent_cmd_s cmd = {
                    .header = {
                            .size = sizeof(struct ct_ent_cmd_s),
                            .type = CT_ID64_0("mesh_set_node"),
                    },


                    .obj = obj,
                    .prop = PROP_NODE_ID,

            };

            strcpy(cmd.str.new_value, items2[item2]);
            strcpy(cmd.str.old_value, items2[item3]);

            ct_cmd_system_a0.execute(&cmd.header);
        };
    }

//    ct_debugui_a0.TreePop();
}

static int _init(struct ct_api_a0 *api) {
    CT_UNUSED(api);

    _G = (struct _G){};

    ct_entity_property_a0.register_component(
            CT_ID64_0("mesh_renderer"),
            on_component);

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("mesh_set_scene"),
            (struct ct_cmd_fce) {
                    .execute = cmd_set_str,
                    .description = cmd_description});

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("mesh_set_mesh"),
            (struct ct_cmd_fce) {
                    .execute = cmd_set_str,
                    .description = cmd_description});

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("mesh_set_node"),
            (struct ct_cmd_fce) {
                    .execute = cmd_set_str,
                    .description = cmd_description});

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("mesh_set_material"),
            (struct ct_cmd_fce) {
                    .execute = cmd_set_str,
                    .description = cmd_description});

    return 1;
}

static void _shutdown() {
    ct_entity_property_a0.unregister_component(CT_ID64_0("transform"));

    _G = (struct _G) {};
}


CETECH_MODULE_DEF(
        mesh_property,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_entity_property_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_cmd_system_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_mesh_renderer_a0);
            CETECH_GET_API(api, ct_scene_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_cdb_a0);
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