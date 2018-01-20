//==============================================================================
// Include
//==============================================================================

#include "cetech/core/allocator.h"
#include "cetech/core/map.inl"
#include "cetech/core/buffer.inl"

#include "cetech/core/hashlib.h"
#include "cetech/core/memory.h"
#include "cetech/core/api_system.h"
#include "cetech/core/log.h"
#include "cetech/core/path.h"
#include "cetech/core/vio.h"
#include "cetech/engine/resource/resource.h"
#include <cetech/core/module.h>
#include <cetech/core/private/ydb.h>
#include <cetech/engine/filesystem/filesystem.h>
#include <cetech/playground//asset_property.h>
#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/renderer/texture.h>
#include <cetech/playground//entity_property.h>
#include <cetech/engine/entity/entity.h>
#include <cetech/engine/transform/transform.h>
#include <cetech/playground/command_system.h>
#include <cetech/engine/debugui/private/ocornut-imgui/imgui.h>
#include <cetech/playground/asset_browser.h>
#include <cetech/engine/renderer/mesh_renderer.h>
#include <cetech/engine/renderer/scene.h>

using namespace celib;
using namespace buffer;

//==============================================================================
// GLobals
//==============================================================================

#define _G mesh_property_globals
static struct _G {
} _G;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_asset_property_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_texture_a0);
CETECH_DECL_API(ct_entity_property_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_cmd_system_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_asset_browser_a0);
CETECH_DECL_API(ct_mesh_renderer_a0);
CETECH_DECL_API(ct_scene_a0);


struct ct_ent_cmd_s {
    // ENT
    ct_world world;
    ct_entity entity;

    // ENT YAML
    const char *filename;
    uint64_t keys[32];
    uint32_t keys_count;
};

struct ct_ent_cmd_str_s {
    ct_cmd header;
    ct_ent_cmd_s ent;

    uint32_t idx;

    // VALUES
    char new_value[128];
    char old_value[128];
};

static void cmd_set_str(const struct ct_cmd *cmd,
                        bool inverse) {
    const struct ct_ent_cmd_str_s *pos_cmd = (const ct_ent_cmd_str_s *) cmd;

    const char *value = inverse ? pos_cmd->old_value : pos_cmd->new_value;

    ct_ydb_a0.set_string(pos_cmd->ent.filename,
                         pos_cmd->ent.keys, pos_cmd->ent.keys_count,
                         value);

    ct_mesh_renderer mr = ct_mesh_renderer_a0.get(pos_cmd->ent.world,
                                                  pos_cmd->ent.entity);
    if (!ct_mesh_renderer_a0.is_valid(mr)) {
        return;
    }

    if (pos_cmd->header.type == CT_ID64_0("mesh_set_material")) {
        ct_mesh_renderer_a0.set_material(mr, pos_cmd->idx, CT_ID64_0(value));

    } else if (pos_cmd->header.type == CT_ID64_0("mesh_set_mesh")) {
        ct_mesh_renderer_a0.set_geometry(mr, pos_cmd->idx, CT_ID64_0(value));

    } else if (pos_cmd->header.type == CT_ID64_0("mesh_set_node")) {
        ct_mesh_renderer_a0.set_node(mr, pos_cmd->idx, CT_ID64_0(value));

    } else if (pos_cmd->header.type == CT_ID64_0("mesh_set_scene")) {
        ct_mesh_renderer_a0.set_scene(mr, CT_ID64_0(value));
    }
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


static bool get_selected_asset(char *asset_name,
                               const char *asset_type) {
    if (ct_asset_browser_a0.get_selected_asset_type() !=
        CT_ID64_0(asset_type)) {
        return false;
    }

    ct_asset_browser_a0.get_selected_asset_name(asset_name);
    return true;
}

static bool ui_select_asset(char *buffer,
                            const char *id,
                            const char *asset_type,
                            const char *old_value,
                            ct_world world,
                            ct_entity entity,
                            const char *filename,
                            uint64_t *keys,
                            uint32_t keys_count,
                            uint64_t cmd_type,
                            uint32_t idx) {

    char id_str[512] = {0};
    sprintf(id_str, ">>##%s", id);

    if (ct_debugui_a0.Button(id_str, (float[2]) {0.0f})) {
        if (get_selected_asset(buffer, asset_type)) {
            struct ct_ent_cmd_str_s cmd = {
                    .header = {
                            .size = sizeof(struct ct_ent_cmd_str_s),
                            .type = cmd_type,
                    },
                    .ent = {
                            .world = world,
                            .entity = entity,
                            .filename = filename,
                            .keys_count = keys_count,
                    },
                    .idx = idx,
            };

            memcpy(cmd.ent.keys, keys,
                   sizeof(uint64_t) * keys_count);

            strcpy(cmd.new_value, buffer);
            strcpy(cmd.old_value, old_value);

            ct_cmd_system_a0.execute(&cmd.header);
            return true;
        }
    }

    strcpy(buffer, old_value);

    return false;
}

static void on_component(struct ct_world world,
                         struct ct_entity entity,
                         const char *filename,
                         uint64_t *keys,
                         uint32_t keys_count) {

    if (!ct_debugui_a0.CollapsingHeader("Mesh renderer",
                                        DebugUITreeNodeFlags_DefaultOpen)) {
        return;
    }

    uint64_t tmp_keys[keys_count + 3];
    memcpy(tmp_keys, keys, sizeof(uint64_t) * keys_count);

    //==========================================================================
    // Scene
    //==========================================================================

    tmp_keys[keys_count] = ct_yng_a0.calc_key("scene");
    const char *scene_str = ct_ydb_a0.get_string(filename, tmp_keys,
                                                 keys_count + 1, "");

    char labelid[128] = {'\0'};
    sprintf(labelid, "mp_scene");

    char scene_buffer[128] = {'\0'};
    ui_select_asset(scene_buffer, labelid, "scene", scene_str, world, entity,
                    filename,
                    tmp_keys, keys_count + 1, CT_ID64_0("mesh_set_scene"), 0);


    ct_debugui_a0.SameLine(0.0f, -1.0f);
    ct_debugui_a0.InputText("scene",
                            (char *) scene_buffer, strlen(scene_buffer),
                            DebugInputTextFlags_ReadOnly, 0, NULL);

    //==========================================================================
    // Geometries
    //==========================================================================
    uint64_t geom[32] = {};
    uint32_t geom_keys_count = 0;

    tmp_keys[keys_count] = ct_yng_a0.calc_key("geometries");
    ct_ydb_a0.get_map_keys(filename,
                           tmp_keys, keys_count + 1,
                           geom, CETECH_ARRAY_LEN(geom),
                           &geom_keys_count);


    for (uint32_t i = 0; i < geom_keys_count; ++i) {
        tmp_keys[keys_count + 1] = geom[i];

        tmp_keys[keys_count + 2] = ct_yng_a0.calc_key("mesh");
        const char *mesh_name = ct_ydb_a0.get_string(filename, tmp_keys,
                                                     keys_count + 3, "");

        tmp_keys[keys_count + 2] = ct_yng_a0.calc_key("material");
        const char *material_name = ct_ydb_a0.get_string(filename, tmp_keys,
                                                         keys_count + 3, "");

        tmp_keys[keys_count + 2] = ct_yng_a0.calc_key("node");
        const char *node_name = ct_ydb_a0.get_string(filename, tmp_keys,
                                                     keys_count + 3, "");

        char id[32] = {0};
        sprintf(id, "element %d", i);
        if (ImGui::TreeNodeEx(id, ImGuiTreeNodeFlags_DefaultOpen)) {


            //======================================================================
            // Material
            //======================================================================

            sprintf(labelid, "mp_select_material_%d", i);

            tmp_keys[keys_count + 2] = ct_yng_a0.calc_key("material");
            char material_buffer[128] = {'\0'};
            ui_select_asset(material_buffer, labelid, "material", material_name,
                            world,
                            entity, filename, tmp_keys, keys_count + 3,
                            CT_ID64_0("mesh_set_material"), i);

            ct_debugui_a0.SameLine(0.0f, -1.0f);
            sprintf(labelid, "material##mp_material_%d", i);
            ct_debugui_a0.InputText(labelid,
                                    (char *) material_buffer,
                                    strlen(material_buffer),
                                    DebugInputTextFlags_ReadOnly, 0, NULL);

            //======================================================================
            // Mesh
            //======================================================================
            {
                char *items;
                uint32_t items_count;
                ct_scene_a0.get_all_geometries(CT_ID64_0(scene_buffer), &items,
                                               &items_count);

                int item2 = -1;
                const char *items2[items_count];
                for (int j = 0; j < items_count; ++j) {
                    items2[j] = &items[j * 128];

                    if (!strcmp(items2[j], mesh_name)) {
                        item2 = j;
                    }
                }

                sprintf(labelid, "mesh##mp_mesh_%d", i);
                if (ct_debugui_a0.Combo(labelid, &item2, items2, items_count,
                                        -1)) {
                    tmp_keys[keys_count + 2] = ct_yng_a0.calc_key("mesh");
                    struct ct_ent_cmd_str_s cmd = {
                            .header = {
                                    .size = sizeof(struct ct_ent_cmd_str_s),
                                    .type = CT_ID64_0("mesh_set_mesh"),
                            },
                            .ent = {
                                    .world = world,
                                    .entity = entity,
                                    .filename = filename,
                                    .keys_count = keys_count + 3,
                            },
                            .idx = i,
                    };

                    memcpy(cmd.ent.keys, tmp_keys,
                           sizeof(uint64_t) * (keys_count + 3));

                    strcpy(cmd.new_value, items2[item2]);
                    strcpy(cmd.old_value, mesh_name);

                    ct_cmd_system_a0.execute(&cmd.header);
                }
            }

            //======================================================================
            // Node
            //======================================================================
            {
                char *items;
                uint32_t items_count;
                ct_scene_a0.get_all_nodes(CT_ID64_0(scene_buffer), &items,
                                          &items_count);

                int item2 = -1;
                const char *items2[items_count];
                for (int j = 0; j < items_count; ++j) {
                    items2[j] = &items[j * 128];

                    if (!strcmp(items2[j], node_name)) {
                        item2 = j;
                    }
                }

                sprintf(labelid, "node##mp_node_%d", i);
                if (ct_debugui_a0.Combo(labelid, &item2, items2, items_count,
                                        -1)) {
                    tmp_keys[keys_count + 2] = ct_yng_a0.calc_key("node");
                    struct ct_ent_cmd_str_s cmd = {
                            .header = {
                                    .size = sizeof(struct ct_ent_cmd_str_s),
                                    .type = CT_ID64_0("mesh_set_node"),
                            },
                            .ent = {
                                    .world = world,
                                    .entity = entity,
                                    .filename = filename,
                                    .keys_count = keys_count + 3,
                            },
                            .idx = i,
                    };

                    memcpy(cmd.ent.keys, tmp_keys,
                           sizeof(uint64_t) * (keys_count + 3));

                    strcpy(cmd.new_value, items2[item2]);
                    strcpy(cmd.old_value, node_name);

                    ct_cmd_system_a0.execute(&cmd.header);
                };
            }

            ImGui::TreePop();
        }
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
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_asset_property_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_texture_a0);
            CETECH_GET_API(api, ct_entity_property_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_cmd_system_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_asset_browser_a0);
            CETECH_GET_API(api, ct_mesh_renderer_a0);
            CETECH_GET_API(api, ct_scene_a0);
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