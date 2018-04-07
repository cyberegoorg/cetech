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
#include <cetech/engine/debugui/private/ocornut-imgui/imgui.h>
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
    const struct ct_ent_cmd_str_s *pos_cmd = (const ct_ent_cmd_str_s *) _cmd;

    const char *value = inverse ? pos_cmd->old_value : pos_cmd->new_value;

    ct_cdb_obj_t *w = ct_cdb_a0.write_begin(pos_cmd->ent.obj);
    ct_cdb_a0.set_uint64(w, pos_cmd->ent.prop, CT_ID64_0(value));
    ct_cdb_a0.write_commit(w);
}

static void cmd_description(char *buffer,
                            uint32_t buffer_size,
                            const struct ct_cmd *cmd,
                            bool inverse) {

    static const uint64_t set_scene = CT_ID64_0("mesh_set_scene");
    static const uint64_t set_mesh = CT_ID64_0("mesh_set_mesh");
    static const uint64_t set_node = CT_ID64_0("mesh_set_node");
    static const uint64_t set_material = CT_ID64_0("mesh_set_material");

    if (cmd->type == set_scene) {
        struct ct_ent_cmd_str_s *ent_cmd = (struct ct_ent_cmd_str_s *) cmd;
        snprintf(buffer, buffer_size,
                 "Set ent mesh scene %s -> %s",
                 ent_cmd->old_value, ent_cmd->new_value);

    } else if (cmd->type == set_mesh) {
        struct ct_ent_cmd_str_s *ent_cmd = (struct ct_ent_cmd_str_s *) cmd;
        snprintf(buffer, buffer_size,
                 "Set ent mesh %s -> %s",
                 ent_cmd->old_value, ent_cmd->new_value);

    } else if (cmd->type == set_node) {
        struct ct_ent_cmd_str_s *ent_cmd = (struct ct_ent_cmd_str_s *) cmd;
        snprintf(buffer, buffer_size,
                 "Set ent mesh node%s -> %s",
                 ent_cmd->old_value, ent_cmd->new_value);

    } else if (cmd->type == set_material) {
        struct ct_ent_cmd_str_s *ent_cmd = (struct ct_ent_cmd_str_s *) cmd;
        snprintf(buffer, buffer_size,
                 "Set ent mesh material%s -> %s",
                 ent_cmd->old_value, ent_cmd->new_value);
    }
}


static bool ui_select_asset(ct_cdb_obj_t *object,
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
        ct_cdb_obj_t *wr = ct_cdb_a0.write_begin(object);
        ct_cdb_a0.set_uint64(wr, key, k);
        ct_cdb_a0.write_commit(wr);

        return true;
    }

    return false;
}

static void ui_scene(ct_cdb_obj_t *ent_obj,
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
    uint64_t geom_count = ct_cdb_a0.read_uint64(obj, PROP_GEOM_COUNT, 0);
    for (uint32_t i = 0; i < geom_count; ++i) {
        char id[32] = {0};
        sprintf(id, "element %d", i);
        if (!ct_debugui_a0.TreeNodeEx(id, ImGuiTreeNodeFlags_DefaultOpen)) {
//            ImGui::TreePop();
            continue;
        }

        uint64_t mesh = ct_cdb_a0.read_uint64(obj, PROP_MESH_ID + i, 0);
        uint64_t node = ct_cdb_a0.read_uint64(obj, PROP_NODE + i, 0);
        uint64_t mid = ct_cdb_a0.read_uint64(obj, PROP_MATERIAL_ID + i, 0);

        struct ct_resource_id rid = (struct ct_resource_id){
                .type = CT_ID32_0("material"),
                .name = static_cast<uint32_t>(mid),
        };

        ct_cdb_obj_t *material = ct_resource_a0.get_obj(rid);


        //======================================================================
        // Material
        //======================================================================
        char material_buffer[128] = {'\0'};
        strcpy(material_buffer,
               ct_cdb_a0.read_str(material, CT_ID64_0("asset_name"), ""));

        sprintf(labelid, "mp_select_material_%d", i);
        ui_select_asset(obj, labelid,
                        CT_ID64_0("material"), PROP_MATERIAL_ID + i);

        ct_debugui_a0.SameLine(0.0f, -1.0f);
        sprintf(labelid, "material##mp_material_%d", i);
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

            sprintf(labelid, "mesh##mp_mesh_%d", i);
            if (ct_debugui_a0.Combo(labelid, &item2, items2, items_count, -1)) {
                struct ct_ent_cmd_str_s cmd = {
                        .header = {
                                .size = sizeof(struct ct_ent_cmd_str_s),
                                .type = CT_ID64_0("mesh_set_mesh"),
                        },
                        .ent = {
                                .obj = obj,
                                .prop = PROP_MESH_ID + i,
                        },
                };


                strcpy(cmd.new_value, items2[item2]);
                strcpy(cmd.old_value, items2[item3]);

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

            sprintf(labelid, "node##mp_node_%d", i);
            if (ct_debugui_a0.Combo(labelid, &item2, items2, items_count, -1)) {
                struct ct_ent_cmd_str_s cmd = {
                        .header = {
                                .size = sizeof(struct ct_ent_cmd_str_s),
                                .type = CT_ID64_0("mesh_set_node"),
                        },
                        .ent = {

                                .obj = obj,
                                .prop = PROP_NODE_ID + i,
                        },
                };

                strcpy(cmd.new_value, items2[item2]);
                strcpy(cmd.old_value, items2[item3]);

                ct_cmd_system_a0.execute(&cmd.header);
            };
        }

        ImGui::TreePop();
    }
}

static int _init(ct_api_a0 *api) {
    CT_UNUSED(api);

    _G = {};

    ct_entity_property_a0.register_component(
            CT_ID64_0("mesh_renderer"),
            on_component);

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("mesh_set_scene"),
            (ct_cmd_fce) {
                    .execute = cmd_set_str,
                    .description = cmd_description});

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("mesh_set_mesh"),
            (ct_cmd_fce) {
                    .execute = cmd_set_str,
                    .description = cmd_description});

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("mesh_set_node"),
            (ct_cmd_fce) {
                    .execute = cmd_set_str,
                    .description = cmd_description});

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("mesh_set_material"),
            (ct_cmd_fce) {
                    .execute = cmd_set_str,
                    .description = cmd_description});

    return 1;
}

static void _shutdown() {
    ct_entity_property_a0.unregister_component(CT_ID64_0("transform"));

    _G = {};
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