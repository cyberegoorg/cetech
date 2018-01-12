//==============================================================================
// Include
//==============================================================================

#include "celib/allocator.h"
#include "celib/map.inl"
#include "celib/buffer.inl"

#include "cetech/hashlib/hashlib.h"
#include "cetech/os/memory.h"
#include "cetech/api/api_system.h"
#include "cetech/log/log.h"
#include "cetech/os/path.h"
#include "cetech/os/vio.h"
#include "cetech/resource/resource.h"
#include <cetech/module/module.h>
#include <cetech/playground//asset_property.h>
#include <cetech/debugui/debugui.h>
#include <cetech/renderer/texture.h>
#include <cetech/playground//entity_property.h>
#include <cetech/entity/entity.h>
#include <cetech/transform/transform.h>
#include <cfloat>
#include <celib/fpumath.h>
#include <cetech/yaml/ydb.h>
#include <cetech/playground/command_system.h>

using namespace celib;
using namespace buffer;

//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
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


struct ct_ent_cmd_s {
    // ENT
    ct_world world;
    ct_entity entity;

    // ENT YAML
    const char *filename;
    uint64_t keys[32];
    uint32_t keys_count;
};

struct ct_ent_cmd_vec3_s {
    ct_cmd header;
    ct_ent_cmd_s ent;

    // VALUES
    float new_value[3];
    float old_value[3];
};

struct ct_ent_cmd_str_s {
    ct_cmd header;
    ct_ent_cmd_s ent;

    // VALUES
    char new_value[128];
    char old_value[128];
};

static void set_pos_cmd(const struct ct_cmd *cmd,
                        bool inverse) {
    const struct ct_ent_cmd_vec3_s *pos_cmd = (const ct_ent_cmd_vec3_s *) cmd;

    const float *value = inverse ? pos_cmd->old_value : pos_cmd->new_value;

    ct_ydb_a0.set_vec3(pos_cmd->ent.filename, pos_cmd->ent.keys,
                       pos_cmd->ent.keys_count, (float *) value);

    ct_transform t = ct_transform_a0.get(pos_cmd->ent.world,
                                         pos_cmd->ent.entity);
    ct_transform_a0.set_position(t, (float *) value);
}

static void set_scale_cmd(const struct ct_cmd *cmd,
                          bool inverse) {
    const struct ct_ent_cmd_vec3_s *pos_cmd = (const ct_ent_cmd_vec3_s *) cmd;

    const float *value = inverse ? pos_cmd->old_value : pos_cmd->new_value;

    ct_ydb_a0.set_vec3(pos_cmd->ent.filename, pos_cmd->ent.keys,
                       pos_cmd->ent.keys_count, (float *) value);

    ct_transform t = ct_transform_a0.get(pos_cmd->ent.world,
                                         pos_cmd->ent.entity);
    ct_transform_a0.set_scale(t, (float *) value);
}


static void set_rotation_cmd(const struct ct_cmd *cmd,
                             bool inverse) {
    const struct ct_ent_cmd_vec3_s *pos_cmd = (const ct_ent_cmd_vec3_s *) cmd;

    const float *value = inverse ? pos_cmd->old_value : pos_cmd->new_value;

    ct_ydb_a0.set_vec3(pos_cmd->ent.filename,
                       pos_cmd->ent.keys,
                       pos_cmd->ent.keys_count,
                       (float *) value);

    float rad_rot[3];
    float norm_rot[3];
    float q[4];
    vec3_mul(rad_rot, value, DEG_TO_RAD);
    quat_from_euler(q, rad_rot[0], rad_rot[1], rad_rot[2]);
    quat_norm(norm_rot, q);

    ct_transform t = ct_transform_a0.get(pos_cmd->ent.world,
                                         pos_cmd->ent.entity);
    ct_transform_a0.set_rotation(t, norm_rot);
}

static void cmd_description(char *buffer,
                            uint32_t buffer_size,
                            const struct ct_cmd *cmd,
                            bool inverse) {
    static const uint64_t set_position = CT_ID64_0(
            "transform_set_position");
    static const uint64_t set_rotation = CT_ID64_0(
            "transform_set_rotation");
    static const uint64_t set_scale = CT_ID64_0(
            "transform_set_scale");

    if (cmd->type == set_position) {
        struct ct_ent_cmd_vec3_s *ent_cmd = (struct ct_ent_cmd_vec3_s *) cmd;
        snprintf(buffer, buffer_size,
                 "Set ent position [%f, %f, %f] -> [%f, %f, %f]",
                 ent_cmd->old_value[0], ent_cmd->old_value[1],
                 ent_cmd->old_value[2],
                 ent_cmd->new_value[0], ent_cmd->new_value[1],
                 ent_cmd->new_value[2]);

    } else if (cmd->type == set_rotation) {
        struct ct_ent_cmd_vec3_s *ent_cmd = (struct ct_ent_cmd_vec3_s *) cmd;
        snprintf(buffer, buffer_size,
                 "Set ent rotation [%f, %f, %f] -> [%f, %f, %f]",
                 ent_cmd->old_value[0], ent_cmd->old_value[1],
                 ent_cmd->old_value[2],
                 ent_cmd->new_value[0], ent_cmd->new_value[1],
                 ent_cmd->new_value[2]);

    } else if (cmd->type == set_scale) {
        struct ct_ent_cmd_vec3_s *ent_cmd = (struct ct_ent_cmd_vec3_s *) cmd;
        snprintf(buffer, buffer_size,
                 "Set ent scale [%f, %f, %f] -> [%f, %f, %f]",
                 ent_cmd->old_value[0], ent_cmd->old_value[1],
                 ent_cmd->old_value[2],
                 ent_cmd->new_value[0], ent_cmd->new_value[1],
                 ent_cmd->new_value[2]);
    }
}

static void on_component(struct ct_world world,
                         struct ct_entity entity,
                         const char *filename,
                         uint64_t *keys,
                         uint32_t keys_count) {
    if (!ct_transform_a0.has(world, entity)) {
        return;
    }
    ct_transform t = ct_transform_a0.get(world, entity);

    if (!ct_debugui_a0.CollapsingHeader("Transformation",
                                        DebugUITreeNodeFlags_DefaultOpen)) {
        return;
    }

    //==========================================================================
    // Position
    //==========================================================================
    float pos[3];
    float pos_new[3];
    uint64_t tmp_keys[keys_count + 1];
    memcpy(tmp_keys, keys, sizeof(uint64_t) * keys_count);

    tmp_keys[keys_count] = ct_yng_a0.calc_key("position");
    ct_ydb_a0.get_vec3(filename, tmp_keys, keys_count + 1,
                       pos_new, (float[3]) {0.0f});

    vec3_move(pos, pos_new);

    if (ct_debugui_a0.DragFloat3("position", pos_new, 1.0f, -FLT_MAX, FLT_MAX,
                                 "%.3f", 1.0f)) {

        struct ct_ent_cmd_vec3_s cmd = {
                .header = {
                        .size = sizeof(struct ct_ent_cmd_vec3_s),
                        .type = CT_ID64_0("transform_set_position"),
                },
                .ent = {
                        .world = world,
                        .entity = entity,
                        .filename = filename,
                        .keys_count = keys_count + 1,
                },
                .new_value = {pos_new[0], pos_new[1], pos_new[2]},
                .old_value = {pos[0], pos[1], pos[2]},
        };
        memcpy(cmd.ent.keys, tmp_keys, sizeof(uint64_t) * cmd.ent.keys_count);
        ct_cmd_system_a0.execute(&cmd.header);
    }


    //==========================================================================
    // Rotation
    //==========================================================================
    float rot[4];
    float norm_rot[4];
    float tmp_rot[3];

    ct_transform_a0.get_rotation(t, rot);
    quat_norm(norm_rot, rot);
    quat_to_euler(rot, norm_rot);
    vec3_mul(tmp_rot, rot, RAD_TO_DEG);

    tmp_keys[keys_count] = ct_yng_a0.calc_key("rotation");
    ct_ydb_a0.get_vec3(filename, tmp_keys, keys_count + 1, tmp_rot,
                       (float[3]) {0.0f});

    vec3_move(rot, tmp_rot);
    if (ct_debugui_a0.DragFloat3("rotation", tmp_rot, 1.0f, 0, 360, "%.5f",
                                 1.0f)) {

        struct ct_ent_cmd_vec3_s cmd = {
                .header = {
                        .type = CT_ID64_0("transform_set_rotation"),
                        .size = sizeof(struct ct_ent_cmd_vec3_s),
                },
                .ent = {
                        .world = world,
                        .entity = entity,
                        .filename = filename,
                        .keys_count = keys_count + 1,
                },
                .new_value = {tmp_rot[0], tmp_rot[1], tmp_rot[2]},
                .old_value = {rot[0], rot[1], rot[2]},
        };
        memcpy(cmd.ent.keys, tmp_keys, sizeof(uint64_t) * cmd.ent.keys_count);
        ct_cmd_system_a0.execute(&cmd.header);
    }

    //==========================================================================
    // Scale
    //==========================================================================
    float scale[3];
    float scale_new[3];
    tmp_keys[keys_count] = ct_yng_a0.calc_key("scale");
    ct_ydb_a0.get_vec3(filename, tmp_keys, keys_count + 1, scale_new,
                       (float[3]) {1.0f});

    vec3_move(scale, scale_new);
    if (ct_debugui_a0.DragFloat3("scale", scale_new, 1.0f,
                                 -FLT_MAX, FLT_MAX,
                                 "%.3f", 1.0f)) {

        struct ct_ent_cmd_vec3_s cmd = {
                .header = {
                        .size = sizeof(struct ct_ent_cmd_vec3_s),
                        .type = CT_ID64_0("transform_set_scale"),
                },
                .ent = {
                        .world = world,
                        .entity = entity,
                        .filename = filename,
                        .keys_count = keys_count + 1,
                },
                .new_value = {scale_new[0], scale_new[1], scale_new[2]},
                .old_value = {scale[0], scale[1], scale[2]},
        };
        memcpy(cmd.ent.keys, tmp_keys, sizeof(uint64_t) * cmd.ent.keys_count);
        ct_cmd_system_a0.execute(&cmd.header);
    }
}

static int _init(ct_api_a0 *api) {
    CEL_UNUSED(api);

    _G = {};

    ct_entity_property_a0.register_component(
            CT_ID64_0("transform"),
            on_component);

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("transform_set_position"),
            (ct_cmd_fce) {
                    .execute = set_pos_cmd,
                    .description = cmd_description});

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("transform_set_scale"),
            (ct_cmd_fce) {
                    .execute = set_scale_cmd,
                    .description = cmd_description});

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("transform_set_rotation"),
            (ct_cmd_fce) {
                    .execute = set_rotation_cmd,
                    .description = cmd_description});

    return 1;
}

static void _shutdown() {

    ct_entity_property_a0.unregister_component(CT_ID64_0("transform"));

    _G = {};
}


CETECH_MODULE_DEF(
        transform_property,
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
        },
        {
            CEL_UNUSED(reload);
            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);

            _shutdown();
        }
)