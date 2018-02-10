//==============================================================================
// Include
//==============================================================================

#include "cetech/core/memory/allocator.h"
#include "cetech/core/containers/map.inl"
#include "cetech/core/containers/buffer.h"

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/log/log.h"
#include "cetech/core/os/path.h"
#include "cetech/core/os/vio.h"
#include "cetech/engine/resource/resource.h"
#include <cetech/core/module/module.h>
#include <cetech/playground//asset_property.h>
#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/renderer/texture.h>
#include <cetech/playground//entity_property.h>
#include <cetech/engine/world/world.h>
#include <cetech/engine/transform/transform.h>
#include <cfloat>

#include <cetech/core/yaml/ydb.h>
#include <cetech/playground/command_system.h>
#include <cetech/core/math/fmath.h>

using namespace celib;

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
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_asset_property_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_texture_a0);
CETECH_DECL_API(ct_entity_property_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_world_a0);
CETECH_DECL_API(ct_cmd_system_a0);
CETECH_DECL_API(ct_cdb_a0);


static void set_vec3_cmd(const struct ct_cmd *cmd,
                        bool inverse) {
    const struct ct_ent_cmd_vec3_s *pos_cmd = (const ct_ent_cmd_vec3_s *) cmd;

    const float *value = inverse ? pos_cmd->old_value : pos_cmd->new_value;

    ct_ydb_a0.set_vec3(pos_cmd->ent.filename, pos_cmd->ent.keys,
                       pos_cmd->ent.keys_count, (float *) value);

    ct_cdb_writer_t* w = ct_cdb_a0.write_begin(pos_cmd->ent.obj);
    ct_cdb_a0.set_vec3(w, pos_cmd->ent.prop, value);
    ct_cdb_a0.write_commit(w);
}

static void cmd_description(char *buffer,
                            uint32_t buffer_size,
                            const struct ct_cmd *cmd,
                            bool inverse) {
    static const uint64_t set_position = CT_ID64_0("transform_set_position");
    static const uint64_t set_rotation = CT_ID64_0("transform_set_rotation");
    static const uint64_t set_scale = CT_ID64_0("transform_set_scale");

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
                         struct ct_cdb_obj_t *obj,
                         const char *filename,
                         uint64_t *keys,
                         uint32_t keys_count) {
    if (!ct_debugui_a0.CollapsingHeader("Transformation",
                                        DebugUITreeNodeFlags_DefaultOpen)) {
        return;
    }

    uint64_t tmp_keys[keys_count + 1];
    memcpy(tmp_keys, keys, sizeof(uint64_t) * keys_count);

//    ct_cdb_obj_t* obj = ct_world_a0.ent_obj(world, entity);

    //==========================================================================
    // Position
    //==========================================================================
    float pos[3];
    float pos_new[3];

    ct_cdb_a0.read_vec3(obj, PROP_POSITION, pos_new);
    ct_vec3_move(pos, pos_new);
    if (ct_debugui_a0.DragFloat3("position", pos_new, 1.0f,
                                 -FLT_MAX, FLT_MAX,
                                 "%.3f", 1.0f)) {

        tmp_keys[keys_count] = ct_yng_a0.key("position");
        struct ct_ent_cmd_vec3_s cmd = {
                .header = {
                        .size = sizeof(struct ct_ent_cmd_vec3_s),
                        .type = CT_ID64_0("transform_set_position"),
                },
                .ent = {
                        .filename = filename,
                        .keys_count = keys_count + 1,
                        .prop = PROP_POSITION,
                        .obj = obj,
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
    float rot[3];
    float rot_new[3];


    ct_cdb_a0.read_vec3(obj, PROP_ROTATION, rot);

    ct_vec3_move(rot_new, rot);
    if (ct_debugui_a0.DragFloat3("rotation", rot_new, 1.0f, 0, 360, "%.5f",
                                 1.0f)) {

        tmp_keys[keys_count] = ct_yng_a0.key("rotation");
        struct ct_ent_cmd_vec3_s cmd = {
                .header = {
                        .size = sizeof(struct ct_ent_cmd_vec3_s),
                        .type = CT_ID64_0("transform_set_rotation"),
                },
                .ent = {
                        .filename = filename,
                        .keys_count = keys_count + 1,
                        .prop = PROP_ROTATION,
                        .obj = obj,
                },
                .new_value = {rot_new[0], rot_new[1], rot_new[2]},
                .old_value = {rot[0], rot[1], rot[2]},
        };
        memcpy(cmd.ent.keys, tmp_keys, sizeof(uint64_t) * cmd.ent.keys_count);
        ct_cmd_system_a0.execute(&cmd.header);
    }

    //==========================================================================
    // Scale
    //==========================================================================
    float scale[3];
    float scale_name[3];

    ct_cdb_a0.read_vec3(obj, PROP_SCALE, scale);
    ct_vec3_move(scale_name, scale);
    if (ct_debugui_a0.DragFloat3("scale", scale_name, 1.0f,
                                 -FLT_MAX, FLT_MAX,
                                 "%.3f", 1.0f)) {

        tmp_keys[keys_count] = ct_yng_a0.key("scale");
        struct ct_ent_cmd_vec3_s cmd = {
                .header = {
                        .size = sizeof(struct ct_ent_cmd_vec3_s),
                        .type = CT_ID64_0("transform_set_scale"),
                },
                .ent = {
                        .filename = filename,
                        .keys_count = keys_count + 1,
                        .prop = PROP_SCALE,
                        .obj = obj,
                },
                .new_value = {scale_name[0], scale_name[1], scale_name[2]},
                .old_value = {scale[0], scale[1], scale[2]},
        };
        memcpy(cmd.ent.keys, tmp_keys, sizeof(uint64_t) * cmd.ent.keys_count);
        ct_cmd_system_a0.execute(&cmd.header);
    }
}

static int _init(ct_api_a0 *api) {
    CT_UNUSED(api);

    _G = {};

    ct_entity_property_a0.register_component(
            CT_ID64_0("transform"),
            on_component);

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("transform_set_position"),
            (ct_cmd_fce) {
                    .execute = set_vec3_cmd,
                    .description = cmd_description});

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("transform_set_scale"),
            (ct_cmd_fce) {
                    .execute = set_vec3_cmd,
                    .description = cmd_description});

    ct_cmd_system_a0.register_cmd_execute(
            CT_ID64_0("transform_set_rotation"),
            (ct_cmd_fce) {
                    .execute = set_vec3_cmd,
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
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_asset_property_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_texture_a0);
            CETECH_GET_API(api, ct_entity_property_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_cmd_system_a0);
            CETECH_GET_API(api, ct_world_a0);
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